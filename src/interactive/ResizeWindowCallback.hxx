#pragma once

#include <vtkCommand.h>

class VisCos;
class vtkObject;

class ResizeWindowCallback : public vtkCommand {
public:
  ResizeWindowCallback(){};
  static ResizeWindowCallback *New() { return new ResizeWindowCallback; }
  VisCos *app;

  void Execute(vtkObject *caller, unsigned long, void *);
};
