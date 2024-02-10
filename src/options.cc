#include "../inc/options.h"

namespace alphaDB {

Options DefaultOptions = {"/tmp", 256 * 1024 * 1024, false, IndexerType::Map};

IteratorOptions DefaultIteratorOptions = {"", false};

WriteBatchOptions DefaultWriteBatchOptions = {10000, true};

}