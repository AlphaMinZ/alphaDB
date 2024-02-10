#include "../inc/batch.h"
#include "../inc/db.h"
#include "../inc/errors.h"

#include <atomic>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace alphaDB {

const uint64_t nonTransactionSeqNo = 0;

std::string txnFinKey("txn-fin");

// NewWriteBatch 初始化 WriteBatch
std::shared_ptr<WriteBatch> DB::NewWriteBatch(WriteBatchOptions opts) {
    WriteBatch::ptr writeBatch(new WriteBatch);
    writeBatch->setOptions(opts);
    writeBatch->setDB(shared_from_this());

    return writeBatch;
}

// Put 批量写数据
void WriteBatch::Put(std::string key, std::string value) {
    if(key.size() == 0) {
        throw MyErrors::ErrKeyIsEmpty;
    }
    alphaMin::Mutex::Lock lock(m_mutex);

    // 暂存 LogRecord
    LogRecord* logRecord = new LogRecord{key, value};
    m_pendingWrites[key] = logRecord;

    lock.unlock();
    return;
}

// Delete 删除数据
void WriteBatch::Delete(std::string key) {
    if(key.size() == 0) {
        throw MyErrors::ErrKeyIsEmpty;
    }
    alphaMin::Mutex::Lock lock(m_mutex);

    // 数据不存在则直接返回
    LogRecordPos::ptr logRecordPos = m_db->getIndex()->Get(key);
    if(logRecordPos.get() == nullptr) {
        if(m_pendingWrites[key]!= nullptr) {
            m_pendingWrites.erase(key);
        }
        lock.unlock();
        return;
    }

    // 暂存 LogRecord
    LogRecord* logRecord = new LogRecord;
    logRecord->Key = key;
    logRecord->Type = LogRecordDeleted;
    m_pendingWrites[key] = logRecord;

    lock.unlock();
    return;
}

// Commit 提交事务，将暂存的数据写到数据文件，并更新内存索引
void WriteBatch::Commit() {
    alphaMin::Mutex::Lock lock(m_mutex);

    if(m_pendingWrites.size() == 0) {
        lock.unlock();
        return;
    }
    if((uint32_t)(m_pendingWrites.size()) > m_options.MaxBatchNum) {
        lock.unlock();
        throw MyErrors::ErrExceedMaxBatchNum;
    }

    // 加锁保证事务提交串行化
    alphaMin::Mutex::Lock ll(getDB()->getMutex());

    // 获取当前最新的事务序列号
    uint64_t seqNo =  getDB()->getSeqNo().fetch_add(1);

    // 开始写数据到数据文件
    std::map<std::string, LogRecordPos::ptr> positions;
    for(const auto& pair : m_pendingWrites) {
        LogRecord* logRecord = new LogRecord{logRecordKeyWithSeq(pair.first, seqNo), pair.second->Value, pair.second->Type};
        LogRecordPos::ptr logRecordPos = getDB()->appendLogRecord(logRecord);
        positions[pair.first] = logRecordPos;
    }

    // 写一条标识事务完成的数据
    LogRecord* finishedRecord = new LogRecord;
    finishedRecord->Key = logRecordKeyWithSeq(txnFinKey, seqNo);
    finishedRecord->Type = LogRecordTxnFinished;
    getDB()->appendLogRecord(finishedRecord);

    // 根据配置决定是否持久化
    if(m_options.SyncWrites && (getDB()->getActiveFile().get() != nullptr)) {
        getDB()->getActiveFile()->Sync();
    }

    // 更新内存索引
    for(const auto& pair : m_pendingWrites) {
        LogRecordPos::ptr pos = positions[pair.first];
        if(pair.second->Type == LogRecordNormal) {
            getDB()->getIndex()->Put(pair.first, pos, nullptr);
        }
        if(pair.second->Type == LogRecordDeleted) {
            getDB()->getIndex()->Delete(pair.first, nullptr);
        }
    }

    // 清空暂存数据
    m_pendingWrites.clear();

    ll.unlock();
    lock.unlock();

    return;
}

// key+Seq Number 编码
std::string logRecordKeyWithSeq(std::string key, uint64_t seqNo) {
    std::string seq(sizeof(uint64_t), '\0');
    // 将 seqNo 放入 seq 开头
    std::memcpy(&seq[0], &seqNo, sizeof(uint64_t));

    std::string enKey(sizeof(uint64_t) + key.size(), '\0');
    std::copy(seq.begin(), seq.begin() + sizeof(uint64_t), enKey);
    std::copy(key.begin(), key.end(), enKey.begin() + sizeof(uint64_t));

    return enKey;
}

// 解析 LogRecord 的 key，获取实际的 key 和事务序列号
std::string parseLogRecordKey(std::string key, uint64_t& seqNo_) {
    // uint32_t crc;
    // std::memcpy(&crc, buf.data(), sizeof(uint32_t));
    uint64_t seqNo;
    std::memcpy(&seqNo, key.data(), sizeof(uint64_t));
    seqNo_ = seqNo;

    std::string realKey(key.begin() + sizeof(uint64_t), key.end());

    return realKey;
}

}