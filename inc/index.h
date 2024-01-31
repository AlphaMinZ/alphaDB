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

    virtual LogRecordPos::ptr Put(const std::vector<uint8_t> key, LogRecordPos::ptr pos) = 0;

    virtual LogRecordPos::ptr Get(const std::vector<uint8_t> key) = 0;

    virtual LogRecordPos::ptr Delete(std::vector<uint8_t> key) = 0;
};

Indexer::ptr NewIndexer(IndexerType typ);

}

#endif