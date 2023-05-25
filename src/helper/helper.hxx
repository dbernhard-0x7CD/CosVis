#pragma once

#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkStructuredGrid.h>

vtkSmartPointer<vtkLookupTable> GetTemperatureLUT();
vtkSmartPointer<vtkLookupTable> GetClusterLUT();
vtkSmartPointer<vtkLookupTable> GetPhiLUT();

vtkSmartPointer<vtkColorTransferFunction> GetSPHLUT();
vtkSmartPointer<vtkStructuredGrid> GetSPHStructuredGrid(int dimensions[3], double spacing[3], double sphOrigin[3]);
