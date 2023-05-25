#include <stdio.h>

#include <vtkAlgorithmOutput.h>
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

#include "BaryonFilter.hxx"
#include "ParticleTypeFilter.hxx"

void BaryonFilter(void *arguments) {
  BaryonFilterParams *input = static_cast<BaryonFilterParams*>(arguments);

  vtkPoints *inPts = input->data->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();

  vtkIntArray *mask = static_cast<vtkIntArray *>(input->data->GetPointData()->GetArray("mask"));
  vtkDoubleArray *rho = static_cast<vtkDoubleArray *>(input->data->GetPointData()->GetArray("rho"));
  vtkDoubleArray *mass = static_cast<vtkDoubleArray *>(input->data->GetPointData()->GetArray("mass"));
  vtkDoubleArray *temperature = static_cast<vtkDoubleArray *>(input->data->GetPointData()->GetArray("Temperature"));

  vtkNew<vtkPoints> baryonPoints;

  vtkNew<vtkDoubleArray> baryonRhoArr;
  vtkNew<vtkDoubleArray> baryonMassArr;
  vtkNew<vtkDoubleArray> temperatureArr;

  baryonRhoArr->SetName("rho");
  baryonRhoArr->SetNumberOfComponents(1);
  baryonMassArr->SetName("mass");
  baryonMassArr->SetNumberOfComponents(1);
  temperatureArr->SetName("Temperature");
  temperatureArr->SetNumberOfComponents(1);

  // Based on the filter do stuff
  size_t num = 0;
  double pos[3];
  for (vtkIdType i = 0; i < numPts; i++) {
    uint16_t this_mask = static_cast<uint16_t>(mask->GetTuple1(i));
    input->data->GetPoints()->GetPoint(i, pos);

    if (this_mask & 0b10) {
      baryonPoints->InsertNextPoint(pos);
      baryonRhoArr->InsertNextTuple1(rho->GetTuple1(i));
      baryonMassArr->InsertNextTuple1(mass->GetTuple1(i));
      temperatureArr->InsertNextTuple1(temperature->GetTuple1(i));
    }
  }

  input->filter->GetPolyDataOutput()->SetPoints(baryonPoints);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(baryonMassArr);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(baryonRhoArr);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(temperatureArr);
}
