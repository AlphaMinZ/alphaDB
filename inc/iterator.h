#ifndef __ALPHA_ITERATOR_H__
#define __ALPHA_ITERATOR_H__

#include <memory>

#include "log_record.h"
#include "index.h"
#include "db.h"
#include "options.h"

namespace alphaDB {

class Iterator {
public:
    typedef std::shared_ptr<Iterator> ptr;

    Iterator() {}

    // Rewind 重新回到迭代器的起点，即第一个数据
    void Rewind();

    // Seek 根据传入的 key 查找到第一个大于（或小于）等于的目标 key，根据从这个 key 开始遍历
    void Seek(std::string key);

    // Next 跳转到下一个 key
    void Next();

    // Valid 是否有效，即是否已经遍历完了所有的 key，用于退出遍历
    bool Valid();

    // Key 当前遍历位置的 Key 数据
    std::string Key();

    // Value 当前遍历位置的 Value 数据
    std::string Value();

    // Close 关闭迭代器，释放相应资源
    void Close();

    void skipToNext();

    void setDB(DB::ptr db) { m_db = db;}

    void setIndexIter(IteratorInterFace::ptr indexIter) { m_indexIter = indexIter;}

    void setOptions(IteratorOptions options) { m_options = options;}

    DB::ptr& getDB() { return m_db;}
    
private:
    IteratorInterFace::ptr m_indexIter;           // 索引迭代器
    DB::ptr m_db;
    IteratorOptions m_options;
};

}

#endif