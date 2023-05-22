#pragma once

#include <cstdint>

class vtkPolyData;
class vtkPolyDataMapper;
class vtkProgrammableFilter;
enum class Selector;

struct particleTypeFilterParams {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
  uint16_t current_filter;
  vtkPolyData *agnParticles;
  vtkPolyData *baryonParticles;
  vtkPolyData *starParticles;
};

void FilterType(void *arguments);

enum class Selector {
  BARYON =                    static_cast<uint16_t>(0b10),
  BARYON_STAR =           static_cast<uint16_t>(0b100000),
  BARYON_WIND =          static_cast<uint16_t>(0b1000000),
  BARYON_STAR_FORMING = static_cast<uint16_t>(0b10000000),
  DARK_AGN =           static_cast<uint16_t>(0b100000000),
  DARK_MATTER =       static_cast<uint16_t>(0b1000000000), // NOT a bitmask
  NONE =            static_cast<uint16_t>(0b100000000000),
  ALL =               static_cast<uint16_t>(0b1111100010),
};

