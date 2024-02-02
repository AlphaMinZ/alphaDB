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

    int size() const { return m_map.size();}
private:
    std::map<std::string, LogRecordPos::ptr> m_map;
    alphaMin::RWMutex m_mutex;
};

}

#endif