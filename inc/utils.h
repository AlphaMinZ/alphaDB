#ifndef __ALPHA_UTILS_H__
#define __ALPHA_UTILS_H__

#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>

namespace alphaDB {

class FileLock {
public:
    typedef std::shared_ptr<FileLock> ptr;

    FileLock() {}

    FileLock(const std::string& filePath);

    ~FileLock();

    void lock();

    bool tryLock();

    bool unlock();

private:
    std::string m_filePath;
    int m_fd;
    DIR* m_dir;
};

int64_t getFileSize(const std::string& filePath);

int64_t DirSize(const std::string& dirPath);

// AvailableDiskSize 获取磁盘剩余可用空间大小
uint64_t AvailableDiskSize();

}

#endif