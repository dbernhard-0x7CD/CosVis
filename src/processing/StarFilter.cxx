#include <cstdint> // for uint16_t
#include <stdio.h>

#include <vtkDoubleArray.h>
#include <vtkGenericDataArray.txx> // for vtkGenericDataArray::InsertNextValue
#include <vtkIntArray.h> // for vtkIntArray
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>   // for vtkPoints
#include <vtkPolyData.h> // for vtkPolyData
#include <vtkProgrammableFilter.h>
#include <vtkType.h> // for vtkIdType

#include "ParticleTypeFilter.hxx"
#include "StarFilter.hxx"

void StarType(void *arguments) {
  StarFilterParams *input = static_cast<StarFilterParams *>(arguments);

  vtkPoints *inPts = input->data->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();

  vtkIntArray *mask =
      static_cast<vtkIntArray *>(input->data->GetPointData()->GetArray("mask"));
  vtkDoubleArray *rho = static_cast<vtkDoubleArray *>(
      input->data->GetPointData()->GetArray("rho"));
  vtkDoubleArray *mass = static_cast<vtkDoubleArray *>(
      input->data->GetPointData()->GetArray("mass"));
  vtkDoubleArray *temperature = static_cast<vtkDoubleArray *>(
      input->data->GetPointData()->GetArray("Temperature"));

  vtkNew<vtkPoints> starPoints;

  vtkNew<vtkDoubleArray> baryonRhoArr;
  vtkNew<vtkDoubleArray> baryonMassArr;
  vtkNew<vtkDoubleArray> temperatureArr;

  baryonRhoArr->SetName("rho");
  baryonRhoArr->SetNumberOfComponents(1);
  baryonMassArr->SetName("mass");
  baryonMassArr->SetNumberOfComponents(1);
  temperatureArr->SetName("Temperature");
  temperatureArr->SetNumberOfComponents(1);

  size_t num = 0;
  double pos[3];
  for (vtkIdType i = 0; i < numPts; i++) {
    uint16_t this_mask = static_cast<uint16_t>(mask->GetTuple1(i));
    input->data->GetPoints()->GetPoint(i, pos);

    if (this_mask & static_cast<uint16_t>(Selector::BARYON_STAR)) {
      starPoints->InsertNextPoint(pos);
    }
  }

  input->filter->GetPolyDataOutput()->SetPoints(starPoints);
  // The following is currently not needed
  //  input->filter->GetPointData()->AddArray(baryonMassArr);
  //  input->filter->GetPointData()->AddArray(baryonRhoArr);
  //  input->filter->GetPointData()->AddArray(temperatureArr);
}
