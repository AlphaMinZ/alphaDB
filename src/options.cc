#include "../inc/options.h"

namespace alphaDB {

Options DefaultOptions = {"/tmp", 256 * 1024 * 1024, false, IndexerType::Map, 0, 0.5};

IteratorOptions DefaultIteratorOptions = {"", false};

WriteBatchOptions DefaultWriteBatchOptions = {10000, true};

}