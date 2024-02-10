#ifndef __ALPHA_BATCH_H__
#define __ALPHA_BATCH_H__

#include <memory>
#include <map>
#include <string>

#include "options.h"
#include "db.h"
#include "log_record.h"
#include "../lib/inc/mutex.h"

namespace alphaDB {

class WriteBatch {
public:
    typedef std::shared_ptr<WriteBatch> ptr;

    WriteBatch() {}

    // Put 批量写数据
    void Put(std::string key, std::string value);

    // Delete 删除数据
    void Delete(std::string key);

    // Commit 提交事务，将暂存的数据写到数据文件，并更新内存索引
    void Commit();

    void setOptions(WriteBatchOptions options) { m_options = options;}

    void setDB(DB::ptr db) { m_db = db;}

    DB::ptr& getDB() { return m_db;}
private:
    WriteBatchOptions m_options;
    alphaMin::Mutex m_mutex;
    DB::ptr m_db;
    std::map<std::string, LogRecord*> m_pendingWrites;  // 暂存用户写入的数据
};

// key+Seq Number 编码
std::string logRecordKeyWithSeq(std::string key, uint64_t seqNo);
// 解析 LogRecord 的 key，获取实际的 key 和事务序列号
std::string parseLogRecordKey(std::string key, uint64_t& seqNo_);

extern const uint64_t nonTransactionSeqNo;

}

#endif