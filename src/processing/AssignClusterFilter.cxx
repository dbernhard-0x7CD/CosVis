#include <stdio.h>
#include <stdlib.h>

#include <vtkShortArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h> // for vtkPolyData
#include <vtkProgrammableFilter.h>
#include <vtkType.h> // for vtkIdType

#include "AssignClusterFilter.hxx"


void AssignCluster(void *arguments) {
  AssignClusterParams *input = static_cast<AssignClusterParams *>(arguments);

  vtkPoints *inPts = input->data->GetPoints();
  vtkIdType numPts = inPts->GetNumberOfPoints();

  vtkTypeInt64Array *ids = dynamic_cast<vtkTypeInt64Array*>(input->data->GetPointData()->GetArray("id"));
  std::map<int, int>* clustering = input->clustering;

  vtkNew<vtkShortArray> cluster_id;
  cluster_id->SetName("Cluster");
  cluster_id->SetNumberOfComponents(1);

  // TODO: parallelize?
  for (vtkIdType i = 0; i < numPts; i++) {
    int this_id = ids->GetTuple1(i);
    cluster_id->InsertNextValue(((*clustering)[this_id]));
  }

  input->filter->GetPolyDataOutput()->ShallowCopy(input->data);
  input->filter->GetPolyDataOutput()->GetPointData()->AddArray(cluster_id);
}
