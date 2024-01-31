#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "../inc/db.h"
#include "../lib/inc/mutex.h"

namespace alphaDB {

// Open 打开 bitcask 存储引擎实例
DB::ptr Open(Options options) {
    // 对用户传入的配置项进行校验
    checkOptions(options);

    // 判断数据目录是否存在，如果不存在的话，则创建这个目录
    struct stat st;
    if(stat(options.DirPath.c_str(), &st) != 0) {
        // 如果目录不存在，则创建目录
        if (mkdir(options.DirPath.c_str(), 0700) != 0) {
            perror("Error creating directory");
        }
    }

    // 初始化 DB 实体
    DB::ptr db;
    {
        db->setOptions(options);
        db->setIndex(NewIndexer(options.IndexType));
    }

    // 加载数据文件
    db->loadDataFiles();

    // 从数据文件中加载索引
    db->loadIndexFromDataFiles();

    return db;
}

// Put 写入 Key/Value 数据，key 不能为空
void DB::Put(std::vector<uint8_t> key, std::vector<uint8_t> value) {
    // 判断 key 是否有效
    if(key.size() == 0) {
        throw MyErrors::ErrKeyIsEmpty;
    }

    // 构造 LogRecord 结构体
    LogRecord* logRecord = new LogRecord{key, value, LogRecordNormal};

    // 追加写入到当前数据活跃文件当中
    LogRecordPos::ptr pos = appendLogRecord(logRecord);

    // 更新内存索引
    m_index->Put(key, pos);
}

// Get 根据 key 读取数据
std::vector<uint8_t> DB::Get(std::vector<uint8_t> key) {
    alphaMin::RWMutex::ReadLock lock(m_RWMutex);

    // 判断 key 的有效性
    if(key.size() == 0) {
        lock.unlock();
        throw MyErrors::ErrKeyIsEmpty;
    }

    // 从内存数据结构中取出 key 对应的索引信息
    LogRecordPos::ptr logRecordPos = m_index->Get(key);
    // 如果 key 不在内存索引中，说明 key 不存在
    if(logRecordPos.get() == nullptr) {
        lock.unlock();
        throw MyErrors::ErrKeyNotFound;
    }

    // 根据文件 id 找到对应的数据文件
    DataFile::ptr dataFile;
    if(m_activeFile->getFileId() == logRecordPos->fId) {
        dataFile = m_activeFile;
    } else {
        dataFile = m_olderFiles[logRecordPos->fId];
    }
    // 数据文件为空
    if(dataFile.get() == nullptr) {
        lock.unlock();
        throw MyErrors::ErrDataFileNotFound;
    }

    // 根据偏移读取对应的数据
    uint32_t size;
    LogRecord* logRecord = dataFile->ReadLogRecord(logRecordPos->offset, size, nullptr);

    if(logRecord->Type == LogRecordDeleted) {
        lock.unlock();
        throw MyErrors::ErrKeyNotFound;
    }

    lock.unlock();
    return logRecord->Value;
}

// 追加写数据到活跃文件中
LogRecordPos::ptr DB::appendLogRecord(LogRecord* logRecord) {
    alphaMin::Mutex::Lock lock(m_mutex);

    // 判断当前活跃数据文件是否存在，因为数据库在没有写入的时候是没有文件生成的
	// 如果为空则初始化数据文件
    if(m_activeFile.get() == nullptr) {
        setActiveDataFileLocked();
    }

    // 写入数据编码
    std::vector<uint8_t> encRecord;
    int64_t size;
    EncodeLogRecord(logRecord, encRecord, size);
    // 如果写入的数据已经到达了活跃文件的阈值，则关闭活跃文件，并打开新的文件
    if((m_activeFile->getWriteOff() + size) > m_options.DataFileSize) {
        // 先持久化数据文件，保证已有的数据持久到磁盘当中
        m_activeFile->Sync();

        // 当前活跃文件转换为旧的数据文件
        m_olderFiles[m_activeFile->getFileId()] = m_activeFile;

        // 打开新的数据文件
        setActiveDataFileLocked();
    }

    int64_t writeOff = m_activeFile->getWriteOff();
    m_activeFile->Write(encRecord);
    
    // 根据用户配置决定是否持久化
    if(m_options.SyncWrites) {
        m_activeFile->Sync();
    }

    // 构建内存索引信息
    LogRecordPos::ptr pos(new LogRecordPos(m_activeFile->getFileId(), writeOff));

    lock.unlock();
    return pos; 
}

// 设置当前活跃文件
// 在访问此方法前必须持有互斥锁
void DB::setActiveDataFileLocked() {
    uint32_t initialFileId = 0;
    if(m_activeFile.get() != nullptr) {
        initialFileId = m_activeFile->getFileId() + 1;
    }
    // 打开新的数据文件
    DataFile::ptr dataFile(new DataFile(m_options.DirPath, initialFileId));
    m_activeFile = dataFile;
}

std::vector<std::string> readDirectory(const std::string& dirPath) {
    std::vector<std::string> dirEntries;

    // 打开目录
    DIR* dir = opendir(dirPath.c_str());
    if(dir == nullptr) {
        throw MyErrors::ErrOpenDirectoryFailed;
        return dirEntries;
    }

    // 读取目录中的所有条目
    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr) {
        // 只处理普通文件和子目录
        if(entry->d_type == DT_REG || entry->d_type == DT_DIR) {
            dirEntries.push_back(entry->d_name);
        }
    }

    // 关闭目录
    closedir(dir);
    return dirEntries;
}

std::vector<int> extractFileIds(const std::vector<std::string>& dirEntries, const char* suffix) {
    std::vector<int> fileIds;

    for(const auto& entry : dirEntries) {
        // 只处理以指定后缀结尾的文件
        if(entry.find(suffix) != std::string::npos) {
            // 使用 stringstream 拆分文件名
            std::vector<std::string> splitNames;
            std::istringstream iss(entry);
            std::string token;
            while (std::getline(iss, token, '.')) {
                splitNames.push_back(token);
            }

            try {
                int fileId = std::stoi(splitNames[0]);
                fileIds.push_back(fileId);
            } catch (const std::invalid_argument& e) {
                throw MyErrors::ErrDataDirectoryCorrupted;
                return fileIds;
            }
        }
    }

    return fileIds;
}

#define DataFileNameSuffix      ".data"

// 从磁盘中加载数据文件
void DB::loadDataFiles() {
    auto dirEntries = readDirectory(m_options.DirPath);

    std::vector<int> fileIds;
    // 遍历目录中的所有文件，找到所有以 .data 结尾的文件
    fileIds = extractFileIds(dirEntries, DataFileNameSuffix);

    //	对文件 id 进行排序，从小到大依次加载
    std::sort(fileIds.begin(), fileIds.end());
    m_fileIds = fileIds;

    // 遍历每个文件id，打开对应的数据文件
    for(int i = 0; i < fileIds.size(); ++i) {
        int fid = fileIds[i];

        DataFile::ptr dataFile(new DataFile(m_options.DirPath, (uint32_t)fid));
        if(i == fileIds.size() - 1) {
            // 最后一个，id是最大的，说明是当前活跃文件
            m_activeFile = dataFile;
        } else {
            m_olderFiles[(uint32_t)fid] = dataFile;
        }
    }
}

// 从数据文件中加载索引
// 遍历文件中的所有记录，并更新到内存索引中
void DB::loadIndexFromDataFiles() {
    // 没有文件，说明数据库是空的，直接返回
    if(m_fileIds.size() == 0) {
        return;
    }

    // 遍历所有文件id，处理文件中的记录
    for(int i = 0; i < m_fileIds.size(); ++i) {
        uint32_t fileId = (uint32_t)m_fileIds[i];
        DataFile::ptr dataFile;
        if(fileId == m_activeFile->getFileId()) {
            dataFile = m_activeFile;
        } else {
            dataFile = m_olderFiles[fileId];
        }

        int64_t offset = 0;
        LogRecord* logRecord;
        while(true) {
            uint32_t size = 0;
            bool isEOF;
            logRecord = dataFile->ReadLogRecord(offset, size, &isEOF);
            if(isEOF) {
                break;
            }

            // 构造内存索引并保存
            LogRecordPos::ptr logRecordPos(new LogRecordPos(fileId, offset));
            if(logRecord->Type == LogRecordDeleted) {
                m_index->Delete(logRecord->Key);
            } else {
                m_index->Put(logRecord->Key, logRecordPos);
            }

            // 递增 offset，下一次从新的位置开始读取
            offset += size;
        }

        // 如果是当前活跃文件，更新这个文件的 WriteOff
        if(i == m_fileIds.size() - 1) {
            m_activeFile->setWriteOff(offset);
        }
    }
}

void checkOptions(Options options) {
    if(options.DirPath == "") {
        throw std::runtime_error("database dir path is empty");
    }
    if(options.DataFileSize <= 0) {
        throw std::runtime_error("database data file size must be greater than 0");
    }
}

}