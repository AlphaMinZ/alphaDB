#ifndef __ALPHA_DATA_FILE_H__
#define __ALPHA_DATA_FILE_H__

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include <cstdint>
#include <limits>

#include "errors.h"
#include "log_record.h"
#include "file_io.h"

namespace alphaDB {

// 数据文件
class DataFile {
public:
    typedef std::shared_ptr<DataFile> ptr;
    
    DataFile() {};

    LogRecord* ReadLogRecord(int64_t offset, uint32_t& size_, bool* isEOF_);

    void Write(std::string& buf);

    void Sync();

    void Close();

    std::string readNBytes(int64_t n, int64_t offset);

    uint32_t getFileId() const { return m_fileId;}

    int64_t getWriteOff() const { return m_writeOff;}

    void setFileId(uint32_t fileId) { m_fileId = fileId;}

    void setIOMgr(IOMgr::ptr ioMgr) { m_ioManager = ioMgr;}

    void setWriteOff(int64_t writeOff) { m_writeOff = writeOff;}

private:
    uint32_t m_fileId;          // 文件ID
    int64_t m_writeOff;         // 文件写到了哪个位置
    IOMgr::ptr m_ioManager;          // io读写管理    
};

DataFile::ptr OpenDataFile(std::string dirPath, uint32_t fileId);

}

#endif