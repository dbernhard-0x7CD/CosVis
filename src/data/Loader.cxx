#include <bits/chrono.h>
#include <cstdlib> // for atoi
#include <filesystem>
#include <map>
#include <regex>
#include <stdexcept> // for runtime_error
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::map<int, fs::__cxx11::path>
load_cosmology_dataset(std::string data_folder_path) {
  fs::path folder{data_folder_path};

  if (!fs::exists(folder)) {
    throw std::runtime_error("Folder at " + data_folder_path +
                             " does not exist!");
  }

  const std::regex vtk_cosmo_file(".*Full\\.cosmo\\.([\\d]{3})\\.vtp$");
  std::map<int, fs::__cxx11::path> ts_to_path;

  std::cmatch base_match;
  for (auto const &dir_entry : fs::directory_iterator{folder}) {
    if (std::regex_match(dir_entry.path().string().c_str(), base_match,
                         vtk_cosmo_file)) {
      auto number = base_match[1];
      ts_to_path.insert_or_assign(std::atoi(number.str().c_str()),
                                  dir_entry.path());
    }
  }

  return ts_to_path;
}
