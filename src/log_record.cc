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

void EncodeLogRecord(LogRecord* logRecoed, std::vector<uint8_t>& str_, int64_t& len_) {
    
}

}