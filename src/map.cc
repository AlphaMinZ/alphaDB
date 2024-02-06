#include "../inc/map.h"

#include <map>
#include <algorithm>

namespace alphaDB {

Map::Map() {}

LogRecordPos::ptr Map::Put(const std::string key, LogRecordPos::ptr pos, bool* isOk_) {
    alphaMin::RWMutex::WriteLock lock(m_RWMutex);

    LogRecordPos::ptr oldLogRecordPos(new LogRecordPos);

    oldLogRecordPos = m_map[key];

    m_map[key] = pos;
    
    if(isOk_ != nullptr) {
        *isOk_ = true;
    }
    lock.unlock();

    return oldLogRecordPos;
}

LogRecordPos::ptr Map::Get(const std::string key) {
    alphaMin::RWMutex::ReadLock lock(m_RWMutex);
    // LogRecordPos::ptr pos(new LogRecordPos);
    LogRecordPos::ptr pos;

    pos = m_map[key];
    
    lock.unlock();
    return pos;
}

LogRecordPos::ptr Map::Delete(const std::string key, bool* isOk_) {
    alphaMin::RWMutex::WriteLock lock(m_RWMutex);

    LogRecordPos::ptr oldLogRecordPos(new LogRecordPos);

    oldLogRecordPos = m_map[key];

    m_map.erase(key);

    if(isOk_ != nullptr) {
        *isOk_ = true;
    }
    lock.unlock();
    return oldLogRecordPos;
}

int Map::Size() {
    return m_map.size();
}

IteratorInterFace::ptr Map::Iterator(bool reverse) {
    if(m_map.empty()) {
        // return std::shared_ptr<IteratorInterFace>(); 
        IteratorInterFace::ptr iter(new mapIterator); 
        return iter;
    }
    alphaMin::Mutex::Lock lock(m_mutex);

    IteratorInterFace::ptr item = newMapIterator(m_map, reverse);

    lock.unlock();
    return item;
}

mapIterator::ptr newMapIterator(std::map<std::string, LogRecordPos::ptr>& map, bool reverse) {
    int idx = 0;
    std::vector<Item::ptr> values(map.size(), std::make_shared<Item>());

    // 将所有的数据存放到数组中
    auto it = map.begin();
    while(it != map.end()) {
        Item::ptr item(new Item(it->first, it->second)); 
        values[idx] = item;
        it++;
        idx++;
    }

    auto compareFunction = [](const Item::ptr& a, const Item::ptr& b){
        return a->key < b->key;
    };

    if(reverse) {
        std::sort(values.begin(), values.end(), std::greater<decltype(values)::value_type>{});
    } else {
        std::sort(values.begin(), values.end(), compareFunction);
    }

    mapIterator::ptr item(new mapIterator);
    item->setCurrIndex(0);
    item->setReveser(reverse);
    item->setValues(values);

    return item;
}

// Rewind 重新回到迭代器的起点，即第一个数据
void mapIterator::Rewind() {
    m_currIndex = 0;
}

// Seek 根据传入的 key 查找到第一个大于（或小于）等于的目标 key，根据从这个 key 开始遍历
void mapIterator::Seek(std::string key) {
    for(int i = 0; i < m_values.size(); ++i) {
        if(m_reveser) {
            if(m_values[i]->key <= key) {
                m_currIndex = i;
                return;
            }
        } else {
            if(m_values[i]->key >= key) {
                m_currIndex = i;
                return;
            }
        }
    }
}

// Next 跳转到下一个 key
void mapIterator::Next() {
    m_currIndex += 1;
} 

// Valid 是否有效，即是否已经遍历完了所有的 key，用于退出遍历
bool mapIterator::Valid() {
    return m_currIndex < m_values.size();
} 

// Key 当前遍历位置的 Key 数据
std::string mapIterator::Key() {
    return m_values[m_currIndex]->key;
} 

// Value 当前遍历位置的 Value 数据
LogRecordPos::ptr mapIterator::Value() {
    return m_values[m_currIndex]->pos;
} 

// Close 关闭迭代器，释放相应资源
void mapIterator::Close() {
    m_values.clear();
} 
    
}
