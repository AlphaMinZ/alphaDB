#include "../inc/iterator.h"

namespace alphaDB {

// NewIterator 初始化迭代器
Iterator::ptr DB::Newiterator(IteratorOptions opts) {
    IteratorInterFace::ptr indexIter = m_index->Iterator(opts.Reverse);

    Iterator::ptr iterator(new Iterator);
    iterator->setDB(shared_from_this());
    iterator->setIndexIter(indexIter);
    iterator->setOptions(opts);

    return iterator;
}

// Rewind 重新回到迭代器的起点，即第一个数据
void Iterator::Rewind() {
    m_indexIter->Rewind();
    skipToNext();
}

// Seek 根据传入的 key 查找到第一个大于（或小于）等于的目标 key，根据从这个 key 开始遍历
void Iterator::Seek(std::string key) {
    m_indexIter->Seek(key);
    skipToNext();
}

// Next 跳转到下一个 key
void Iterator::Next() {
    m_indexIter->Next();
    skipToNext();
}

// Valid 是否有效，即是否已经遍历完了所有的 key，用于退出遍历
bool Iterator::Valid() {
    bool isValid =  m_indexIter->Valid();
    skipToNext();
    return isValid;
}

// Key 当前遍历位置的 Key 数据
std::string Iterator::Key() {
    return m_indexIter->Key();
}

// Value 当前遍历位置的 Value 数据
std::string Iterator::Value() {
    LogRecordPos::ptr logRecordPos = m_indexIter->Value();

    alphaMin::Mutex::Lock lock(getDB()->getMutex());

    std::string value = getDB()->getValueByPosition(logRecordPos);

    lock.unlock();
    return value;
}

// Close 关闭迭代器，释放相应资源
void Iterator::Close() {
    m_indexIter->Close();
}

void Iterator::skipToNext() {
    int prefixLen = m_options.Prefix.size();
    if(prefixLen == 0) {
        return;
    }

    for(; m_indexIter->Valid(); m_indexIter->Next()) {
        std::string key = m_indexIter->Key();
        if(prefixLen <= key.size() && m_options.Prefix.compare(0, m_options.Prefix.size(), key, 0, prefixLen) == 0) {
            break;
        }
    }
}

}