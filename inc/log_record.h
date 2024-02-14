#ifndef __ALPHA_LOG_RECORD_H__
#define __ALPHA_LOG_RECORD_H__

#include <stdint.h>
#include <memory>
#include <vector>
#include <string>
#include <limits>
#include <cstdint>

// crc type keySize valueSize
// 4 +  1  +  4   +   4 = 13
#define maxLogRecordHeaderSize          13
#define CRC32_SIZE                      4

namespace alphaDB {
    
// 定义 LogRecordType 别名
using LogRecordType = char;

// 定义日志记录类型的枚举
enum LogRecordTypeEnum : LogRecordType {
    LogRecordNormal,       // 表示普通的日志记录类型
    LogRecordDeleted,      // 表示已删除的日志记录类型
    LogRecordTxnFinished   // 表示事务已完成的日志记录类型
};

// LogRecord 写入到数据文件的记录
// 之所以叫日志，是因为数据文件中的数据是追加写入的，类似日志的格式
class LogRecord {
public:
    typedef std::shared_ptr<LogRecord> ptr;

    std::string Key;
    std::string Value;
    LogRecordTypeEnum Type;
};

// LogRecord 的头部信息
class LogRecordHeader {
public:
    typedef std::shared_ptr<LogRecordHeader> ptr;

    uint32_t crc;                   // crc 校验值
    LogRecordTypeEnum recordType;   // 标识 LogRecord 的类型
    uint32_t keySize;               // key 的长度
    uint32_t valueSize;             // value 的长度
};

// LogRecordPos 数据内存索引，主要是描述数据在磁盘上的位置
class LogRecordPos {
public:
    typedef std::shared_ptr<LogRecordPos> ptr;

    uint32_t fId;   // 文件ID 表示将数据存储到那个文件当中
    int64_t offset; // 文件偏移
    uint32_t size;  // 标识条数据在磁盘上的大小

    LogRecordPos() {};

    LogRecordPos(LogRecordPos* tmp); 

    LogRecordPos(uint32_t Fid, int64_t Offset); 

    LogRecordPos(uint32_t Fid, int64_t Offset, uint32_t Size);
}; 

// TransactionRecord 暂存的事务相关的数据
class TransactionRecord {
public:
    typedef std::shared_ptr<TransactionRecord> ptr;

    LogRecord* Record;
    LogRecordPos::ptr Pos;
};

// EncodeLogRecord 对 LogRecord 进行编码，返回字节数组及长度
std::string EncodeLogRecord(LogRecord* logRecoed, int64_t& len_);

// EncodeLogRecordPos 对位置信息进行编码
std::string EncodeLogRecordPos(LogRecordPos::ptr pos);

// DecodeLogRecordPos 解码 LogRecordPos
LogRecordPos::ptr DecodeLogRecordPos(std::string buf);

// 对字节数组中的 Header 信息进行解码
LogRecordHeader* decodeLogRecordheader(std::string buf, int64_t& headerSize_);

uint32_t getLogRecordCRC(LogRecord* lr, std::string header);

}

#endif