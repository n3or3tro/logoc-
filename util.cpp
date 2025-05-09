#include "util.hpp"
#include <filesystem>
#include <fstream>
#include <string>

std::string read_file(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    std::string s(std::filesystem::file_size(path), '\0');
    f.read(s.data(), s.size());
    return s;
}