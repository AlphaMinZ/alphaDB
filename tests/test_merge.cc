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
    opts.DataFileMergeRatio = 0;

    auto db = alphaDB::Open(opts);

    for(int i = 0; i < 50; ++i) {
        db->Put(std::to_string(i), std::to_string(i));
    }

    db->Merge();

    // 重启校验
    db->Close();

    auto db2 = alphaDB::Open(opts);

    auto keys = db2->ListKeys();

    if(keys.size() == 50) {
        std::cout << "ListKeys is true\n";
    }

    for(int i = 0; i < 50; i++) {
        auto val = db2->Get(std::to_string(i));
        std::cout << val << "\n";
    }

}

// 有失效数据
void Test_Merge3() {
    auto opts = alphaDB::DefaultOptions;
    opts.DirPath = "/home/mz/workspace/alphaDB/bin/data_3";
    opts.DataFileSize = 32 * 1024 * 1024;
    opts.DataFileMergeRatio = 0;

    auto db = alphaDB::Open(opts);

    for(int i = 0; i < 50; ++i) {
        db->Put(std::to_string(i), std::to_string(i));
    }

    for(int i = 1; i < 50; ++i) {
        db->Delete(std::to_string(i));
    }

    db->Merge();

    // 重启校验
    db->Close();

    std::cout << "---------------------\n";

    auto db2 = alphaDB::Open(opts);

    std::cout << "=====================\n";

    auto keys = db2->ListKeys();

    std::cout << "+++++++++++++++++++++\n";

    if(keys.size() == 40) {
        std::cout << "ListKeys is true\n";
    }

    for(int i = 0; i < 40; i++) {
        auto val = db2->Get(std::to_string(i));
        std::cout << val << "\n";
    } 

}

void Test() {
    // uint64_t availableSize = alphaDB::AvailableDiskSize();
    // std::cout << "Available disk space: " << availableSize << " bytes" << std::endl;

    // std::cout << alphaDB::readDirectory("/home/mz/workspace/alphaDB/bin/data").size();
    // for(const auto& fileName : mergeFileNames) {
    //     std::string srcPath = mergePath + "/" + fileName;
    //     std::string destPath = m_options.DirPath + "/" + fileName;

    //     if (std::rename(srcPath.c_str(), destPath.c_str()) != 0) {
    //         std::cerr << "移动文件 " << fileName << " 出错" << std::endl;
    //         removeDirectory(mergePath);
    //         return;
    //     }
    // }
    // std::string srcPath = "/home/mz/workspace/alphaDB/bin/data_2/aa/111";
    // std::string destPath = "/home/mz/workspace/alphaDB/bin/data_2/111";
    //     if (std::rename(srcPath.c_str(), destPath.c_str()) != 0) {
    //         std::cerr << "移动文件出错" << std::endl;
    //         return;
    //     }

    auto is = alphaDB::directoryExists("/home/mz/workspace/alphaDB/bin/data_2/000000003.data");
    if(is) {
        std::cout << "true\n";
    } else {
        std::cout << "false\n";
    }
}

int main(int argc, char** argv) {
    try {
        
        // Test_Merge();
        // Test();
        // Test_Merge2();
        Test_Merge3();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}
