#include "../inc/log_record.h"
#include "../lib/inc/log.h"

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void printf_string(std::string str) {
    std::cout << str.size() << "\n";
    for(char ch : str) {
        std::cout << (int)ch << " ";
    }
    std::cout << "\n";
}

void TestEncodeLogRecord() {
    // 正常情况
    alphaDB::LogRecord* rec1 = new alphaDB::LogRecord;

    rec1->Key = std::string("name_");
    rec1->Value = std::string("bitcask-cpp");
    rec1->Type = alphaDB::LogRecordNormal;

    int64_t n1;

    std::string res1 = alphaDB::EncodeLogRecord(rec1, n1);
    printf_string(res1);
    // ALPHA_LOG_INFO(g_logger) << (int)res1.data()[0] << " " << (int)res1.data()[1] << " " << n1 << " " << sizeof(res1.data());

    // // // value 为空的情况
    // alphaDB::LogRecord* rec2 = new alphaDB::LogRecord;

    // rec2->Key = std::string("name");
    // rec2->Type = alphaDB::LogRecordNormal;

    // ALPHA_LOG_DEBUG(g_logger) << res1.size() << res1.c_str();

    // int64_t n2;

    // std::string res2 = alphaDB::EncodeLogRecord(rec2, n2);
    // ALPHA_LOG_INFO(g_logger) << (int)res2.data()[0] << " " << (int)res2.data()[1] << " " << n2 << " " << sizeof(res2.data());

    // // 正常情况
    // alphaDB::LogRecord* rec3 = new alphaDB::LogRecord;

    // rec1->Key = std::string("name");
    // rec1->Value = std::string("bitcask-cpp");
    // rec1->Type = alphaDB::LogRecordDeleted;

    // int64_t n3;

    // std::string res3 = alphaDB::EncodeLogRecord(rec3, n3);
    // ALPHA_LOG_INFO(g_logger) << (int)res3.data()[0] << " " << (int)res3.data()[1] << " " << n3 << " " << sizeof(res3.data());
}

void TestDecodeLogRecordHeader() {
    int arr[] = {26, -103, -117, 72, 0, 5, 0, 0, 0, 11, 0, 0, 0, 110, 97, 109, 101, 95, 98, 105, 116, 99, 97, 115, 107, 45, 99, 112, 112};

    // 使用 std::stringstream 构造字符串
    std::stringstream ss;
    for (int i : arr) {
        ss << char(i);
    }

    std::string result = ss.str();

    std::cout << result.size() << "\n";

    int64_t headerSize;

    alphaDB::LogRecordHeader* da;

    da = alphaDB::decodeLogRecordheader(result, headerSize);

    std::cout << da->keySize << " " << (int)da->valueSize << "\n";

    if(da->recordType == alphaDB::LogRecordNormal) {
        std::cout << "True\n";
    }
}

void TestGetLogRecordCRC() {
    alphaDB::LogRecord* rec1 = new alphaDB::LogRecord;

    rec1->Key = std::string("name_");
    rec1->Value = std::string("bitcask-cpp");
    rec1->Type = alphaDB::LogRecordNormal;

    int arr[] = {26, -103, -117, 72, 0, 5, 0, 0, 0, 11, 0, 0, 0, 110, 97, 109, 101, 95, 98, 105, 116, 99, 97, 115, 107, 45, 99, 112, 112};

    // 使用 std::stringstream 构造字符串
    std::stringstream ss;
    for (int i : arr) {
        ss << char(i);
    }

    std::string headerBuf1 = ss.str();
    std::string headfer(headerBuf1.begin() + 4, headerBuf1.end());

    alphaDB::LogRecordHeader* da;

    int64_t headdadadad;

    da = alphaDB::decodeLogRecordheader(headerBuf1, headdadadad);

    uint32_t crc1 = alphaDB::getLogRecordCRC(rec1, headerBuf1.substr(4));

    if(crc1 == da->crc) {
        std::cout << "True\n"; 
    } else {
        std::cout << "False\n";
    }

}

int main(int argc, char** argv) {
    try {
        // TestEncodeLogRecord();
        TestDecodeLogRecordHeader();
        // TestGetLogRecordCRC();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}