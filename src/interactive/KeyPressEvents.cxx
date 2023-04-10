
#include <iostream>                            // for operator<<, endl, bas...
#include <stdio.h>                             // for printf
#include <string>                              // for operator==, allocator
#include <vtkCamera.h>                         // for vtkCamera
#include <vtkInteractorStyleTrackballCamera.h> // for vtkInteractorStyleTra...
#include <vtkObjectFactory.h>                  // for vtkStandardNewMacro
#include <vtkRenderWindowInteractor.h>         // for vtkRenderWindowIntera...
#include <vtkSetGet.h>                         // for vtkTypeMacro

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
  static KeyPressInteractorStyle *New();
  vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);
  vtkCamera *camera;

  virtual void OnKeyPress() override {
    // Get the keypress
    vtkRenderWindowInteractor *rwi = this->Interactor;
    std::string key = rwi->GetKeySym();

    // Output the key that was pressed
    printf(("Pressed " + key + "\n").c_str());

    if (key == "m") {
      printf("The m key was pressed.\n");
      double fpt[3];
      camera->GetFocalPoint(fpt);
      printf("FP is at %lf %lf %lf\n", fpt[0], fpt[1], fpt[2]);
      camera->GetPosition(fpt);
      printf("Position is at %lf %lf %lf\n", fpt[0], fpt[1], fpt[2]);
      return;
    }

    // Forward events
    vtkInteractorStyleTrackballCamera::OnKeyPress();
  }
};
vtkStandardNewMacro(KeyPressInteractorStyle);
