#ifndef __ALPHA_ERRORS_H__
#define __ALPHA_ERRORS_H__

#include <stdexcept>
#include <iostream>

namespace alphaDB {

class MyErrors {
public:
    static const std::runtime_error ErrKeyIsEmpty;
    static const std::runtime_error ErrIndexUpdateFailed;
    static const std::runtime_error ErrKeyNotFound;
    static const std::runtime_error ErrDataFileNotFound;
    static const std::runtime_error ErrOpenDirectoryFailed;
    static const std::runtime_error ErrDataDirectoryCorrupted;

    static const std::runtime_error ErrInvalidCRC;

    static const std::runtime_error ErrExceedMaxBatchNum;
};

}

#endif