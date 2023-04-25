#pragma once

class vtkPolyData;
class vtkPolyDataMapper;
class vtkProgrammableFilter;

struct tempFilterParams {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;

  // Defines if we update the scalar range of the data mapper
  bool updateScalarRange;
  vtkPolyDataMapper *mapper;
};

void CalculateTemperature(void *arguments);
