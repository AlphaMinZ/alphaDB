#include "../inc/db.h"
#include "../inc/errors.h"
#include "../inc/batch.h"
#include "../inc/data_file.h"
#include "../inc/utils.h"

#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>

namespace alphaDB {

const std::string mergeDirName = "-merge";
const std::string mergeFinishedKey = "merge.finished";

bool directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0 || (info.st_mode & S_IFREG) != 0;
}

bool createDirectory(const std::string& path) {
    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
        std::cerr << "Error creating directory" << std::endl;
        return false;
    }
    return true;
}

bool removeDirectory(const std::string& path) {
    std::string command = "rm -r " + path; // Linux/Unix系统下的删除目录命令
    // std::string command = "rmdir /s /q " + path; // Windows系统下的删除目录命令

    int result = std::system(command.c_str());

    if (result != 0) {
        std::cerr << "Error removing directory!" << std::endl;
        return false;
    }
    return true;
}

// Merge 清理无效数据，生成 Hint 文件
void DB::Merge() {
    // 如果数据库为空，则直接返回
    if(m_activeFile.get() == nullptr) {
        return;
    }
    alphaMin::Mutex::Lock lock(m_mutex);
    // 如果 merge 正在进行当中，则直接返回
    if(m_isMerging) {
        lock.unlock();
        throw MyErrors::ErrMergeIsProgress;
    }

    // 查看可以 merge 的数据量是否达到了阈值
    auto totalSize = DirSize(m_options.DirPath); 
    if((float)m_reclaimSize / (float)totalSize < m_options.DataFileMergeRatio) {
        lock.unlock();
        throw MyErrors::ErrMergeRatioUnreached;
    }

    // 有点问题 在删除数据之后读取的文件大小会为0 描述的不清楚
    // auto availableDiskSize = AvailableDiskSize();
    // if((uint64_t)(totalSize - m_reclaimSize) >= availableDiskSize) {
    //     std::cout << totalSize << " " << m_reclaimSize << " " << totalSize - m_reclaimSize << " " << availableDiskSize << "\n";
    //     lock.unlock();
    //     throw MyErrors::ErrNoEnoughSpaceForMerge;
    // }

    m_isMerging = true;
    
    // 将当前活跃文件转换为旧的数据文件
    m_olderFiles[m_activeFile->getFileId()] = m_activeFile;

    // 打开新的活跃文件
    setActiveDataFileLocked();

    // 记录最近没有参与 merge 的文件 id
    uint32_t nonMergeFileId = m_activeFile->getFileId();

    // 取出所有需要 merge 的文件
    std::vector<DataFile::ptr> mergeFiles;
    for(const auto& pair : m_olderFiles) {
        mergeFiles.push_back(pair.second);
    }
    lock.unlock();

    //	待 merge 的文件从小到大进行排序，依次 merge
    auto compareFunction = [](const DataFile::ptr& a, const DataFile::ptr& b){
        return a->getFileId() < b->getFileId();
    };
    std::sort(mergeFiles.begin(), mergeFiles.end(), compareFunction);

    std::string mergePath = getMergePath();

    // 如果目录存在，说明发生过 merge，将其删除掉
    if(directoryExists(mergePath)) {
        if(!removeDirectory(mergePath)) {
            m_isMerging = false;
            return;
        }
    }

    // 新建一个 merge path 的目录
    if(!createDirectory(mergePath)) {
        return;
    }

    // 打开一个新的临时 bitcask 实例
    Options mergeOptions = m_options;
    mergeOptions.DirPath = mergePath;
    mergeOptions.SyncWrites = false;
    DB::ptr mergeDB = Open(mergeOptions);

    // 打开 hint 文件存储索引
    DataFile::ptr hintFile = OpenHintFile(mergePath);

    // 遍历处理每个数据文件
    for(int i = 0; i < mergeFiles.size(); ++i) {
        int64_t offset = 0;
        for(;;) {
            uint32_t size = 0;
            bool is_EOF = false;
            LogRecord* logRecord = mergeFiles[i]->ReadLogRecord(offset, size, &is_EOF);
            if(is_EOF) {
                break;
            }
            // 解析拿到实际的 key
            uint64_t seqNo;
            std::string realKey = parseLogRecordKey(logRecord->Key, seqNo);
            LogRecordPos::ptr logRecordPos = getIndex()->Get(realKey);
            // 和内存中的索引位置进行比较，如果有效则重写
            if(logRecordPos.get() != nullptr &&
                logRecordPos->fId == mergeFiles[i]->getFileId() &&
                logRecordPos->offset == offset) {
                // 清除事务标记
                logRecord->Key = logRecordKeyWithSeq(realKey, nonTransactionSeqNo);
                LogRecordPos::ptr pos = mergeDB->appendLogRecord(logRecord);
                hintFile->WriteHintRecord(realKey, pos);
            }
            // 增加 offset
            offset += size;
        }
    }

    // sync 保证持久化
    hintFile->Sync();
    mergeDB->Sync();

    // 写标识 merge 完成的文件
    DataFile::ptr mergeFinishedFile = OpenMergeFinishedFile(mergePath);
    LogRecord* mergeFinRecord = new LogRecord;
    mergeFinRecord->Key = mergeFinishedKey;
    mergeFinRecord->Value = std::to_string(nonMergeFileId);

    int64_t len;
    std::string enRecord = EncodeLogRecord(mergeFinRecord, len);
    mergeFinishedFile->Write(enRecord);
    mergeFinishedFile->Sync();

    m_isMerging = false;
    return;
}

std::string DB::getMergePath() {
    return m_options.DirPath + "/" + mergeDirName;
}

// 加载 merge 数据目录
void DB::loadMergeFiles() {
    std::string mergePath = getMergePath();
    // merge 目录不存在的话直接返回
    if(!directoryExists(mergePath)) {
        return;
    }

    std::vector<std::string> dirEntries = readDirectory(mergePath);

    // 查找标识 merge 完成的文件，判断 merge 是否处理完了
    bool mergeFinished;
    std::vector<std::string> mergeFileNames;

    for(int i = 0; i < dirEntries.size(); ++i) {
        if(dirEntries[i] == MergeFinishedFileName) {
            mergeFinished = true;
        }
        if(dirEntries[i] == SeqNoFileName) {
            continue;
        }
        mergeFileNames.push_back(dirEntries[i]);
    }

    // 没有 merge 完成则直接返回
    if(!mergeFinished) {
        removeDirectory(mergePath);
        return;
    }

    auto nonMergeFileId = getNonMergeFileId(mergePath);

    // 删除旧的数据文件
    uint32_t fileId = 0;
    for(; fileId < nonMergeFileId; ++fileId) {
        auto fileName = GetDataFileName(m_options.DirPath, fileId);
        if(directoryExists(fileName)) {
            removeDirectory(fileName);
        }
    }

    // 将新的数据文件移动到数据目录中
    // for _, fileName := range mergeFileNames {
	// 	srcPath := filepath.Join(mergePath, fileName)
	// 	destPath := filepath.Join(db.options.DirPath, fileName)
	// 	if err := os.Rename(srcPath, destPath); err != nil {
	// 		return err
	// 	}
	// }
    for(const auto& fileName : mergeFileNames) {
        std::string srcPath = mergePath + "/" + fileName;
        std::string destPath = m_options.DirPath + "/" + fileName;

        if(fileName != "." && fileName != "..") {
            if (std::rename(srcPath.c_str(), destPath.c_str()) != 0) {
                std::cerr << "移动文件 " << fileName << " 出错" << std::endl;
                removeDirectory(mergePath);
                return;
            }
        } 
    }

    removeDirectory(mergePath);
    return;
}

uint32_t DB::getNonMergeFileId(std::string dirPath) {
    auto mergeFinishedFile = OpenMergeFinishedFile(dirPath);
    uint32_t size;
    bool is_EOF;
    auto record = mergeFinishedFile->ReadLogRecord(0, size, &is_EOF);
    auto nonMergeFileId = std::stoi(record->Value);

    return (uint32_t)nonMergeFileId;
}

// 从 hint 文件中加载索引
void DB::loadIndexFromHintFile() {
    // 查看 hint 索引文件是否存在
    auto hintFileName = m_options.DirPath + "/" + HintFileName;
    if(!directoryExists(hintFileName)) {
        std::cout << "Hint 文件不存在\n";
        return;
    }

    //	打开 hint 索引文件
    auto hintFile = OpenHintFile(m_options.DirPath);

    // 读取文件中的索引
    int64_t offset = 0;
    for(;;) {
        uint32_t size;
        bool is_EOF;
        auto logRecord = hintFile->ReadLogRecord(offset, size, &is_EOF);
        if(is_EOF) {
            break;
        }

        // 解码拿到实际的位置索引
        auto pos = DecodeLogRecordPos(logRecord->Value);
        getIndex()->Put(logRecord->Key, pos, nullptr);
        offset += size;
    }

    std::cout << "index.size() = " << getIndex()->Size() << "\n";
    return;
}

}