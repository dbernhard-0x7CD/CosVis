#include <filesystem>
#include <map>
#include <string>

namespace fs = std::__fs::filesystem;

std::map<int, std::__fs::filesystem::path>
load_cosmology_dataset(std::string data_folder_path);
