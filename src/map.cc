#include "../inc/map.h"

namespace alphaDB {

Map::Map() {}

LogRecordPos::ptr Map::Put(const std::vector<uint8_t> key, LogRecordPos::ptr pos, bool* isOk_) {
    alphaMin::RWMutex::WriteLock lock(m_mutex);

    LogRecordPos::ptr oldLogRecordPos(new LogRecordPos);

    oldLogRecordPos = m_map[key];

    m_map[key] = pos;

    lock.unlock();

    return oldLogRecordPos;
}

LogRecordPos::ptr Map::Get(const std::vector<uint8_t> key) {
    alphaMin::RWMutex::ReadLock lock(m_mutex);
    LogRecordPos::ptr pos(new LogRecordPos);

    pos = m_map[key];
    
    lock.unlock();
    return pos;
}

LogRecordPos::ptr Map::Delete(const std::vector<uint8_t> key, bool* isOk_) {
    alphaMin::RWMutex::WriteLock lock(m_mutex);

    LogRecordPos::ptr oldLogRecordPos(new LogRecordPos);

    oldLogRecordPos = m_map[key];

    m_map.erase(key);
    lock.unlock();
    
    return oldLogRecordPos;
}

}
