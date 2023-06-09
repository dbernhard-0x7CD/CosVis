#include <stdio.h>

#include <cmath> // for pow
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkGenericDataArray.txx> // for vtkGenericDataArray::InsertNextValue
#include <vtkInformation.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>   // for vtkPoints
#include <vtkPolyData.h> // for vtkPolyData
#include <vtkPolyDataMapper.h>
#include <vtkProgrammableFilter.h>
#include <vtkType.h> // for vtkIdType

#include "CalculateTemperatureFilter.hxx"

void CalculateTemperature(void *arguments) {
  TempFilterParams *input = static_cast<TempFilterParams *>(arguments);

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

  if (input->updateScalarRange) {
    double range[2];
    temp->GetRange(range);
    input->mapper->SetScalarRange(range);
  }

  input->filter->GetPolyDataOutput()->ShallowCopy(input->data);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(temp);
}
