#pragma once

#include <vtkCommand.h>

class VisCos;
class vtkObject;

class TimeSliderCallback : public vtkCommand {
public:
  TimeSliderCallback(){};
  static TimeSliderCallback *New() { return new TimeSliderCallback; }
  VisCos *app;

  void Execute(vtkObject *caller, unsigned long, void *);
};
