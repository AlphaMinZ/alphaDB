#include "../inc/utils.h"

namespace alphaDB {

FileLock::FileLock(const std::string& filePath)
        :m_filePath(filePath)
        ,m_fd(-1)
        ,m_dir(nullptr) {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        throw std::runtime_error("Failed to stat file: " + filePath);
    }

    if (S_ISDIR(fileStat.st_mode)) {
        m_dir = opendir(filePath.c_str());
        if (m_dir == nullptr) {
            throw std::runtime_error("Failed to open directory: " + filePath);
        }
    } else if (S_ISREG(fileStat.st_mode)) {
        m_fd = open(filePath.c_str(), O_RDWR);
        if (m_fd == -1) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }
    } else {
        throw std::runtime_error("Unsupported file type: " + filePath);
    }
}

FileLock::~FileLock() {
    if(m_fd != -1) {
        unlock();
        close(m_fd);
    } 
    if(m_dir != nullptr) {
        closedir(m_dir);
    }
}

void FileLock::lock() {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    if (fcntl(m_fd != -1 ? m_fd : dirfd(m_dir), F_SETLKW, &fl) == -1) {
        throw std::runtime_error("Failed to acquire file lock: " + m_filePath);
    }
}

bool FileLock::tryLock() {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    if (fcntl(m_fd != -1 ? m_fd : dirfd(m_dir), F_SETLK, &fl) == -1) {
        return false;
    }
    return true;
}

bool FileLock::unlock() {
    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    if (fcntl(m_fd != -1 ? m_fd : dirfd(m_dir), F_SETLKW, &fl) == -1) {
        throw std::runtime_error("Failed to release file lock: " + m_filePath);
    }
}

int64_t getFileSize(const std::string& filePath) {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        throw std::runtime_error("Failed to get file size: " + filePath);
    }
    std::cout << filePath << "的大小为 " << fileStat.st_size << "\n";
    return static_cast<int64_t>(fileStat.st_size);
}

int64_t DirSize(const std::string& dirPath) {
    int64_t size = 0;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dirPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fileName = ent->d_name;
            std::cout << fileName << "\n";
            if (fileName != "." && fileName != "..") {
                std::string filePath = dirPath + "/" + fileName;
                size += getFileSize(filePath);
            }
        }
        closedir(dir);
    } else {
        throw std::runtime_error("Failed to open directory: " + dirPath);
    }

    return size;
}

// AvailableDiskSize 获取磁盘剩余可用空间大小
uint64_t AvailableDiskSize() {
    char cwd[4096]; // 用于存储当前工作目录的缓冲区
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        throw std::runtime_error("Failed to get current working directory");
    }

    struct statvfs stat;
    if (statvfs(cwd, &stat) == -1) {
        throw std::runtime_error("Failed to get filesystem status");
    }

    return stat.f_bavail * stat.f_bsize;
}

}