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

  vtkNew<vtkUnsignedCharArray> hiddenPoints;

  hiddenPoints->SetName(vtkDataSetAttributes::GhostArrayName());
  hiddenPoints->SetNumberOfValues(numPts);
  hiddenPoints->Fill(0);

  // Based on the filter do stuff
  size_t num = 0;
  uint16_t all_mask = static_cast<uint16_t>(Selector::ALL);
  if ((input->current_filter & all_mask) == all_mask) {
    // keep all particles as is
    printf("[ParticleFilter]: Number of points: %ld\n", inPts->GetNumberOfPoints());
  } else {
    uint16_t mask_filter = static_cast<uint16_t>(input->current_filter);
    for (vtkIdType i = 0; i < numPts; i++) {
      uint16_t this_mask = static_cast<uint16_t>(mask->GetTuple1(i));

      if ((this_mask & mask_filter || ((mask_filter & static_cast<uint16_t>(Selector::DARK_MATTER)) && ((this_mask & 0b10) == 0)))) {
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
