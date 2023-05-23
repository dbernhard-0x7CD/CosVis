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

#include "ParticleTypeFilter.hxx"

void FilterType(void *arguments) {
  particleTypeFilterParams *input = static_cast<particleTypeFilterParams*>(arguments);

  vtkPoints *inPts = input->data->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();

  vtkIntArray *mask = static_cast<vtkIntArray *>(input->data->GetPointData()->GetArray("mask"));
  vtkDoubleArray *rho = static_cast<vtkDoubleArray *>(input->data->GetPointData()->GetArray("rho"));
  vtkDoubleArray *mass = static_cast<vtkDoubleArray *>(input->data->GetPointData()->GetArray("mass"));
  vtkDoubleArray *temperature = static_cast<vtkDoubleArray *>(input->data->GetPointData()->GetArray("Temperature"));

  vtkNew<vtkUnsignedCharArray> hiddenPoints;
  hiddenPoints->SetName(vtkDataSetAttributes::GhostArrayName());
  hiddenPoints->SetNumberOfValues(numPts);
  hiddenPoints->Fill(0);

  vtkNew<vtkPoints> agnPoints;
  vtkNew<vtkPoints> baryonPoints;
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

  // Based on the filter do stuff
  size_t num = 0;
  uint16_t all_mask = static_cast<uint16_t>(Selector::ALL);
  uint16_t mask_filter = static_cast<uint16_t>(input->current_filter);
  if ((input->current_filter & all_mask) == all_mask) {
    // keep all particles as is
    printf("[ParticleFilter]: ALL filter is active, Number of points: %ld\n", inPts->GetNumberOfPoints());

    for (vtkIdType i = 0; i < numPts; i++) {
      uint16_t this_mask = static_cast<uint16_t>(mask->GetTuple1(i));

      if (this_mask & static_cast<uint16_t>(Selector::DARK_AGN)) {
        agnPoints->InsertNextPoint(input->data->GetPoints()->GetPoint(i));
      }
      if (this_mask & 0b10) {
        baryonPoints->InsertNextPoint(input->data->GetPoints()->GetPoint(i));
        baryonRhoArr->InsertNextValue(rho->GetValue(i));
        baryonMassArr->InsertNextValue(mass->GetValue(i));
        temperatureArr->InsertNextValue(temperature->GetValue(i));
      }

      if (this_mask & static_cast<uint16_t>(Selector::BARYON_STAR)) {
        starPoints->InsertNextPoint(input->data->GetPoints()->GetPoint(i));
      }
    }
  } else {
    for (vtkIdType i = 0; i < numPts; i++) {
      uint16_t this_mask = static_cast<uint16_t>(mask->GetTuple1(i));

      if (((this_mask & mask_filter) || ((mask_filter & (static_cast<uint16_t>(Selector::DARK_MATTER))) && ((this_mask & 0b10) == 0)) && static_cast<uint16_t>(Selector::DARK_AGN) != mask_filter)) {
        if (this_mask & static_cast<uint16_t>(Selector::DARK_AGN)) {
          num++;
          agnPoints->InsertNextPoint(input->data->GetPoints()->GetPoint(i));
        }
      } else {
        hiddenPoints->SetValue(i, vtkDataSetAttributes::HIDDENPOINT);
      }

      if ((this_mask & static_cast<uint16_t>(Selector::BARYON_STAR)) && (mask_filter & static_cast<uint16_t>(Selector::BARYON_STAR))) {
        starPoints->InsertNextPoint(input->data->GetPoints()->GetPoint(i));
      }

      if (this_mask & 0b10) {
        baryonPoints->InsertNextPoint(input->data->GetPoints()->GetPoint(i));
        baryonRhoArr->InsertNextValue(rho->GetValue(i));
        baryonMassArr->InsertNextValue(mass->GetValue(i));
        temperatureArr->InsertNextValue(temperature->GetValue(i));
      }
    }
    printf("[ParticleFilter]: Number of points: %ld\n", num);
  }

  input->agnParticles->SetPoints(agnPoints);

  // double rhoRange[2];
  // baryonRhoArr->GetRange(rhoRange);
  // printf("Rho range for baryons: [%lf, %lf]\n", rhoRange[0], rhoRange[1]);

  input->baryonParticles->SetPoints(baryonPoints);
  input->baryonParticles->GetPointData()->AddArray(baryonMassArr);
  input->baryonParticles->GetPointData()->AddArray(baryonRhoArr);
  input->baryonParticles->GetPointData()->AddArray(temperatureArr);

  input->starParticles->SetPoints(starPoints);

  input->filter->GetPolyDataOutput()->ShallowCopy(input->data);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(hiddenPoints);
}
