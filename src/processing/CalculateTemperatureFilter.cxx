#include <cmath> // for pow
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkGenericDataArray.txx> // for vtkGenericDataArray::InsertNextValue
#include <vtkInformation.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>   // for vtkPoints
#include <vtkPolyData.h> // for vtkPolyData
#include <vtkProgrammableFilter.h>
#include <vtkType.h> // for vtkIdType
#include <vtkLookupTable.h>

#include "CalculateTemperatureFilter.hxx"

void CalculateTemperature(void *arguments) {
  params *input = static_cast<params *>(arguments);

  vtkPoints *inPts = input->data->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();

  vtkDataArray *uu = input->data->GetPointData()->GetArray("uu");

  vtkInformation *info = input->data->GetInformation();
  double dtimestep = info->Get(vtkPolyData::DATA_TIME_STEP());
  double z = 200 * (1 - dtimestep / 625);

  vtkNew<vtkDoubleArray> temp;
  temp->SetName("Temperature");
  temp->SetNumberOfComponents(1);

  const double factor = 4.8e5;
  for (vtkIdType i = 0; i < numPts; i++) {
    double this_uu = uu->GetTuple1(i);
    temp->InsertNextValue(factor * this_uu / pow(1.0 + z, 3));
  }

  double range[2];
  temp->GetRange(range);
  input->temp_lut->SetRange(range);
  input->mapper->SetScalarRange(range);
  printf("Range is from %lf to %lf\n", range[0], range[1]);
  input->filter->GetPolyDataOutput()->ShallowCopy(input->data);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(temp);
}
