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

class Indexer {
public:
    typedef::std::shared_ptr<Indexer> ptr;

    virtual LogRecordPos::ptr Put(const std::string key, LogRecordPos::ptr pos, bool* isOk_) = 0;

    virtual LogRecordPos::ptr Get(const std::string key) = 0;

    virtual LogRecordPos::ptr Delete(std::string key, bool* isOk_) = 0;
};

Indexer::ptr NewIndexer(IndexerType typ);

}

#endif