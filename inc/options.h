#ifndef __ALPHA_OPTIONS_H__
#define __ALPHA_OPTIONS_H__

#include <memory>
#include <stdint.h>

#include "index.h"

namespace alphaDB {

typedef struct _Options {
    std::string DirPath;    // 数据库数据目录
    int64_t DataFileSize;   // 数据文件大小
    bool SyncWrites;        // 每次写数据是否持久化
    IndexerType IndexType;  // 索引类型
}Options;

typedef struct _IteratorOption {
    // 遍历前缀为指定值的 Key，默认为空
    std::string Prefix;
    // 是否反向遍历，默认 false 是正向
    bool Reverse;
}IteratorOption;

}

#endif