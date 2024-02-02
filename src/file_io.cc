#include "../inc/file_io.h"

#include <string>
#include <stdexcept>
#include <cstring>

namespace alphaDB {

IOMgr::ptr NewIOManager(std::string fileName) {
    return std::make_shared<FileIO>(fileName);
}

FileIO::FileIO(const std::string& fileName) {
    m_file.open(fileName, std::ios::in | std::ios::out | std::ios::app);
    if(!m_file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }
}

FileIO::~FileIO() {
    if(m_file.is_open()) {
        m_file.close();
    }
}

int FileIO::Read(char* buffer, std::streamsize size, std::streampos offset) {
    m_file.seekg(offset);
    m_file.read(buffer, size);
    return m_file.gcount();
}

int FileIO::Write(const char* buffer) {
    m_file.write(buffer, strlen(buffer));
    return m_file.good() ? strlen(buffer) : -1;
}

int FileIO::Sync() {
    m_file.flush();
    return 0;
}

void FileIO::Close() {
    m_file.close();
}

int64_t FileIO::Size() {
    std::streampos currentPosition = m_file.tellg();
    m_file.seekg(0, std::ios::end);
    std::streampos size = m_file.tellg();
    m_file.seekg(currentPosition); // 恢复文件指针位置
    return (int64_t)size;
}

}