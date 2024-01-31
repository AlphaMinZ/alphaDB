#ifndef __ALPHA_FILE_IO_H__
#define __ALPHA_FILE_IO_H__

#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>

namespace alphaDB {

class IOMgr {
public:
    virtual int Read(std::vector<uint8_t>& buffer, std::streamsize size, std::streampos offset) = 0;

    virtual int Write(const std::vector<uint8_t>& buffer) = 0;

    virtual int Sync() = 0;

    virtual void Close() = 0;
}; 

class FileIO : public IOMgr {
public:
    FileIO() {}

    FileIO(const std::string& fileName);

    ~FileIO();

    int Read(std::vector<uint8_t>& buffer, std::streamsize size, std::streampos offset) override;

    int Write(const std::vector<uint8_t>& buffer) override;

    int Sync() override;

    void Close() override;
private:
    std::fstream m_file;
};

}

#endif