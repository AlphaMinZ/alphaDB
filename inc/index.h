#ifndef __ALPHA_INDEX_H__
#define __ALPHA_INDEX_H__

#include <string>
#include <memory>
#include <stdint.h>
#include <vector>
#include <stdint.h>

#include "log_record.h"

namespace alphaDB {

enum class IndexerType : int8_t {
    Map = 1,
    Btree
};

// Iterator 通用索引迭代器
class IteratorInterFace {
public:
    typedef std::shared_ptr<IteratorInterFace> ptr;

    // Rewind 重新回到迭代器的起点，即第一个数据
    virtual void Rewind() = 0;

    // Seek 根据传入的 key 查找到第一个大于（或小于）等于的目标 key，根据从这个 key 开始遍历
    virtual void Seek(std::string key) = 0;

    // Next 跳转到下一个 key
    virtual void Next() = 0;

    // Valid 是否有效，即是否已经遍历完了所有的 key，用于退出遍历
    virtual bool Valid() = 0;

    // Key 当前遍历位置的 Key 数据
    virtual std::string Key() = 0;

    // Value 当前遍历位置的 Value 数据
    virtual LogRecordPos::ptr Value() = 0;

    // Close 关闭迭代器，释放相应资源
    virtual void Close() = 0;
};

class Indexer {
public:
    typedef::std::shared_ptr<Indexer> ptr;

    virtual LogRecordPos::ptr Put(const std::string key, LogRecordPos::ptr pos, bool* isOk_) = 0;

    virtual LogRecordPos::ptr Get(const std::string key) = 0;

    virtual LogRecordPos::ptr Delete(std::string key, bool* isOk_) = 0;

    // Size 索引中的数据量
    virtual int Size() = 0;

    // Iterator 索引迭代器
    virtual IteratorInterFace::ptr Iterator(bool reverse) = 0;
};

class Item {
public:
    typedef std::shared_ptr<Item> ptr;

    Item() {}

    Item(const std::string& key, LogRecordPos::ptr pos) : key(key), pos(pos) {}

    std::string key;

    LogRecordPos::ptr pos;
};

Indexer::ptr NewIndexer(IndexerType typ);

}

#endif