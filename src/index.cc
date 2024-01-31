#include "../inc/index.h"
#include "../inc/map.h"

namespace alphaDB {

Indexer::ptr NewIndexer(IndexerType typ) {
    switch(typ) {
        case IndexerType::Map:
            return std::make_shared<Map>();
        default:
            return std::make_shared<Map>();
    }
}

}