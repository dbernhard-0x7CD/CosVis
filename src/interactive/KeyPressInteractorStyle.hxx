#pragma once

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h> // for vtkTypeMacro

class vtkCamera;
class VisCos;
class vtkActor;
class vtkPolyDataMapper;
class vtkRenderWindow;

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera {
private:
public:
  static KeyPressInteractorStyle *New();

  VisCos *app;
  vtkActor* actor;
  vtkRenderWindow *renderWindow;
  vtkPolyDataMapper* dataMapper;

  vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);
  vtkCamera *camera;
  virtual void OnKeyPress() override;
};
