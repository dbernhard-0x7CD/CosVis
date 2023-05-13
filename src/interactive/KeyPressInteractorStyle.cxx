#include <stdio.h>                             // for printf
#include <string>                              // for operator==, allocator
#include <vtkCamera.h>                         // for vtkCamera
#include <vtkInteractorStyleTrackballCamera.h> // for vtkInteractorStyleTra...
#include <vtkMath.h>
#include <vtkObjectFactory.h> // for vtkStandardNewMacro
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h> // for vtkRenderWindowIntera...

#include "../interactive/KeyPressInteractorStyle.hxx"
#include "../app/VisCos.hpp"

// Define interaction style
void KeyPressInteractorStyle::OnKeyPress() {
  // Get the keypress
  vtkRenderWindowInteractor *rwi = this->Interactor;
  std::string key = rwi->GetKeySym();

  if (key == "Up") {
    double pt[3];
    double fpt[3];
    camera->GetPosition(pt);
    camera->GetFocalPoint(fpt);

    double up[] = {0, 0, 0};
    double vec[3];
    double newPt[3];
    double newFpt[3];

    vtkMath::Subtract(fpt, pt, vec); // vec <- fpt - pt

    vtkMath::Normalize(vec);
    vec[0] *= 3;
    vec[1] *= 3;
    vec[2] *= 3;
    // vtkMath::Cross(vec, up, vec);
    vtkMath::Add(pt, vec, newPt);
    vtkMath::Add(fpt, vec, newFpt);

    camera->SetPosition(newPt);
    camera->SetFocalPoint(newFpt);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "Down") {
    double pt[3];
    double fpt[3];
    camera->GetPosition(pt);
    camera->GetFocalPoint(fpt);

    double up[] = {0, 0, 0};
    double vec[3];
    double newPt[3];
    double newFpt[3];

    vtkMath::Subtract(pt, fpt, vec); // vec <- fpt - pt

    vtkMath::Normalize(vec);
    vtkMath::Add(pt, vec, newPt);
    vtkMath::Add(fpt, vec, newFpt);

    camera->SetPosition(newPt);
    camera->SetFocalPoint(newFpt);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "Left") {
    double pt[3];
    double fpt[3];
    camera->GetPosition(pt);
    camera->GetFocalPoint(fpt);

    double up[] = {0, 0, 0};
    camera->GetViewUp(up);
    double vec[3];
    double newPt[3];
    double newFpt[3];

    vtkMath::Subtract(fpt, pt, vec); // vec <- fpt - pt

    vtkMath::Normalize(vec);
    vec[0] *= -1;
    vec[1] *= -1;
    vec[2] *= -1;
    vtkMath::Cross(vec, up, vec);
    vtkMath::Add(pt, vec, newPt);
    vtkMath::Add(fpt, vec, newFpt);

    camera->SetPosition(newPt);
    camera->SetFocalPoint(newFpt);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "Right") {
    double pt[3];
    double fpt[3];
    camera->GetPosition(pt);
    camera->GetFocalPoint(fpt);

    double up[] = {0, 0, 0};
    camera->GetViewUp(up);
    double vec[3];
    double newPt[3];
    double newFpt[3];

    vtkMath::Subtract(fpt, pt, vec); // vec <- fpt - pt

    vtkMath::Normalize(vec);
    vec[0] *= 1;
    vec[1] *= 1;
    vec[2] *= 1;
    vtkMath::Cross(vec, up, vec);
    vtkMath::Add(pt, vec, newPt);
    vtkMath::Add(fpt, vec, newFpt);

    camera->SetPosition(newPt);
    camera->SetFocalPoint(newFpt);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "c") {
    app->ShowClusters();
    return;
  }

  if (key == "t") {
    app->ShowTemperature();
    return;
  }

  if (key == "k") {
    double pos[3];
    camera->GetPosition(pos);
    pos[0] = pos[0] + 1;
    camera->SetPosition(pos);
  }

  if (key == "n") {
    double pos[3];
    camera->GetPosition(pos);
    pos[0] = pos[0] + 1;
    camera->SetPosition(pos);
  }

  if (key == "a") {
    camera->Yaw(2);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "d") {
    camera->Yaw(-1);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "w") {
    camera->Pitch(1);
    camera->Modified();

    renderWindow->Render();
    return;
  }
  if (key == "s") {
    camera->Pitch(-1);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  // Change amount of steps taken when the slider is moved with "[" and "]""
  if (key == "bracketleft") {
    app->lessSteps();
    return;
  }
  if (key == "bracketright") {
    app->moreSteps();
    return;
  }

  // Print keyboard shortcuts with "h" (help)
  if (key == "h") {
    printf("Keyboard keybindings:");
    printf("\t* Move camera with arrow keys\n");
    printf("\t* Look around with 'w,a,s,d'\n");
    printf("\t* Switch between temperature and cluster with 'c' and 't'");
    printf("\t* Change amount of steps taken for moving to a timestep with '[' and ']'");
    printf("\t* Quit with 'q'");
  }

  // Output the key that was pressed
  printf("Pressed unhandled %s\n", key.c_str());

  // Forward events
  vtkInteractorStyleTrackballCamera::OnKeyPress();
}

vtkStandardNewMacro(KeyPressInteractorStyle);
