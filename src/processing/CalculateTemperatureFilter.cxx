#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProgrammableFilter.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkDoubleArray.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkInformation.h>
#include <vtkSphereSource.h>

namespace {
struct params {
  vtkPolyData *data;
  vtkProgrammableFilter *filter;
};

void CalculateTemperature(void *arguments) {
  params *input = static_cast<params *>(arguments);

  vtkPoints *inPts = input->data->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();

  vtkDataArray* uu = input->data->GetPointData()->GetArray("uu");

  vtkInformation* info = input->data->GetInformation();
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

  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(temp);
}

} // namespace
