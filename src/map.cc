#include "../inc/map.h"

namespace alphaDB {

Map::Map() {}

LogRecordPos::ptr Map::Put(const std::string key, LogRecordPos::ptr pos, bool* isOk_) {
    alphaMin::RWMutex::WriteLock lock(m_mutex);

    LogRecordPos::ptr oldLogRecordPos(new LogRecordPos);

    oldLogRecordPos = m_map[key];

    m_map[key] = pos;
    
    *isOk_ = true;
    lock.unlock();

    return oldLogRecordPos;
}

LogRecordPos::ptr Map::Get(const std::string key) {
    alphaMin::RWMutex::ReadLock lock(m_mutex);
    LogRecordPos::ptr pos(new LogRecordPos);

    pos = m_map[key];
    
    lock.unlock();
    return pos;
}

LogRecordPos::ptr Map::Delete(const std::string key, bool* isOk_) {
    alphaMin::RWMutex::WriteLock lock(m_mutex);

    LogRecordPos::ptr oldLogRecordPos(new LogRecordPos);

    oldLogRecordPos = m_map[key];

    m_map.erase(key);

    *isOk_ = true;
    lock.unlock();
    return oldLogRecordPos;
}

}
