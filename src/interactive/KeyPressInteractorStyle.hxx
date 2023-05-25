#pragma once

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h> // for vtkTypeMacro

class vtkCamera;
class VisCos;
class vtkPolyDataMapper;
class vtkRenderWindow;

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera {
private:
public:
  vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);
  static KeyPressInteractorStyle *New();

  VisCos *app;
  vtkRenderWindow *renderWindow;
  vtkPolyDataMapper* dataMapper;

  vtkCamera *camera;
  virtual void OnKeyPress() override;
  virtual void OnChar() override;
};
