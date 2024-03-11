基于 Bitcask 模型，兼容 Redis 数据结构和协议的高性能 KV 存储引擎
部分测试代码在 tests 目录下，有一些是之前做测试的废代码，还没有做清理

void Test() {
    auto opts = alphaDB::DefaultOptions;  // 获得默认配置
    opts.DirPath = "/home/mz/workspace/alphaDB/bin/data_2";  // 自定义位置
    opts.DataFileSize = 32 * 1024 * 1024;  // 每个数据文件大小
    opts.DataFileMergeRatio = 0; 

    auto db = alphaDB::Open(opts);  // 打开 DB 实例

    for(int i = 0; i < 50; ++i) {
        db->Put(std::to_string(i), std::to_string(i));  // 写入 K/V 数据
    }

    db->Merge();  // 数据 merge

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

以上是一个简单的使用案例，打开 DB 实例，写入数据和数据 merge
