class vtkPolyData;
class vtkPolyDataMapper;
class vtkProgrammableFilter;

struct ac_params {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
  std::map<int, int> *clustering; // id -> cluster ID
};

void AssignCluster(void *arguments);

