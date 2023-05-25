#include <stdio.h>

#include <vtkDataSetAttributes.h> // for vtkDataSetAttributes
#include <vtkDoubleArray.h>
#include <vtkGenericDataArray.txx> // for vtkGenericDataArray::InsertNextValue
#include <vtkIntArray.h> // for vtkIntArray
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>   // for vtkPoints
#include <vtkPolyData.h> // for vtkPolyData
#include <vtkProgrammableFilter.h>
#include <vtkType.h>              // for vtkIdType

#include "ParticleTypeFilter.hxx"

void FilterType(void *arguments) {
  ParticleTypeFilterParams *input =
      static_cast<ParticleTypeFilterParams *>(arguments);

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

  vtkNew<vtkUnsignedCharArray> hiddenPoints;
  hiddenPoints->SetName(vtkDataSetAttributes::GhostArrayName());
  hiddenPoints->SetNumberOfValues(numPts);
  hiddenPoints->Fill(0);

  // Based on the filter do stuff
  size_t num = 0;
  uint16_t all_mask = static_cast<uint16_t>(Selector::ALL);
  uint16_t mask_filter = static_cast<uint16_t>(input->current_filter);
  double pos[3];
  if ((input->current_filter & all_mask) == all_mask) {
    // keep all particles as is
    printf("[ParticleFilter]: ALL filter is active, Number of points: %lld\n",
           inPts->GetNumberOfPoints());

    for (vtkIdType i = 0; i < numPts; i++) {
      uint16_t this_mask = static_cast<uint16_t>(mask->GetTuple1(i));
      input->data->GetPoints()->GetPoint(i, pos);
    }
  } else {
    for (vtkIdType i = 0; i < numPts; i++) {
      uint16_t this_mask = static_cast<uint16_t>(mask->GetTuple1(i));
      input->data->GetPoints()->GetPoint(i, pos);

      if (((this_mask & mask_filter) ||
           ((mask_filter & (static_cast<uint16_t>(Selector::DARK_MATTER))) &&
            ((this_mask & 0b10) == 0)) &&
               static_cast<uint16_t>(Selector::DARK_AGN) != mask_filter)) {
        num++;
      } else {
        hiddenPoints->SetValue(i, vtkDataSetAttributes::HIDDENPOINT);
      }
    }
    printf("[ParticleFilter]: Number of points: %ld\n", num);
  }

  input->filter->GetPolyDataOutput()->ShallowCopy(input->data);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(hiddenPoints);
}
