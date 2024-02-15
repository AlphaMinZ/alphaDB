#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "../inc/db.h"
#include "../inc/batch.h"
#include "../lib/inc/mutex.h"
#include "../inc/utils.h"

namespace alphaDB {

// Open 打开 bitcask 存储引擎实例
DB::ptr Open(Options& options) {
    // 对用户传入的配置项进行校验
    checkOptions(options);

    bool isInitial;
    // 判断数据目录是否存在，如果不存在的话，则创建这个目录
    struct stat st;
    if(stat(options.DirPath.c_str(), &st) != 0) {
        isInitial = true;
        // 如果目录不存在，则创建目录
        if (mkdir(options.DirPath.c_str(), 0700) != 0) {
            perror("Error creating directory");
        }
    }

    auto entries = readDirectory(options.DirPath);
    if(entries.size() == 2) {
        isInitial = true;
    }

    // 初始化 DB 实体
    DB::ptr db(new DB);
    {
        db->setOptions(options);
        db->setIndex(NewIndexer(options.IndexType));
        db->setIsInitial(isInitial);
    }

    // 加载 merge 数据目录
    db->loadMergeFiles();

    // 加载数据文件
    db->loadDataFiles();

    // 从 hint 索引文件中加载索引
    db->loadIndexFromHintFile();

    // 从数据文件中加载索引
    db->loadIndexFromDataFiles();

    return db;
}

// Close 关闭数据库
void DB::Close() {
    if(m_activeFile.get() == nullptr) {
        return;
    }
    alphaMin::Mutex::Lock lock(m_mutex);

    // 关闭活跃文件
    m_activeFile->Close();

    // 关闭旧的数据文件
    for(const auto& pair : m_olderFiles) {
        m_olderFiles[pair.first]->Close();
    }

    lock.unlock();
}

// Sync 持久化数据文件
void DB::Sync() {
    if(m_activeFile.get() == nullptr) {
        return;
    }
    alphaMin::Mutex::Lock lock(m_mutex);

    m_activeFile->Sync();

    lock.unlock();
}

// Stat 返回数据库的相关统计信息
std::shared_ptr<Stat> DB::stat() {
    alphaMin::RWMutex::ReadLock rlock(m_RWMutex);

    auto dataFiles = (uint32_t)m_olderFiles.size();
    if(m_activeFile.get() != nullptr) {
        dataFiles++;
    }

    auto dirSize = DirSize(m_options.DirPath);    
    std::shared_ptr<Stat> ret_stat(new Stat);
    ret_stat->KeyNum = (uint32_t)getIndex()->Size();
    ret_stat->DataFileNum = dataFiles;
    ret_stat->ReclaimableSize = m_reclaimSize;

    rlock.unlock();
    return ret_stat;
}

// Put 写入 Key/Value 数据，key 不能为空
void DB::Put(std::string key, std::string value) {
    // 判断 key 是否有效
    if(key.size() == 0) {
        throw MyErrors::ErrKeyIsEmpty;
    }

    // 构造 LogRecord 结构体
    // LogRecord* logRecord = new LogRecord{key, value, LogRecordNormal};
    LogRecord* logRecord = new LogRecord{logRecordKeyWithSeq(key, nonTransactionSeqNo), value, LogRecordNormal};

    // 追加写入到当前数据活跃文件当中
    LogRecordPos::ptr pos = appendLogRecordWithLock(logRecord);

    // 更新内存索引
    auto oldPos = m_index->Put(key, pos, nullptr);
    if(oldPos.get() != nullptr) {
        m_reclaimSize += (int64_t)oldPos->size;
    }

    return;
}

// Delete 根据 key 删除对应的数据
void DB::Delete(std::string key) {
    alphaMin::RWMutex::ReadLock lock(m_RWMutex);

    // 判断 key 的有效性
    if(key.size() == 0) {
        lock.unlock();
        throw MyErrors::ErrKeyIsEmpty;
    }

    // 先检查 key 是否存在，如果不存在的话直接返回
    LogRecordPos::ptr pos = m_index->Get(key);
    if(pos.get() == nullptr) {
        return;
    }

    // 构造 LogRecord，标识其是被删除的
    LogRecord* logRecord = new LogRecord;
    // logRecord->Key = key;
    logRecord->Key = logRecordKeyWithSeq(key, nonTransactionSeqNo);
    logRecord->Type = LogRecordDeleted;
    pos = appendLogRecordWithLock(logRecord);
    m_reclaimSize += (int64_t)pos->size;

    // 从内存索引中将对应的 key 删除
    auto oldPos = m_index->Delete(key, nullptr);
    if(oldPos.get() != nullptr) {
        m_reclaimSize += (int64_t)oldPos->size;
    }

    return;
}

// Get 根据 key 读取数据
std::string DB::Get(std::string key) {
    alphaMin::RWMutex::ReadLock lock(m_RWMutex);

    // 判断 key 的有效性
    if(key.size() == 0) {
        lock.unlock();
        throw MyErrors::ErrKeyIsEmpty;
    }

    // 从内存数据结构中取出 key 对应的索引信息
    LogRecordPos::ptr logRecordPos = m_index->Get(key);
    // 如果 key 不在内存索引中，说明 key 不存在
    if(logRecordPos.get() == nullptr) {
        lock.unlock();
        std::cout << "-------------\n";
        throw MyErrors::ErrKeyNotFound;
        std::cout << "-------------\n";
    }

    std::string value = getValueByPosition(logRecordPos); 

    lock.unlock();
    return value;
}

// ListKeys 获取数据库中所有的 key
std::vector<std::string> DB::ListKeys() {
    IteratorInterFace::ptr iterator = m_index->Iterator(false);
    std::vector<std::string> keys(m_index->Size());

    int idx = 0;
    for(iterator->Rewind(); iterator->Valid(); iterator->Next()) {
        keys[idx] = iterator->Key();
        idx++;
    }

    return keys;
}

// Fold 获取所有的数据，并执行用户指定的操作，函数返回 false 时终止遍历
void DB::Fold(std::function<bool (std::string key, std::string value)> f) {
    alphaMin::Mutex::Lock lock(m_mutex);

    IteratorInterFace::ptr iterator = m_index->Iterator(false);
    for(iterator->Rewind(); iterator->Valid(); iterator->Next()) {
        std::string value = getValueByPosition(iterator->Value());

        if(!f(iterator->Key(), value)) {
            break;
        }
    }
}

// 根据索引信息获取对应的 value
std::string DB::getValueByPosition(LogRecordPos::ptr logRecordPos) {
    // 根据文件 id 找到对应的数据文件
    DataFile::ptr dataFile;
    if(m_activeFile->getFileId() == logRecordPos->fId) {
        dataFile = m_activeFile;
    } else {
        dataFile = m_olderFiles[logRecordPos->fId];
    }
    // 数据文件为空
    if(dataFile.get() == nullptr) {
        throw MyErrors::ErrDataFileNotFound;
    }

    // 根据偏移读取对应的数据
    uint32_t size;
    LogRecord* logRecord = dataFile->ReadLogRecord(logRecordPos->offset, size, nullptr);

    if(logRecord->Type == LogRecordDeleted) {
        std::cout << "Type = Delete\n";
        throw MyErrors::ErrKeyNotFound;
    }

    return logRecord->Value;
}

LogRecordPos::ptr DB::appendLogRecordWithLock(LogRecord* logRecord) {
    alphaMin::Mutex::Lock lock(m_mutex);

    LogRecordPos::ptr logRecordPos = appendLogRecord(logRecord);

    lock.unlock();
    return logRecordPos;
}

// 追加写数据到活跃文件中
LogRecordPos::ptr DB::appendLogRecord(LogRecord* logRecord) {
    // 判断当前活跃数据文件是否存在，因为数据库在没有写入的时候是没有文件生成的
	// 如果为空则初始化数据文件
    if(m_activeFile.get() == nullptr) {
        setActiveDataFileLocked();
    }

    // 写入数据编码
    std::string encRecord;
    int64_t size;
    encRecord= EncodeLogRecord(logRecord, size);
    // 如果写入的数据已经到达了活跃文件的阈值，则关闭活跃文件，并打开新的文件
    if((m_activeFile->getWriteOff() + size) > m_options.DataFileSize) {
        // 先持久化数据文件，保证已有的数据持久到磁盘当中
        m_activeFile->Sync();

        // 当前活跃文件转换为旧的数据文件
        m_olderFiles[m_activeFile->getFileId()] = m_activeFile;

        // 打开新的数据文件
        setActiveDataFileLocked();
    }

    int64_t writeOff = m_activeFile->getWriteOff();
    m_activeFile->Write(encRecord);
    
    m_bytesWrite += (uint32_t)size;
    // 根据用户配置决定是否持久化
    auto needSync = m_options.SyncWrites;
    if(!needSync && m_options.BytesPerSync > 0 && m_bytesWrite >= m_options.BytesPerSync) {
        needSync = true;
    }
    if(needSync) {
        m_activeFile->Sync();

        // 清空累计值
        if(m_bytesWrite > 0) {
            m_bytesWrite = 0;
        }
    }

    // 构建内存索引信息
    LogRecordPos::ptr pos(new LogRecordPos(m_activeFile->getFileId(), writeOff));
    pos->size = (uint32_t)size;

    return pos; 
}

// 设置当前活跃文件
// 在访问此方法前必须持有互斥锁
void DB::setActiveDataFileLocked() {
    uint32_t initialFileId = 0;
    if(m_activeFile.get() != nullptr) {
        initialFileId = m_activeFile->getFileId() + 1;
    }
    // 打开新的数据文件
    // DataFile::ptr dataFile(new DataFile(m_options.DirPath, initialFileId));
    DataFile::ptr dataFile = OpenDataFile(m_options.DirPath, initialFileId);
    m_activeFile = dataFile;
}

std::vector<std::string> readDirectory(const std::string& dirPath) {
    std::vector<std::string> dirEntries;

    // 打开目录
    DIR* dir = opendir(dirPath.c_str());
    if(dir == nullptr) {
        std::cout << "ErrOpenDirectoryFailed\n";
        throw MyErrors::ErrOpenDirectoryFailed;
        return dirEntries;
    }

    // 读取目录中的所有条目
    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr) {
        // 只处理普通文件和子目录
        if(entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            dirEntries.push_back(entry->d_name);
        }
    }

    // 关闭目录
    closedir(dir);
    return dirEntries;
}

std::vector<int> extractFileIds(const std::vector<std::string>& dirEntries, const char* suffix) {
    std::vector<int> fileIds;

    for(const auto& entry : dirEntries) {
        // 只处理以指定后缀结尾的文件
        if(entry.find(suffix) != std::string::npos) {
            // 使用 stringstream 拆分文件名
            std::vector<std::string> splitNames;
            std::istringstream iss(entry);
            std::string token;
            while (std::getline(iss, token, '.')) {
                splitNames.push_back(token);
            }

            try {
                int fileId = std::stoi(splitNames[0]);
                fileIds.push_back(fileId);
            } catch (const std::invalid_argument& e) {
                throw MyErrors::ErrDataDirectoryCorrupted;
                return fileIds;
            }
        }
    }

    return fileIds;
}

#define DataFileNameSuffix      ".data"

// 从磁盘中加载数据文件
void DB::loadDataFiles() {
    auto dirEntries = readDirectory(m_options.DirPath);

    std::vector<int> fileIds;
    // 遍历目录中的所有文件，找到所有以 .data 结尾的文件
    fileIds = extractFileIds(dirEntries, DataFileNameSuffix);

    //	对文件 id 进行排序，从小到大依次加载
    std::sort(fileIds.begin(), fileIds.end());
    m_fileIds = fileIds;

    // 遍历每个文件id，打开对应的数据文件
    for(int i = 0; i < fileIds.size(); ++i) {
        int fid = fileIds[i];

        // DataFile::ptr dataFile(new DataFile(m_options.DirPath, (uint32_t)fid));
        DataFile::ptr dataFile = OpenDataFile(m_options.DirPath, (uint32_t)fid);
        if(i == fileIds.size() - 1) {
            // 最后一个，id是最大的，说明是当前活跃文件
            m_activeFile = dataFile;
        } else {
            m_olderFiles[(uint32_t)fid] = dataFile;
        }
    }
}

// 从数据文件中加载索引
// 遍历文件中的所有记录，并更新到内存索引中
void DB::loadIndexFromDataFiles() {
    // 没有文件，说明数据库是空的，直接返回
    if(m_fileIds.size() == 0) {
        return;
    }

    // 查看是否发生过 merge
    auto hasMerge = false;
    auto nonMergeFileId = (uint32_t)0;
    auto mergeFinFileName = m_options.DirPath + "/" + MergeFinishedFileName;
    if(directoryExists(mergeFinFileName)) {
        auto fid = getNonMergeFileId(m_options.DirPath);
        hasMerge = true;
        nonMergeFileId = fid;
    }

    auto sharedPtr = shared_from_this();

    auto updateIndex = [sharedPtr](std::string key, LogRecordType typ, LogRecordPos::ptr pos){
        bool ok;
        LogRecordPos::ptr oldPos;
        if(typ == LogRecordDeleted) {
            oldPos = sharedPtr->getIndex()->Delete(key, &ok);
            sharedPtr->m_reclaimSize += (int64_t)pos->size;
        } else {
            oldPos = sharedPtr->getIndex()->Put(key, pos, &ok);
        }
        if(oldPos.get() != nullptr) {
            sharedPtr->m_reclaimSize += (int64_t)oldPos->size;
        }
        if(!ok) {
            perror("failed to update index at startup");
        }
    };

    // 暂存事务数据
    std::map<uint64_t, TransactionRecord::ptr> transactionRecords;
    uint64_t currentSeqNo =  nonTransactionSeqNo;

    // 遍历所有文件id，处理文件中的记录
    for(int i = 0; i < m_fileIds.size(); ++i) {
        uint32_t fileId = (uint32_t)m_fileIds[i];

        // 如果比最近未参与 merge 的文件 id 更小，则说明已经从 Hint 文件中加载索引了
        if(hasMerge && (fileId < nonMergeFileId)) {
            continue;
        }

        DataFile::ptr dataFile;
        if(fileId == m_activeFile->getFileId()) {
            dataFile = m_activeFile;
        } else {
            dataFile = m_olderFiles[fileId];
        }

        int64_t offset = 0;
        LogRecord* logRecord;
        while(true) {
            uint32_t size = 0;
            bool isEOF = false;
            logRecord = dataFile->ReadLogRecord(offset, size, &isEOF);
            if(isEOF) {
                break;
            }

            if(logRecord == nullptr) {
                break;
            }

            // 构造内存索引并保存
            LogRecordPos::ptr logRecordPos(new LogRecordPos(fileId, offset, (uint32_t)size));
            // bool isOk = false;
            // if(logRecord->Type == LogRecordDeleted) {
            //     m_index->Delete(logRecord->Key, &isOk);
            // } else {
            //     m_index->Put(logRecord->Key, logRecordPos, &isOk);
            // }
            // if(!isOk) {
            //     throw MyErrors::ErrIndexUpdateFailed;
            // }

            // 解析 key，拿到事务序列号
            uint64_t seqNo;
            std::string realKey = parseLogRecordKey(logRecord->Key, seqNo);
            if(seqNo == nonTransactionSeqNo) {
                // 非事务操作，直接更新内存索引
                updateIndex(realKey, logRecord->Type, logRecordPos);
            } else {
                // 事务完成，对应的 seq no 的数据可以更新到内存索引中
                if(logRecord->Type == LogRecordTxnFinished) {
                    for(const auto& pair : transactionRecords) {
                        updateIndex(pair.second->Record->Key, pair.second->Record->Type, pair.second->Pos);
                    }
                    transactionRecords.erase(seqNo);
                } else {
                    logRecord->Key = realKey;
                    TransactionRecord::ptr transactionRecord(new TransactionRecord);
                    transactionRecord->Record = logRecord;
                    transactionRecord->Pos = logRecordPos;
                    transactionRecords[seqNo] = transactionRecord;
                }
            }

            // 更新事务序列号
            if(seqNo > currentSeqNo) {
                currentSeqNo = seqNo;
            }

            // 递增 offset，下一次从新的位置开始读取
            offset += size;
        }

        // 如果是当前活跃文件，更新这个文件的 WriteOff
        if(i == m_fileIds.size() - 1) {
            m_activeFile->setWriteOff(offset);
        }
    }

    // 更新事物序列号
    m_seqNo = currentSeqNo;
    return;
}

void checkOptions(Options options) {
    if(options.DirPath == "") {
        throw std::runtime_error("database dir path is empty");
    }
    if(options.DataFileSize <= 0) {
        throw std::runtime_error("database data file size must be greater than 0");
    }
    if(options.DataFileMergeRatio < 0 || options.DataFileMergeRatio > 1) {
        throw std::runtime_error("invalid merge ratio, must between 0 and 1");
    }
}

void DB::loadSeqNo() {
    auto fileName = m_options.DirPath + "/" + SeqNoFileName;
    if(!directoryExists(fileName)) {
        return;
    }

    auto seqNoFile = OpenSeqNoFile(m_options.DirPath);
    uint32_t size;
    auto record = seqNoFile->ReadLogRecord(0, size, nullptr);

    uint64_t seqNo = 0;
    std::memcpy(&seqNo, &record->Value[0], sizeof(uint64_t));

    m_seqNo = seqNo;
    m_seqNoFileExists = true;

    removeDirectory(fileName);

    return;
}

}