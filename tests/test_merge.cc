#include "../inc/db.h"
#include "../inc/file_io.h"
#include "../lib/inc/log.h"
#include "../inc/utils.h"

#include <iostream>
#include <string>
#include <vector>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

// 没有任何数据的情况下进行 merge
void Test_Merge() {
    auto opts = alphaDB::DefaultOptions;
    opts.DirPath = "/home/mz/workspace/alphaDB/bin/data";
    auto db = alphaDB::Open(opts);
    db->Merge();
}

// 全部都是有效的数据
void Test_Merge2() {
    auto opts = alphaDB::DefaultOptions;
    opts.DirPath = "/home/mz/workspace/alphaDB/bin/data_2";
    opts.DataFileSize = 32 * 1024 * 1024;
    // opts.
    auto db = alphaDB::Open(opts);
}

void Test() {
    uint64_t availableSize = alphaDB::AvailableDiskSize();
    std::cout << "Available disk space: " << availableSize << " bytes" << std::endl;

    std::cout << alphaDB::readDirectory("/home/mz/workspace/alphaDB/bin/data").size();
}

int main(int argc, char** argv) {
    try {
        
        // Test_Merge();
        Test();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}
