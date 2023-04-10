#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h> // for vtkTypeMacro
class vtkCamera;

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
  static KeyPressInteractorStyle *New();
  vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);
  vtkCamera *camera;
  virtual void OnKeyPress() override;
};
