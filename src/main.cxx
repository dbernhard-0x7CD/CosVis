#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <string>
// IWYU pragma: no_include <bits/chrono.h>

#include "app/VisCos.hpp"

namespace fs = std::filesystem;

// Some program constants
const std::string background("#111111");

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage is %s DATA_FOLDER_PATH\n", argv[0]);
    return 0;
  }
  std::string data_folder_path;
  data_folder_path = argv[1];

  if (!std::filesystem::exists(data_folder_path)) {
    printf("The data folder path does not exist: %s. It has to contain the *.vtp files.\n", data_folder_path.c_str());
    return 0;
  }

  std::string cluster_path(data_folder_path);
  cluster_path.append("/clusters.vtp");
  if (!std::filesystem::exists(cluster_path)) {
    printf("The clusters file is missing at %s. Please read the README.md on how to generate it.\n", cluster_path.c_str());
    return 0;
  }

  VisCos app(566, data_folder_path, cluster_path);

  // Load the data
  app.Load();

  // Sets up the data pipeline and some default visuals
  app.SetupPipeline();
  app.SetBackgroundColor(background);

  app.ShowTemperature();

  app.Run();

  return EXIT_SUCCESS;
}
