#include "PointToPolyDataFilter.hxx"

#include <vtkDataObject.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkSmartPointer.h>

PointToPolyDataFilter::PointToPolyDataFilter() {
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

PointToPolyDataFilter::~PointToPolyDataFilter() {}

int PointToPolyDataFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                       vtkInformationVector **inputVector,
                                       vtkInformationVector *outputVector) {
  vtkPolyData *input = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData *output = vtkPolyData::GetData(outputVector, 0);

  output->ShallowCopy(input);

  return 1;
}

void PointToPolyDataFilter::PrintSelf(ostream &os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
}

vtkStandardNewMacro(PointToPolyDataFilter);
