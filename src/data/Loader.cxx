#include <cstdlib> // for atoi
#include <filesystem>
#include <map>
#include <regex>
#include <stdexcept> // for runtime_error
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::map<int, fs::path>
load_cosmology_dataset(std::string data_folder_path) {
  fs::path folder{data_folder_path};

  if (!fs::exists(folder)) {
    throw std::runtime_error("Folder at " + data_folder_path +
                             " does not exist!");
  }

  const std::regex vtk_cosmo_file(".*Full\\.cosmo\\.([\\d]{3})\\.vtp$");
  std::map<int, fs::path> ts_to_path;

  std::smatch base_match;
  for (auto const &dir_entry : fs::directory_iterator{folder}) {
    std::string path = dir_entry.path().c_str();
    if (std::regex_search(path, base_match,
                         vtk_cosmo_file)) {
      auto number = base_match.str(1);
      int num = std::atoi(number.c_str());
      ts_to_path.insert_or_assign(num, dir_entry.path());
    }
  }

  return ts_to_path;
}
