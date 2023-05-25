#pragma once

#include <iosfwd> // for ostream
#include <vtkAlgorithm.h>
#include <vtkIOStream.h> // for ostream
#include <vtkSetGet.h>   // for vtkTypeMacro

class vtkDataObject;
class vtkIndent;
class vtkInformation;
class vtkInformationVector;

class vtkImageData;
class vtkPolyData;

class PolyDataToImageDataAlgorithm : public vtkAlgorithm {
public:
  static PolyDataToImageDataAlgorithm *New();
  vtkTypeMacro(PolyDataToImageDataAlgorithm, vtkAlgorithm);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  int dimensions[3];
  double volumeLengths[3];
  double sphOrigin[3];

  vtkImageData *GetOutput();
  vtkImageData *GetOutput(int);
  virtual void SetOutput(vtkDataObject *d);

  virtual int ProcessRequest(vtkInformation *, vtkInformationVector **,
                             vtkInformationVector *) override;

  vtkPolyData *GetInput();
  vtkPolyData *GetInput(int port);

  void SetInput(vtkDataObject *);
  void SetInput(int, vtkDataObject *);

  void AddInput(vtkDataObject *);
  void AddInput(int, vtkDataObject *);

protected:
  PolyDataToImageDataAlgorithm();
  ~PolyDataToImageDataAlgorithm();

  virtual int RequestDataObject(vtkInformation *request,
                                vtkInformationVector **inputVector,
                                vtkInformationVector *outputVector);

  virtual int RequestInformation(vtkInformation *request,
                                 vtkInformationVector **inputVector,
                                 vtkInformationVector *outputVector);

  virtual int RequestData(vtkInformation *request,
                          vtkInformationVector **inputVector,
                          vtkInformationVector *outputVector);

  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
                                  vtkInformationVector *);

  virtual int FillOutputPortInformation(int port,
                                        vtkInformation *info) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  PolyDataToImageDataAlgorithm(
      const PolyDataToImageDataAlgorithm &);            // Not implemented.
  void operator=(const PolyDataToImageDataAlgorithm &); // Not implemented.
};
