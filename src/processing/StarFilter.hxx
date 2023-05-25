#pragma once

class vtkPolyData;
class vtkProgrammableFilter;

struct StarFilterParams {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
};

void StarType(void *arguments);
