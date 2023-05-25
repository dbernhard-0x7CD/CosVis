#pragma once

#include <map>

class vtkPolyData;
class vtkProgrammableFilter;

struct AssignClusterParams {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
  std::map<int, int> *clustering; // id -> cluster ID
};

void AssignCluster(void *arguments);
