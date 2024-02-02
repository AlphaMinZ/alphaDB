#include "../inc/data_file.h"
#include "../lib/inc/log.h"

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void TestOpenDataFile() {
    alphaDB::DataFile::ptr dataFile = alphaDB::OpenDataFile("/home/mz/workspace/alphaDB/bin", 0);
    alphaDB::DataFile::ptr dataFile2 = alphaDB::OpenDataFile("/home/mz/workspace/alphaDB/bin", 111);
    alphaDB::DataFile::ptr dataFile3 = alphaDB::OpenDataFile("/home/mz/workspace/alphaDB/bin", 222);
}

void TestDataFile_Write() {
    alphaDB::DataFile::ptr dataFile = alphaDB::OpenDataFile("/home/mz/workspace/alphaDB/bin", 0);
    dataFile->Write("aaa");
    dataFile->Write("bbb");
    dataFile->Write("ccc");
    dataFile->Write("ddd");
}

int main(int argc, char** argv) {
    try {
        // TestOpenDataFile(); 
        TestDataFile_Write();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}