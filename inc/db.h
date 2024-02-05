#ifndef __ALPHA_DB_H__
#define __ALPHA_DB_H__

#include <memory>
#include <map>
#include <vector>

#include "../lib/inc/mutex.h"
#include "errors.h"
#include "options.h"
#include "data_file.h"
#include "index.h"

namespace alphaDB {

class Iterator;

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
    std::shared_ptr<Iterator> Newiterator(IteratorOption opts); 

    alphaMin::Mutex& getMutex() { return m_mutex;}

private:
    Options m_options;
    alphaMin::RWMutex m_RWMutex;
    alphaMin::Mutex m_mutex;
    std::vector<int> m_fileIds;                     // 文件 id，只能在加载索引的时候使用，不能在其他的地方更新和使用
    DataFile::ptr m_activeFile;                     // 当前活跃数据文件 可以用于写入
    std::map<uint32_t, DataFile::ptr> m_olderFiles; // 旧的数据文件 只能进行读
    Indexer::ptr m_index;                               // 内存索引
};

DB::ptr Open(Options& options);

void checkOptions(Options options);

}

#endif