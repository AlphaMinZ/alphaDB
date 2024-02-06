#include "../inc/file_io.h"
#include "../lib/inc/log.h"
#include "../inc/map.h"

#include <iostream>
#include <string>
#include <vector>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

void TestMap_Iterator() {
    alphaDB::Map::ptr map1(new alphaDB::Map);
    // 1 map 为空
    std::cout << "======================\n";
    alphaDB::IteratorInterFace::ptr iter1 = map1->Iterator(false);
    if(iter1->Valid() == false) {
        std::cout << "map is empoty\n";
    }

    // 2 map 有数据
    std::cout << "======================\n";
    map1->Put("aaa", std::make_shared<alphaDB::LogRecordPos>(1, 20), nullptr);
    alphaDB::IteratorInterFace::ptr iter2 = map1->Iterator(false);
    if(iter2->Valid() == true) {
        std::cout << "map is not empty\n";
        std::cout << iter2->Key() << "\n";
    }

    alphaDB::LogRecordPos::ptr pos = iter2->Value();
    std::cout << pos->fId << " " << pos->offset << "\n";

    iter2->Next();

    if(iter2->Valid() == false) {
        std::cout << "map is last item\n";
    }

    // 3.有多条数据
    std::cout << "======================\n";
    map1->Put("bbb", std::make_shared<alphaDB::LogRecordPos>(1, 20), nullptr);
    map1->Put("ccc", std::make_shared<alphaDB::LogRecordPos>(1, 20), nullptr);
    map1->Put("ddd", std::make_shared<alphaDB::LogRecordPos>(1, 20), nullptr);
    alphaDB::IteratorInterFace::ptr iter3 = map1->Iterator(false);
    for(iter3->Rewind(); iter3->Valid(); iter3->Next()) {
        if(!iter3->Key().empty()) {
            std::cout << "key is : " << iter3->Key() << "\n";
        }
    }

    alphaDB::IteratorInterFace::ptr iter4 = map1->Iterator(true);
    for(iter4->Rewind(); iter4->Valid(); iter4->Next()) {
        if(!iter4->Key().empty()) {
            std::cout << "key is : " << iter4->Key() << "\n";
        }
    }

    // 4 测试 Seek
    std::cout << "======================\n";
    alphaDB::IteratorInterFace::ptr iter5 = map1->Iterator(false);
    for(iter5->Seek("bbb"); iter5->Valid(); iter5->Next()) {
        if(!iter5->Key().empty()) {
            std::cout << "key is : " << iter5->Key() << "\n";
        }
    }
    // 4 测试反向遍历 Seek
    std::cout << "======================\n";
    alphaDB::IteratorInterFace::ptr iter6 = map1->Iterator(true);
    for(iter6->Seek("ccc"); iter6->Valid(); iter6->Next()) {
        if(!iter6->Key().empty()) {
            std::cout << "key is : " << iter6->Key() << "\n";
        }
    }
}


int main(int argc, char** argv) {
    try {

       TestMap_Iterator(); 

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}