#pragma once

#include <vtkPolyDataAlgorithm.h>

class PointToPolyDataFilter : public vtkPolyDataAlgorithm {
public:
  vtkTypeMacro(PointToPolyDataFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream &os, vtkIndent indent);

  static PointToPolyDataFilter *New();

protected:
  PointToPolyDataFilter();
  ~PointToPolyDataFilter();

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);

private:
  PointToPolyDataFilter(const PointToPolyDataFilter &);
  void operator=(const PointToPolyDataFilter &);
};
