#ifndef __ALPHA_DB_H__
#define __ALPHA_DB_H__

#include <memory>
#include <map>
#include <vector>
#include <atomic>

#include "../lib/inc/mutex.h"
#include "errors.h"
#include "options.h"
#include "data_file.h"
#include "index.h"

namespace alphaDB {

class Iterator;
class WriteBatch;

class Stat {
public:
    typedef std::shared_ptr<Stat> ptr;

    Stat() {}

    uint32_t KeyNum;            // key 的总数量
    uint32_t DataFileNum;       // 数据文件的数量
    int64_t ReclaimableSize;    // 可以进行 merge 回收的数据量，字节为单位
    int64_t DiskSize;           // 数据目录所占磁盘空间大小
};

class DB : public std::enable_shared_from_this<DB> {
public:
    typedef std::shared_ptr<DB> ptr;

    DB() {}

    std::vector<int> getFileIds() const { return m_fileIds;}

    DataFile::ptr getActiveFile() const { return m_activeFile;}

    std::map<uint32_t, DataFile::ptr> getOlderFiles() const { return m_olderFiles;}

    Indexer::ptr getIndex() const { return m_index;}

    void setOptions(Options& options) { m_options = options;}

    void setFileIds(std::vector<int> fileIds) { m_fileIds = fileIds;}

    void setActiveFile(DataFile::ptr activeFile) { m_activeFile = activeFile;}

    void setIndex(Indexer::ptr index) { m_index = index;}

    void setIsInitial(bool isInitial) { m_isInitial = isInitial;}

    // Put 写入 Key/Value 数据，key 不能为空
    void Put(std::string key, std::string value);

    // Get 根据 key 读取数据
    std::string Get(std::string key);

    // Delete 根据 key 删除对应的数据
    void Delete(std::string key);

    // Close 关闭数据库
    void Close();

    // Sync 持久化数据文件
    void Sync();

    LogRecordPos::ptr appendLogRecordWithLock(LogRecord* logRecord);

    // 追加写数据到活跃文件中
    LogRecordPos::ptr appendLogRecord(LogRecord* logRecord);

    // 设置当前活跃文件
    // 在访问此方法前必须持有互斥锁
    void setActiveDataFileLocked();

    // 从磁盘中加载数据文件
    void loadDataFiles();

    // 从数据文件中加载索引
    // 遍历文件中的所有记录，并更新到内存索引中
    void loadIndexFromDataFiles();

    // ListKeys 获取数据库中所有的 key
    std::vector<std::string> ListKeys();

    // Fold 获取所有的数据，并执行用户指定的操作，函数返回 false 时终止遍历
    void Fold(std::function<bool (std::string key, std::string value)> f);

    // 根据索引信息获取对应的 value
    std::string getValueByPosition(LogRecordPos::ptr logRecordPos);

    // NewIterator 初始化迭代器
    std::shared_ptr<Iterator> Newiterator(IteratorOptions opts); 

    // NewWriteBatch 初始化 WriteBatch
    std::shared_ptr<WriteBatch> NewWriteBatch(WriteBatchOptions opts);

    // Merge 清理无效数据，生成 Hint 文件
    void Merge();

    std::string getMergePath();

    // 加载 merge 数据目录
    void loadMergeFiles();

    uint32_t getNonMergeFileId(std::string dirPath);

    // 从 hint 文件中加载索引
    void loadIndexFromHintFile();

    // Stat 返回数据库的相关统计信息
    Stat::ptr stat();

    void loadSeqNo();

    alphaMin::Mutex& getMutex() { return m_mutex;}

    std::atomic<uint64_t>& getSeqNo() { return m_seqNo;}

    void addReclaimSize(int64_t cnt) { m_reclaimSize += cnt;}

private:
    Options m_options;
    alphaMin::RWMutex m_RWMutex;
    alphaMin::Mutex m_mutex;
    std::vector<int> m_fileIds;                     // 文件 id，只能在加载索引的时候使用，不能在其他的地方更新和使用
    DataFile::ptr m_activeFile;                     // 当前活跃数据文件 可以用于写入
    std::map<uint32_t, DataFile::ptr> m_olderFiles; // 旧的数据文件 只能进行读
    Indexer::ptr m_index;                           // 内存索引
    std::atomic<uint64_t> m_seqNo;                                 // 事务序列号，全局递增
    bool m_isMerging = false;                               // 是否正在 merge
    bool m_seqNoFileExists;                           // 存储事务序列号的文件是否存在
    bool m_isInitial;                                 // 是否是第一次初始化此数据目录
    // 文件锁
    uint32_t m_bytesWrite;                            // 累计写了多少个字节
    int64_t m_reclaimSize;                            // 表示有多少数据是无效的
};

DB::ptr Open(Options& options);

void checkOptions(Options options);

std::vector<std::string> readDirectory(const std::string& dirPath);

std::vector<int> extractFileIds(const std::vector<std::string>& dirEntries, const char* suffix);

bool directoryExists(const std::string& path);

bool createDirectory(const std::string& path);

bool removeDirectory(const std::string& path);

}

#endif