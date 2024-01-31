#include "../inc/data_file.h"

namespace alphaDB {

DataFile::DataFile(std::string dirPath, uint32_t fileId) {

}

LogRecord* DataFile::ReadLogRecord(int offset, uint32_t& size_, bool* isEOF_) {
    return nullptr;
}

void DataFile::Write(std::vector<uint8_t>& buf) {

}

void DataFile::Sync() {

}

}