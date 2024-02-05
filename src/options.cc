#include "../inc/options.h"

namespace alphaDB {

Options DefaultOptions = {"/tmp", 256 * 1024 * 1024, false, IndexerType::Map};

IteratorOption DefaultIteratorOptions = {"", false};

}