class vtkPolyData;
class vtkPolyDataMapper;
class vtkProgrammableFilter;

struct params {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
  vtkPolyDataMapper *mapper;
};

void CalculateTemperature(void *arguments);
