#pragma once

#include <vtkLookupTable.h>

vtkNew<vtkLookupTable> GetTemperatureLUT();
vtkNew<vtkLookupTable> GetClusterLUT();
vtkNew<vtkLookupTable> GetPhiLUT();
