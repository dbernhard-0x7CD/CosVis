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

  // Output the key that was pressed
  printf("Pressed %s\n", key.c_str());

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
    printf("The a key was pressed.\n");
    camera->Yaw(2);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "d") {
    printf("The d key was pressed.\n");
    camera->Yaw(-1);
    camera->Modified();

    renderWindow->Render();
    return;
  }

  if (key == "w") {
    printf("The a key was pressed.\n");
    camera->Pitch(1);
    camera->Modified();

    renderWindow->Render();
    return;
  }
  if (key == "s") {
    printf("The s key was pressed.\n");
    camera->Pitch(-1);
    camera->Modified();

    renderWindow->Render();
    return;
  }
  // Forward events
  vtkInteractorStyleTrackballCamera::OnKeyPress();
}

vtkStandardNewMacro(KeyPressInteractorStyle);
