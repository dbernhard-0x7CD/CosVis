#include <stdio.h>

#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h> // for vtkPolyData
#include <vtkPolyDataMapper.h>
#include <vtkProgrammableFilter.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include "PolyDataToImageDataAlgorithm.hxx"

vtkStandardNewMacro(PolyDataToImageDataAlgorithm);

PolyDataToImageDataAlgorithm::PolyDataToImageDataAlgorithm() {
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

PolyDataToImageDataAlgorithm::~PolyDataToImageDataAlgorithm() {}

void PolyDataToImageDataAlgorithm::PrintSelf(ostream &os, vtkIndent indent) {
  this->Superclass::PrintSelf(os, indent);
}

vtkImageData *PolyDataToImageDataAlgorithm::GetOutput() {
  return this->GetOutput(0);
}

vtkImageData *PolyDataToImageDataAlgorithm::GetOutput(int port) {
  return dynamic_cast<vtkImageData *>(this->GetOutputDataObject(port));
}

void PolyDataToImageDataAlgorithm::SetOutput(vtkDataObject *d) {
  this->GetExecutive()->SetOutputData(0, d);
}

vtkPolyData *PolyDataToImageDataAlgorithm::GetInput() {
  return this->GetInput(0);
}

vtkPolyData *PolyDataToImageDataAlgorithm::GetInput(int port) {
  return static_cast<vtkPolyData *>(
      this->GetExecutive()->GetInputData(port, 0));
}

int PolyDataToImageDataAlgorithm::ProcessRequest(
    vtkInformation *request, vtkInformationVector **inputVector,
    vtkInformationVector *outputVector) {
  // Create an output object of the correct type.
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT())) {
    return this->RequestDataObject(request, inputVector, outputVector);
  }
  // generate the data
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA())) {
    return this->RequestData(request, inputVector, outputVector);
  }

  if (request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT())) {
    return this->RequestUpdateExtent(request, inputVector, outputVector);
  }

  // execute information
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION())) {
    return this->RequestInformation(request, inputVector, outputVector);
  }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

int PolyDataToImageDataAlgorithm::FillOutputPortInformation(
    int vtkNotUsed(port), vtkInformation *info) {
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
  return 1;
}

int PolyDataToImageDataAlgorithm::FillInputPortInformation(
    int vtkNotUsed(port), vtkInformation *info) {
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

int PolyDataToImageDataAlgorithm::RequestDataObject(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **vtkNotUsed(inputVector),
    vtkInformationVector *outputVector) {
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkPolyData *output =
      dynamic_cast<vtkPolyData *>(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!output) {
    output = vtkPolyData::New();
    outInfo->Set(vtkDataObject::DATA_OBJECT(), output);
    output->FastDelete();

    this->GetOutputPortInformation(0)->Set(vtkDataObject::DATA_EXTENT_TYPE(),
                                           output->GetExtentType());
  }

  return 1;
}

int PolyDataToImageDataAlgorithm::RequestInformation(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **vtkNotUsed(inputVector),
    vtkInformationVector *vtkNotUsed(outputVector)) {
  return 1;
}

int PolyDataToImageDataAlgorithm::RequestUpdateExtent(
    vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector,
    vtkInformationVector *vtkNotUsed(outputVector)) {
  int numInputPorts = this->GetNumberOfInputPorts();
  for (int i = 0; i < numInputPorts; i++) {
    int numInputConnections = this->GetNumberOfInputConnections(i);
    for (int j = 0; j < numInputConnections; j++) {
      vtkInformation *inputInfo = inputVector[i]->GetInformationObject(j);
      inputInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
    }
  }
  return 1;
}

int PolyDataToImageDataAlgorithm::RequestData(
    vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector,
    vtkInformationVector *outputVector) {
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

  vtkPolyData *input = dynamic_cast<vtkPolyData *>(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *output = dynamic_cast<vtkImageData *>(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetDimensions(this->dimensions);
  output->SetSpacing(this->volumeLengths[0] / this->dimensions[0],
                     this->volumeLengths[1] / this->dimensions[1],
                     this->volumeLengths[2] / this->dimensions[2]);
  output->SetOrigin(this->sphOrigin);

  output->AllocateScalars(VTK_DOUBLE, 1);

  for (vtkIdType i = 0; i < input->GetNumberOfPoints(); i++) {
    output->GetPointData()->GetScalars()->SetTuple1(
        i, input->GetPointData()->GetArray("rho")->GetTuple1(i));
  }

  return 1;
}

void PolyDataToImageDataAlgorithm::SetInput(vtkDataObject *input) {
  this->SetInput(0, input);
}

void PolyDataToImageDataAlgorithm::SetInput(int index, vtkDataObject *input) {
  if (input) {
    this->SetInputDataObject(index, input);
  } else {
    this->SetInputDataObject(index, 0);
  }
}

void PolyDataToImageDataAlgorithm::AddInput(vtkDataObject *input) {
  this->AddInput(0, input);
}

void PolyDataToImageDataAlgorithm::AddInput(int index, vtkDataObject *input) {
  if (input) {
    this->AddInputDataObject(index, input);
  }
}
