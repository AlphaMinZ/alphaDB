#include "../inc/log_record.h"
#include <stdint.h>
#include <string>

namespace alphaDB {

LogRecordPos::LogRecordPos(LogRecordPos* tmp) {
    fId = tmp->fId;
    offset = tmp->offset;
    size = tmp->size;
}

LogRecordPos::LogRecordPos(uint32_t Fid, int64_t Offset) {
    fId = Fid;
    offset = Offset;
}

std::string EncodeLogRecord(LogRecord* logRecoed, int64_t& len_) {
    
}

// 对字节数组中的 Header 信息进行解码
logRecordHeader* decodeLogRecordheader(std::string buf, int64_t& headerSize_) {

}

uint32_t getLogRecordCRC(LogRecord* lr, std::string header) {

}

}