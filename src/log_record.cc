#include "../inc/log_record.h"
#include <stdint.h>
#include <iostream>
#include <string>
#include <cstring>
#include <zlib.h>
#include <algorithm>

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

// EncodeLogRecord 对 LogRecord 进行编码，返回字节数组及长度
//
//	+-------------+-------------+-------------+--------------+-------------+--------------+
//	| crc 校验值  |  type 类型   |    key size |   value size |      key    |      value   |
//	+-------------+-------------+-------------+--------------+-------------+--------------+
//	    4字节          1字节          4字节          4字节           变长           变长
std::string EncodeLogRecord(LogRecord* logRecoed, int64_t& len_) {
    std::string header(maxLogRecordHeaderSize, '\0');

    // 第五个字节存储 Type
	header[4] = logRecoed->Type;
    int32_t index = 5;
    // 5 字节之后，存储的是 key 和 value 的长度信息
    int32_t keyLength = static_cast<int32_t>(logRecoed->Key.size());
    int32_t valueLength = static_cast<int32_t>(logRecoed->Value.size());
    std::memcpy(&header[index], &keyLength, sizeof(uint32_t));
    index += sizeof(uint32_t);
    std::memcpy(&header[index], &valueLength, sizeof(uint32_t));
    index += sizeof(uint32_t);

    int64_t size = index + keyLength + valueLength;
    std::string encBytes(size, '\0');

    // 将 header 部分的内容拷贝过来
    std::copy(header.begin(), header.begin() + index, encBytes.begin());
    // 将 key 和 value 数据拷贝到字节数组中
    std::copy(logRecoed->Key.begin(), logRecoed->Key.end(), encBytes.begin() + index);
    std::copy(logRecoed->Value.begin(), logRecoed->Value.end(), encBytes.begin() + index + logRecoed->Key.size());

    // 对整个 LogRecord 的数据进行 crc 校验
    size_t crcStartIndex = 4;
    // 计算 CRC32
    uint32_t crc = crc32(0, reinterpret_cast<const Bytef*>(&encBytes[crcStartIndex]), encBytes.size() - crcStartIndex);
     // 将 CRC32 写入 encBytes 的前 4 个字节
    std::copy(reinterpret_cast<char*>(&crc), reinterpret_cast<char*>(&crc) + sizeof(uint32_t), encBytes.begin());

    len_ = size;

    std::cerr << "Before return, index: " << index << ", keyLength: " << keyLength << ", valueLength: " << valueLength << std::endl;
    return encBytes;
}

// 对字节数组中的 Header 信息进行解码
LogRecordHeader* decodeLogRecordheader(std::string buf, int64_t& headerSize_) {
    if(buf.size() <= 4) {
        headerSize_ = 0;
        return nullptr;
    }

    LogRecordHeader* header = new LogRecordHeader;
    uint32_t crc;
    std::memcpy(&crc, buf.data(), sizeof(uint32_t));
    header->crc = crc;
    header->recordType = (LogRecordTypeEnum)buf[4];

    int64_t index = 5;
    // 取出实际的 key size
    uint64_t keySize;
    std::memcpy(&keySize, &buf[index], sizeof(uint32_t));
    header->keySize = keySize;
    index += 4;

    // 取出实际的 value size
    uint64_t valueSize;
    std::memcpy(&valueSize, &buf[index], sizeof(uint32_t));
    header->valueSize = valueSize;
    index += 4;

    headerSize_ = index;
    return header;
}

uint32_t getLogRecordCRC(LogRecord* lr, std::string header) {
    if(lr == nullptr) {
        return 0;
    }

    uint32_t crc = crc32(0, reinterpret_cast<const Bytef*>(header.data()), header.size());
    crc = crc32(crc, reinterpret_cast<const Bytef*>(lr->Key.data()), lr->Key.size());
    crc = crc32(crc, reinterpret_cast<const Bytef*>(lr->Value.data()), lr->Value.size());

    return crc;
}

}