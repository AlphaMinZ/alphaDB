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

    LogRecordPos::ptr Put(const std::vector<uint8_t> key, LogRecordPos::ptr pos) override;

    LogRecordPos::ptr Get(const std::vector<uint8_t> key) override;

    LogRecordPos::ptr Delete(std::vector<uint8_t> key) override;

    int size() const { return m_map.size();}
private:
    std::map<std::vector<uint8_t>, LogRecordPos::ptr> m_map;
    alphaMin::RWMutex m_mutex;
};

}

#endif