#include "../inc/file_io.h"

#include <string>
#include <stdexcept>

namespace alphaDB {

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

int FileIO::Read(std::vector<uint8_t>& buffer, std::streamsize size, std::streampos offset) {
    m_file.seekg(offset);
    buffer.resize(size);
    m_file.read(reinterpret_cast<char*>(buffer.data()), size);
    return m_file.gcount();
}

int FileIO::Write(const std::vector<uint8_t>& buffer) {
    m_file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    return m_file.good() ? static_cast<int>(buffer.size()) : -1;
}

int FileIO::Sync() {
    m_file.flush();
    return 0;
}

void FileIO::Close() {
    m_file.close();
}

//     std::streampos Size() {
//         std::streampos currentPosition = file.tellg();
//         file.seekg(0, std::ios::end);
//         std::streampos size = file.tellg();
//         file.seekg(currentPosition); // 恢复文件指针位置
//         return size;
//     }

}