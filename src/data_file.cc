#include "../inc/data_file.h"

#include <iostream>
#include <iomanip>
#include <sstream>

namespace alphaDB {

DataFile::ptr OpenDataFile(std::string dirPath, uint32_t fileId) {
    std::string fileName = GetDataFileName(dirPath, fileId);
    return NewDataFile(fileName, fileId);
}

// OpenHintFile 打开 Hint 索引文件
DataFile::ptr OpenHintFile(const std::string& dirPath) {
    std::string fileName = dirPath + "/" + HintFileName;
    return NewDataFile(fileName, 0);
}

// OpenMergeFinishedFile 打开标识 merge 完成的文件
DataFile::ptr OpenMergeFinishedFile(const std::string& dirPath) {
    std::string fileName = dirPath + "/" + MergeFinishedFileName;
    return NewDataFile(fileName, 0);
}

// OpenSeqNoFile 存储事务序列号的文件
DataFile::ptr OpenSeqNoFile(const std::string& dirPath) {
    std::string fileName = dirPath + "/" + SeqNoFileName;
    return NewDataFile(fileName, 0);
}

std::string GetDataFileName(const std::string& dirPath, uint32_t fileId) {
    // 格式化文件名
    std::ostringstream fileNameStream;
    fileNameStream << std::setw(9) << std::setfill('0') << fileId << DataFileNameSuffix;

    // 构建完整的文件路径
    std::string fileName = dirPath + "/" + fileNameStream.str();

    return fileName;
}

DataFile::ptr NewDataFile(std::string fileName, uint32_t fileId) {
    // 初始化 IOManager 管理器接口
    IOMgr::ptr ioManager = NewIOManager(fileName);

    DataFile::ptr dataFile(new DataFile);
    dataFile->setFileId(fileId);
    dataFile->setWriteOff((uint32_t)0);
    dataFile->setIOMgr(ioManager);

    return dataFile;
}

// ReadLogRecord 根据 offset 从数据文件中读取 LogRecord
LogRecord* DataFile::ReadLogRecord(int64_t offset, uint32_t& size_, bool* isEOF_) {
    int64_t fileSize = m_ioManager->Size();

    // 如果读取的最大 header 长度已经超过了文件的长度，则只需要读取到文件的末尾即可
    int64_t headerBytes = (int64_t)maxLogRecordHeaderSize;
    if(offset + headerBytes > fileSize) {
        headerBytes = fileSize - offset;
    }

    // 读取 Header 信息
    std::string headerBuf = readNBytes(headerBytes, offset);

    int64_t headerSize = 0;
    LogRecordHeader* header = decodeLogRecordheader(headerBuf, headerSize);
    // 下面的两个条件表示读取到了文件末尾，直接返回 EOF 错误
    if(header == nullptr) {
        *isEOF_ = true;
        return nullptr;
    }
    if(header->crc == 0 && header->keySize == 0 && header->valueSize == 0) {
        *isEOF_ = true;
        return nullptr;
    }

    // 取出对应的 key 和 value 的长度
    int64_t keySize = (int64_t)header->keySize;
    int64_t valueSize = (int64_t)header->valueSize;
    int recordSize = headerSize + keySize + valueSize;
    size_ = recordSize;

    LogRecord* logRecord = new LogRecord;
    logRecord->Type = header->recordType;
    // 开始读取用户实际存储的 key/value 数据
    if(keySize > 0 || valueSize > 0) {
        std::string kvBuf = readNBytes(keySize + valueSize, offset + headerSize);
        //	解出 key 和 value
        logRecord->Key = kvBuf.substr(0, keySize);
        logRecord->Value = kvBuf.substr(keySize);
    }

    // 校验数据的有效性
    uint32_t crc = getLogRecordCRC(logRecord, headerBuf.substr(CRC32_SIZE, headerSize));
    if(crc != header->crc) {
        throw MyErrors::ErrInvalidCRC;
    }

    return logRecord;
}

void DataFile::Write(std::string buf) {
    int n = m_ioManager->Write(buf.c_str(), buf.size());
    
    m_writeOff += (int64_t)buf.size();
}

void DataFile::WriteHintRecord(std::string key, LogRecordPos::ptr pos) {
    LogRecord* record = new LogRecord;
    record->Key = key;
    record->Value = EncodeLogRecordPos(pos);

    int64_t len;
    std::string encRecord = EncodeLogRecord(record, len);
    return Write(encRecord);
}

void DataFile::Sync() {
    m_ioManager->Sync();
}

void DataFile::Close() {
    m_ioManager->Close();
}

std::string DataFile::readNBytes(int64_t n, int64_t offset) {
    char ch[n];
    m_ioManager->Read(ch, n, offset);
    std::string ret(ch, ch + n);

    return ret;
    // std::string ret(n, '\0');
    // m_ioManager->Read(ret, n, offset);
    // return ret;
}

}