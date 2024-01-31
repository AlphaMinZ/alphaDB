#ifndef __ALPHA_LOG_RECORD_H__
#define __ALPHA_LOG_RECORD_H__

#include <stdint.h>
#include <memory>
#include <vector>

namespace alphaDB {
    
// 定义 LogRecordType 别名
using LogRecordType = char;

// 定义日志记录类型的枚举
enum LogRecordTypeEnum : LogRecordType {
    LogRecordNormal,       // 表示普通的日志记录类型
    LogRecordDeleted,      // 表示已删除的日志记录类型
    LogRecordTxnFinished   // 表示事务已完成的日志记录类型
};

typedef struct _LogRecord {
    std::vector<uint8_t> Key;
    std::vector<uint8_t> Value;
    LogRecordTypeEnum Type;
}LogRecord;

class LogRecordPos {
public:
    typedef std::shared_ptr<LogRecordPos> ptr;

    uint32_t fId;   // 文件ID 表示将数据存储到那个文件当中
    int64_t offset; // 文件偏移
    uint32_t size;  // 标识条数据在磁盘上的大小

    LogRecordPos() {};

    LogRecordPos(LogRecordPos* tmp); 

    LogRecordPos(uint32_t Fid, int64_t Offset); 
}; 

void EncodeLogRecord(LogRecord* logRecoed, std::vector<uint8_t>& str_, int64_t& len_);

}

#endif