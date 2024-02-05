#include "../inc/data_file.h"
#include "../lib/inc/log.h"
#include "../inc/log_record.h"

#include <iostream>
#include <fstream>
#include <string>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void printf_string(std::string str) {
    std::cout << str.size() << "\n";
    for(char ch : str) {
        std::cout << (int)ch << " ";
    }
    std::cout << "\n";
}

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

void TestDataFile_ReadLogRecor() {
    alphaDB::DataFile::ptr dataFile = alphaDB::OpenDataFile("/home/mz/workspace/alphaDB/bin", 0);

    alphaDB::LogRecord* rec1 = new alphaDB::LogRecord;
    rec1->Key = std::string("name_cpp");
    rec1->Value = std::string("bitcask kv my cpp");

    

    int64_t size1;
    std::string res1 = alphaDB::EncodeLogRecord(rec1, size1);
    std::cout << res1.size() << " " << size1 << std::endl;
    dataFile->Write(res1);

    uint32_t readSize1;
    bool isEOF;
    alphaDB::LogRecord* readRec1 = dataFile->ReadLogRecord(0, readSize1, &isEOF);

    std::cout << readRec1->Key << std::endl;

    std::cout << readRec1->Value << std::endl;

    alphaDB::LogRecord* rec2 = new alphaDB::LogRecord;
    rec2->Key = std::string("name_go");
    rec2->Value = std::string("bitcask kv my go");

    int64_t size2;
    std::string res2 = alphaDB::EncodeLogRecord(rec2, size2);
    std::cout << res2.size() << std::endl;
    dataFile->Write(res2);
    std::cout << "----\n";
    uint32_t readSize2;
    alphaDB::LogRecord* readRec2 = dataFile->ReadLogRecord(size1, readSize2, &isEOF);

    std::cout << "----\n";
    std::cout << readRec2->Key << std::endl;

    std::cout << readRec2->Value << std::endl;

    alphaDB::LogRecord* rec3 = new alphaDB::LogRecord;
    rec3->Key = std::string("name_rust");
    rec3->Value = std::string("bitcask kv my rust");
    rec3->Type = alphaDB::LogRecordDeleted;
    int64_t size3;
    std::string res3 = alphaDB::EncodeLogRecord(rec3, size3);
    std::cout << res3.size() << std::endl;
    dataFile->Write(res3);

    uint32_t readSize3;
    alphaDB::LogRecord* readRec3 = dataFile->ReadLogRecord(size2 + size1, readSize3, &isEOF);

    std::cout << readRec3->Key << std::endl;

    std::cout << readRec3->Value << std::endl;
   
}

void TestDataFile_ReadNBytes() {
    alphaDB::DataFile::ptr dataFile = alphaDB::OpenDataFile("/home/mz/workspace/alphaDB/bin", 0);
    std::string ret = dataFile->readNBytes(3, 2);

    std::cout << ret.c_str() << std::endl;
}

// void Test__() {
//     std::ofstream outputFile("/home/mz/workspace/alphaDB/bin/123.data", std::ios::binary);

//     if (!outputFile.is_open()) {
//         std::cerr << "Error opening file for writing." << std::endl;
//         return;
//     }

// alphaDB::LogRecord* rec1 = new alphaDB::LogRecord;
//     rec1->Key = std::string("name_");
//     rec1->Value = std::string("bitcask kv my cpp");

    

//     int64_t size1;
//     std::string res1 = alphaDB::EncodeLogRecord(rec1, size1);
//     std::cout << res1.size() << std::endl;

//     // 获取字符串的长度
//     std::size_t dataSize = res1.size();

//     // 写入字符串长度
//     outputFile.write(res1.c_str(), dataSize);

//     printf_string(res1);

//     std::string read_f(35, '\0');
//     // outputFile.
//     outputFile.close();
// }

// void read_te() {
//     std::ifstream inputFile("/home/mz/workspace/alphaDB/bin/123.data", std::ios::binary);

//     if (!inputFile.is_open()) {
//         std::cerr << "Error opening file for reading." << std::endl;
//         return;
//     }

//     // 读取字符串长度


//     // 分配足够的内存
//     std::string data(35, '\0');

//     // 读取字符串数据
//     inputFile.read(&data[0], 35);

//     printf_string(data);
// }


int main(int argc, char** argv) {
    try {
        // TestOpenDataFile(); 
        // TestDataFile_Write();
        TestDataFile_ReadLogRecor();
        // TestDataFile_ReadNBytes();
        // Test__();
        // read_te();
        // test_class();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}