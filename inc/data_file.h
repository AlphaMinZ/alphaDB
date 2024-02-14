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

#define DataFileNameSuffix          ".data"
#define HintFileName                "hint-index"
#define MergeFinishedFileName       "merge-finished"
#define SeqNoFileName               "seq-no"

namespace alphaDB {

// 数据文件
class DataFile {
public:
    typedef std::shared_ptr<DataFile> ptr;
    
    DataFile() {};

    LogRecord* ReadLogRecord(int64_t offset, uint32_t& size_, bool* isEOF_);

    void Write(std::string buf);

    // WriteHintRecord 写入索引信息到 hint 文件中
    void WriteHintRecord(std::string key, LogRecordPos::ptr pos);

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

// OpenDataFile 打开新的数据文件
DataFile::ptr OpenDataFile(std::string dirPath, uint32_t fileId);

// OpenHintFile 打开 Hint 索引文件
DataFile::ptr OpenHintFile(const std::string& dirPath);

// OpenMergeFinishedFile 打开标识 merge 完成的文件
DataFile::ptr OpenMergeFinishedFile(const std::string& dirPath);

// OpenSeqNoFile 存储事务序列号的文件
DataFile::ptr OpenSeqNoFile(const std::string& dirPath);

std::string GetDataFileName(const std::string& dirPath, uint32_t fileId);

DataFile::ptr NewDataFile(std::string fileName, uint32_t fileId);

}

#endif