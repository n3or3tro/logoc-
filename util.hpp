#pragma once
#include "util.hpp"
#include <filesystem>
#include <fstream>
#include <numbers>
#include <string>

inline std::string read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    std::string   s(std::filesystem::file_size(path), '\0');
    f.read(s.data(), s.size());
    return s;
}

inline float degree_to_radians(float degrees) {
    return degrees * (std::numbers::pi / 180.0f);
}