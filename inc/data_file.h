#ifndef __ALPHA_DATA_FILE_H__
#define __ALPHA_DATA_FILE_H__

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#include "errors.h"
#include "log_record.h"
#include "file_io.h"

namespace alphaDB {

// 数据文件
class DataFile {
public:
    typedef std::shared_ptr<DataFile> ptr;
    
    DataFile() {};

    DataFile(std::string dirPath, uint32_t fileId);

    LogRecord* ReadLogRecord(int offset, uint32_t& size_, bool* isEOF_);

    void Write(std::vector<uint8_t>& buf);

    void Sync();

    uint32_t getFileId() const { return m_fileId;}

    int64_t getWriteOff() const { return m_writeOff;}

    void setWriteOff(int64_t writeOff) { m_writeOff = writeOff;}

private:
    uint32_t m_fileId;          // 文件ID
    int64_t m_writeOff;         // 文件写到了哪个位置
    IOMgr* m_ioManager;          // io读写管理    
};

}

#endif