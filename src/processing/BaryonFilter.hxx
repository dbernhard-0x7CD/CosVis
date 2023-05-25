#pragma once

#include <cstdint>

class vtkPolyData;
class vtkPolyDataMapper;
class vtkProgrammableFilter;
enum class Selector;

struct BaryonFilterParams {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
  vtkPolyData *baryonParticles;
};

void BaryonFilter(void *arguments);
