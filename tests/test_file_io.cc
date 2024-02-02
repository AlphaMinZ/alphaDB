#include "../inc/file_io.h"
#include "../lib/inc/log.h"

#include <iostream>
#include <string>
#include <vector>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();

int main(int argc, char** argv) {
    try {
        alphaDB::FileIO file(std::string("/home/mz/workspace/alphaDB/bin/example.txt"));

        int ret = file.Write("Hello");



        char ch[7];

        sizeof(ch);

        ret = file.Read(ch, 4, 0);

        ALPHA_LOG_INFO(g_logger) << ch << " " << ret;

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}
