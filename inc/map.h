#ifndef __ALPHA_MAP_H__
#define __ALPHA_MAP_H__

#include "index.h"
#include "log_record.h"
#include "../lib/inc/mutex.h"

#include <map>
#include <vector>
#include <memory>
#include <stdint.h>

namespace alphaDB {

class Map : public Indexer {
public:
    typedef std::shared_ptr<Map> ptr;

    Map();
    ~Map() {}

    LogRecordPos::ptr Put(const std::string key, LogRecordPos::ptr pos, bool* isOk_) override;

    LogRecordPos::ptr Get(const std::string key) override;

    LogRecordPos::ptr Delete(std::string key, bool* isOk_) override;

    int Size() override; 

    IteratorInterFace::ptr Iterator(bool reverse) override;
private:
    std::map<std::string, LogRecordPos::ptr> m_map;
    alphaMin::Mutex m_mutex;
    alphaMin::RWMutex m_RWMutex;
};

class mapIterator : public IteratorInterFace {
public:
    typedef std::shared_ptr<mapIterator> ptr;

    mapIterator() {}

    // Rewind 重新回到迭代器的起点，即第一个数据
    void Rewind() override;

    // Seek 根据传入的 key 查找到第一个大于（或小于）等于的目标 key，根据从这个 key 开始遍历
    void Seek(std::string key) override;

    // Next 跳转到下一个 key
    void Next() override;

    // Valid 是否有效，即是否已经遍历完了所有的 key，用于退出遍历
    bool Valid() override;

    // Key 当前遍历位置的 Key 数据
    std::string Key() override;

    // Value 当前遍历位置的 Value 数据
    LogRecordPos::ptr Value() override;

    // Close 关闭迭代器，释放相应资源
    void Close() override;

    void setCurrIndex(int currIndex) { m_currIndex = currIndex;}

    void setReveser(bool reveser) { m_reveser = reveser;}

    void setValues(std::vector<Item::ptr> values) { m_values = values;}

private:
    int m_currIndex;                // 当前遍历的下标位置
    bool m_reveser;                   // 是否是反向遍历
    std::vector<Item::ptr> m_values;  // key + 位置索引信息
};

mapIterator::ptr newMapIterator(std::map<std::string, LogRecordPos::ptr>& map, bool reverse);

}

#endif