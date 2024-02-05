#include "../inc/db.h"
#include "../inc/options.h"
#include "../lib/inc/log.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>

extern alphaDB::Options DefaultOptions;

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void TestOpen() {
    alphaDB::Options opts = {"/home/mz/workspace/alphaDB/bin/data", 256 * 1024, false, alphaDB::IndexerType::Map};
    alphaDB::DB::ptr db = alphaDB::Open(opts);
}

void TestPut() {
    alphaDB::Options opts = {"/home/mz/workspace/alphaDB/bin/data", 256 * 1024, false, alphaDB::IndexerType::Map};
    alphaDB::DB::ptr db = alphaDB::Open(opts);

    db->Put("9", "9");

    // db->Put("33", "33");
    // db->Put("44", "44");

    // db->Delete("33");

    // for(int i = 0; i < 100000; ++i) {
    //     db->Put(std::to_string(i), std::to_string(i));
    // }

    db->Delete("9");

    db->Put("9", "9");

    std::string ret1 = db->Get("9");

    std::cout << ret1 << std::endl;

    // db->Put("11", "alphaMin");

    // ret1 = db->Get("11");

    std::cout << ret1 << std::endl;

    db->Delete("9");

    alphaDB::DB::ptr db2 = alphaDB::Open(opts);
    ret1 = db2->Get("9");
    
    std::cout << ret1 << std::endl;
}

int main(int argc, char** argv) {
    try {
        // TestOpen();
        TestPut();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}