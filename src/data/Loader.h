#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace fs = std::filesystem;

std::map<int, fs::path>
load_cosmology_dataset(std::string data_folder_path);
