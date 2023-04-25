
#include "TimeSliderCallback.hxx"

#include <vtkSliderRepresentation.h>
#include <vtkSliderWidget.h>
#include <vtkObject.h>

#include "../app/VisCos.hpp"

void TimeSliderCallback::Execute(vtkObject *caller, unsigned long, void *) {
  vtkSliderWidget *sliderWidget = reinterpret_cast<vtkSliderWidget *>(caller);
  double dvalue = reinterpret_cast<vtkSliderRepresentation *>(
                      sliderWidget->GetRepresentation())
                      ->GetValue();

  // We only have readers for even timesteps, for now.
  int val = (((int)dvalue) / 2) * 2;

  app->MoveToTimestep(val);
}

