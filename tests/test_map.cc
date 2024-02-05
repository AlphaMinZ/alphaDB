#include "../inc/file_io.h"
#include "../lib/inc/log.h"
#include "../inc/map.h"

#include <iostream>
#include <string>
#include <vector>

alphaMin::Logger::ptr g_logger = ALPHA_LOG_ROOT();



int main(int argc, char** argv) {
    try {
        

    } catch(const std::exception& e) {
        ALPHA_LOG_ERROR(g_logger) << "Error : " << e.what();
    }

    return 0;
}