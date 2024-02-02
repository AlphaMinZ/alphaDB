#ifndef __ALPHA_FILE_IO_H__
#define __ALPHA_FILE_IO_H__

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <fstream>

namespace alphaDB {

class IOMgr {
public:
    typedef std::shared_ptr<IOMgr> ptr;

    virtual int Read(char* buffer, std::streamsize size, std::streampos offset) = 0;

    virtual int Write(const char* buffer) = 0;

    virtual int Sync() = 0;

    virtual void Close() = 0;

    virtual int64_t Size() = 0;
}; 

class FileIO : public IOMgr {
public:
    typedef std::shared_ptr<FileIO> ptr;

    FileIO() {}

    FileIO(const std::string& fileName);

    ~FileIO();

    int Read(char* buffer, std::streamsize size, std::streampos offset) override;

    int Write(const char* buffer) override;

    int Sync() override;

    void Close() override;

    int64_t Size() override;
private:
    std::fstream m_file;
};

IOMgr::ptr NewIOManager(std::string fileName);

}

#endif