#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkProgrammableFilter.h>

struct params {
  vtkPolyData *data;
  vtkLookupTable *temp_lut;
  vtkProgrammableFilter *filter;
  vtkPolyDataMapper *mapper;
};

void CalculateTemperature(void *arguments);
