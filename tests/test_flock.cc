#include "../inc/utils.h"
#include "../lib/inc/log.h"

#include <iostream>
#include <string>
#include <vector>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void TestFLock() {
    std::string filePath = "/home/mz/workspace/alphaDB/bin/000000000.data";

    alphaDB::FileLock::ptr fileLock(new alphaDB::FileLock(filePath));

    if (fileLock->tryLock()) {
        std::cout << "File locked successfully: " << filePath << std::endl;
        // 在这里进行需要锁定文件的操作
        fileLock->unlock();
        std::cout << "File unlocked: " << filePath << std::endl;
    } else {
        std::cout << "Failed to lock file: " << filePath << std::endl;
    }


}

int main(int argc, char** argv) {
    try {

        TestFLock();        

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}