#pragma once

#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkStructuredGrid.h>

vtkNew<vtkLookupTable> GetTemperatureLUT();
vtkNew<vtkLookupTable> GetClusterLUT();
vtkNew<vtkLookupTable> GetPhiLUT();

vtkNew<vtkColorTransferFunction> GetSPHLUT();
vtkNew<vtkStructuredGrid> GetSPHStructuredGrid(int dimensions[3], double spacing[3], double sphOrigin[3]);
