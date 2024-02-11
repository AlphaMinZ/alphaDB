#include "../inc/batch.h"
#include "../lib/inc/log.h"
#include "../inc/options.h"
#include "../inc/db.h"

#include <iostream>
#include <string>
#include <vector>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void TestDB_WriteBatch1() {
    alphaDB::Options opts = alphaDB::DefaultOptions;
    std::string dir = "/home/mz/workspace/alphaDB/bin/data";
    opts.DirPath = dir;
    alphaDB::DB::ptr db = alphaDB::Open(opts);

    // 写数据之后并不提交
    alphaDB::WriteBatchOptions wopts = {10000, true};
    alphaDB::WriteBatch::ptr wb = db->NewWriteBatch(wopts);
    wb->Put("aaa", "111");
    wb->Delete("bbb");

    // db->Get("aaa");

    // 正常提交数据
    wb->Commit();
    std::string val1 = db->Get("aaa");
    ALPHA_LOG_INFO(g_logger) << val1;

    // 删除有效的数据
    wb->Delete("aaa");
    wb->Commit();
    db->Get("aaa");
}

void TestDB_WriteBatch2() {
    alphaDB::Options opts = alphaDB::DefaultOptions;
    std::string dir = "/home/mz/workspace/alphaDB/bin/data";
    opts.DirPath = dir;
    alphaDB::DB::ptr db = alphaDB::Open(opts);

    // 写数据之后并不提交
    alphaDB::WriteBatchOptions wopts = {10000, true};
    alphaDB::WriteBatch::ptr wb = db->NewWriteBatch(wopts);

    db->Put("aaa", "111");

    wb->Put("bbb", "222");
    wb->Delete("aaa");

    wb->Commit();

    wb->Put("ccc", "333");

    wb->Commit();

    db->Close();

    // 重启
    alphaDB::DB::ptr db2 = alphaDB::Open(opts);
    // std::string val1 = db->Get("aaa");
    // ALPHA_LOG_INFO(g_logger) << val1;

    std::cout << db->getSeqNo() << std::endl;

}

void Test_logRecordKeyWithSeq() {
    std::string val = alphaDB::logRecordKeyWithSeq("aaa", 5);
    uint64_t cnt;
    std::string valll = alphaDB::parseLogRecordKey(val, cnt);
    std::cout << valll << " " << cnt << "\n";
}

int main(int argc, char** argv) {
    try {
        
        // TestDB_WriteBatch1();
        // Test_logRecordKeyWithSeq();
        TestDB_WriteBatch2();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}