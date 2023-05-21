
#include "ResizeWindowCallback.hxx"

#include "../app/VisCos.hpp"


void ResizeWindowCallback::Execute(vtkObject *caller, unsigned long, void *) {
  app->UpdateGUIElements();
}

