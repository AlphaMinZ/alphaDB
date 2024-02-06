#include "../inc/file_io.h"
#include "../lib/inc/log.h"
#include "../inc/map.h"
#include "../inc/iterator.h"
#include "../inc/db.h"
#include "../inc/options.h"

#include <iostream>
#include <string>
#include <vector>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void TestDB_NewIterator() {

    alphaDB::Options opts = alphaDB::DefaultOptions;
    std::string dir = "/home/mz/workspace/alphaDB/bin/data";
    opts.DirPath = dir;
    alphaDB::DB::ptr db = alphaDB::Open(opts);

    alphaDB::Iterator::ptr iterator(db->Newiterator(alphaDB::DefaultIteratorOptions));
    if(iterator->Valid() == false) {
        std::cout << "iterator is empty\n";
    }
}

void TestDB_Iterator_One_Value() {
    alphaDB::Options opts = alphaDB::DefaultOptions;
    std::string dir = "/home/mz/workspace/alphaDB/bin/data";
    opts.DirPath = dir;
    alphaDB::DB::ptr db = alphaDB::Open(opts);

    db->Put("aabc", "1234");
    db->Put("aacd", "1234");

    alphaDB::Iterator::ptr iterator(db->Newiterator(alphaDB::DefaultIteratorOptions));
    iterator->Next();
    std::cout << iterator->Key() << "\n";
    std::cout << iterator->Value() << "\n";
}

void TestDB_Iterator_Multi_Values() {
    alphaDB::Options opts = alphaDB::DefaultOptions;
    std::string dir = "/home/mz/workspace/alphaDB/bin/data";
    opts.DirPath = dir;
    alphaDB::DB::ptr db = alphaDB::Open(opts);

    db->Put("aabc", "1");
    db->Put("abbc", "2");
    db->Put("babc", "3");
    db->Put("cebc", "4");
    db->Put("bbbc", "5");
    db->Put("dabc", "6");

    // 正向迭代
    std::cout << "==========================\n";
    alphaDB::Iterator::ptr iter1(db->Newiterator(alphaDB::DefaultIteratorOptions));
    for(iter1->Rewind(); iter1->Valid(); iter1->Next()) {
        std::cout << iter1->Key() << " " << iter1->Value() << "\n";
    }
    iter1->Rewind();
    std::cout << "--------------------------\n";
    for(iter1->Seek("c"); iter1->Valid(); iter1->Next()) {
        std::cout << iter1->Key() << " " << iter1->Value() << "\n";
    }

    // 反向迭代
    std::cout << "==========================\n";
    alphaDB::IteratorOption iterOpts1 = alphaDB::DefaultIteratorOptions;
    iterOpts1.Reverse = true;
    alphaDB::Iterator::ptr iter2(db->Newiterator(iterOpts1));
    for(iter2->Rewind(); iter2->Valid(); iter2->Next()) {
        std::cout << iter2->Key() << " " << iter2->Value() << "\n";
    }
    iter2->Rewind();
    std::cout << "--------------------------\n";
    for(iter2->Seek("c"); iter2->Valid(); iter2->Next()) {
        std::cout << iter2->Key() << " " << iter2->Value() << "\n";
    }

    // 指定的 Prefix
    std::cout << "==========================\n";
    alphaDB::IteratorOption iterOpts2 = alphaDB::DefaultIteratorOptions;
    iterOpts2.Prefix = "a";
    alphaDB::Iterator::ptr iter3(db->Newiterator(iterOpts2));
    for(iter3->Rewind(); iter3->Valid(); iter3->Next()) {
        std::cout << iter3->Key() << " " << iter3->Value() << "\n";
    }

}

int main(int argc, char** argv) {
    try {

    //    TestDB_NewIterator(); 
    // TestDB_Iterator_One_Value();
    TestDB_Iterator_Multi_Values();

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}