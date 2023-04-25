#include <vtkLookupTable.h>

vtkNew<vtkLookupTable> GetTemperatureLUT() {
  // LUT for coloring the particles
  vtkNew<vtkLookupTable> lut;

  lut->SetHueRange(0.667, 0.0);
  lut->SetAlphaRange(0.3, 0.7);
  lut->SetTableRange(0.0, 1e5);
  lut->SetNumberOfColors(512);
  // TODO: we should probably move to a logarithm scale

  lut->SetUseAboveRangeColor(1);
  lut->SetAboveRangeColor(0.0, 1.0, 1.0, 1.0);
  lut->SetUseBelowRangeColor(1);
  lut->SetBelowRangeColor(0.0, 1.0, 1.0, 1.0);

  // lut->SetScaleToLog10();

  lut->Build();
  
  return lut;
}

vtkNew<vtkLookupTable> GetClusterLUT() {
  vtkNew<vtkLookupTable> lut;

  lut->SetTableRange(0.0, 26.0);
  lut->SetAlpha(0.4);
  // categoryLut->SetNumberOfColors(27);
  lut->SetNumberOfTableValues(27);
  lut->Build();

  // green
  lut->SetNanColor(0.0, 1.0, 0.0, 1.0);

  // 26 is noise points (are not assigned to any cluster)
  lut->SetTableValue(26, 0.0, 1.0, 0.0, 0.0);
  lut->SetTableValue(0, 1.0, 0.0, 0.0, 1.0);
  lut->SetTableValue(1, 0.33, 0.42, 0.18, 1.0);
  lut->SetTableValue(2, 0.54, 0.26, 0.07, 1.0);
  lut->SetTableValue(3, 0.28, 0.23, 0.54, 1.0);
  lut->SetTableValue(4, 0.23, 0.7, 0.44, 1.0);
  lut->SetTableValue(5, 0.0, 0.54, 0.54, 1.0);
  lut->SetTableValue(6, 0.0, 0.0, 0.5, 1.0);
  lut->SetTableValue(7, 0.6, 0.8, 0.19, 1.0);
  lut->SetTableValue(8, 0.54, 0.0, 0.54, 1.0);
  lut->SetTableValue(9, 1.0, 0.0, 0.0, 1.0);
  lut->SetTableValue(10, 1.0, 0.54, 0.0, 1.0);
  lut->SetTableValue(11, 1.0, 1.0, 0.0, 1.0);
  lut->SetTableValue(12, 0.0, 1.0, 0.0, 1.0);
  lut->SetTableValue(13, 0.54, 0.167, 0.88, 1.0);
  lut->SetTableValue(14, 1.0, 1.0, 1.0, 1.0);
  lut->SetTableValue(15, 0.86, 0.078, 0.156, 1.0);
  lut->SetTableValue(16, 0.0, 1.0, 1.0, 1.0);
  lut->SetTableValue(17, 0.0, 0.746, 1.0, 1.0);
  lut->SetTableValue(18, 0.0, 0.0, 1.0, 1.0);
  lut->SetTableValue(19, 1.0, 0.0, 1.0, 1.0);
  lut->SetTableValue(20, 0.117, 0.56, 1.0, 1.0);
  lut->SetTableValue(21, 0.855, 0.437, 0.574, 1.0);
  lut->SetTableValue(22, 0.937, 0.898, 0.547, 1.0);
  lut->SetTableValue(23, 1.0, 0.078, 0.574, 1.0);
  lut->SetTableValue(24, 1.0, 0.626, 0.476, 1.0);
  lut->SetTableValue(25, 0.93, 0.508, 0.93, 1.0);
  lut->SetUseAboveRangeColor(1);
  lut->SetAboveRangeColor(0.0, 0.0, 1.0, 1.0);
  lut->SetUseBelowRangeColor(1);
  lut->SetBelowRangeColor(0.0, 0.0, 0.0, 1.0);

  return lut;
}
