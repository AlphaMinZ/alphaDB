#include "../inc/errors.h"

namespace alphaDB {
    const std::runtime_error MyErrors::ErrKeyIsEmpty("the key is empty");
    const std::runtime_error MyErrors::ErrIndexUpdateFailed("failed to update index");
    const std::runtime_error MyErrors::ErrKeyNotFound("key not found in database");
    const std::runtime_error MyErrors::ErrDataFileNotFound("data file is not found");
    const std::runtime_error MyErrors::ErrOpenDirectoryFailed("failed to open directory");
    const std::runtime_error MyErrors::ErrDataDirectoryCorrupted("the database directory maybe corrupted");

    const std::runtime_error MyErrors::ErrInvalidCRC("invalid crc value, log record maybe corrupted");

    const std::runtime_error MyErrors::ErrExceedMaxBatchNum("exceed the max batch num");
}