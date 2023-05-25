#pragma once

class vtkPolyData;
class vtkProgrammableFilter;

struct BaryonFilterParams {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
  vtkPolyData *baryonParticles;
};

void BaryonFilter(void *arguments);
