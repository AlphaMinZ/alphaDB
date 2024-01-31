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
};

const std::runtime_error MyErrors::ErrKeyIsEmpty("the key is empty");
const std::runtime_error MyErrors::ErrIndexUpdateFailed("failed to update index");
const std::runtime_error MyErrors::ErrKeyNotFound("key not found in database");
const std::runtime_error MyErrors::ErrDataFileNotFound("data file is not found");
const std::runtime_error MyErrors::ErrOpenDirectoryFailed("failed to open directory");
const std::runtime_error MyErrors::ErrDataDirectoryCorrupted("the database directory maybe corrupted");

}

#endif