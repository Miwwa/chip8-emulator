#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <vector>
#include <string>

namespace platform {
    std::expected<std::vector<uint8_t>, std::string> read_file(const char* path);
    std::expected<std::vector<uint8_t>, std::string> read_file(const std::filesystem::path& filepath);
}