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

void KeyPressInteractorStyle::OnKeyPress() {
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
    // Forward should be a little bit faster than other movements
    vec[0] *= 2.3 * this->app->GetMovementAlpha();
    vec[1] *= 2.3 * this->app->GetMovementAlpha();
    vec[2] *= 2.3 * this->app->GetMovementAlpha();
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
    vec[0] *= 1 * this->app->GetMovementAlpha();
    vec[1] *= 1 * this->app->GetMovementAlpha();
    vec[2] *= 1 * this->app->GetMovementAlpha();
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
    vec[0] *= -1 * this->app->GetMovementAlpha();
    vec[1] *= -1 * this->app->GetMovementAlpha();
    vec[2] *= -1 * this->app->GetMovementAlpha();
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
    vec[0] *= this->app->GetMovementAlpha();
    vec[1] *= this->app->GetMovementAlpha();
    vec[2] *= this->app->GetMovementAlpha();
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

  if (key == "0") {
    app->showAll();
    app->UpdateVisbleParticlesText();
    renderWindow->Render();
    return;
  }

  if (key == "9") {
    app->hideAll();
    app->UpdateVisbleParticlesText();
    renderWindow->Render();
    return;
  }

  if (key == "8") {
    app->toggleBaryon();
    app->UpdateVisbleParticlesText();
    renderWindow->Render();
    return;
  }

  if (key == "7") {
    app->toggleDarkMatter();
    app->UpdateVisbleParticlesText();
    renderWindow->Render();
    return;
  }

  if (key == "6") {
    app->toggleBaryonWind();
    app->UpdateVisbleParticlesText();
    renderWindow->Render();
    return;
  }

  if (key == "5") {
    app->toggleBaryonStar();
    app->UpdateVisbleParticlesText();
    renderWindow->Render();
    return;
  }

  if (key == "4") {
    app->toggleBaryonSF();
    app->UpdateVisbleParticlesText();
    renderWindow->Render();
    return;
  }

  if (key == "2") {
    app->toggleDarkAGN();
    app->UpdateVisbleParticlesText();
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

  if (key == "z") {
    this->app->SetMovementAlpha(this->app->GetMovementAlpha() * 0.8);
    printf("Set movement speed to %f\n", this->app->GetMovementAlpha());
    return;
  }
  if (key == "x") {
    this->app->SetMovementAlpha(this->app->GetMovementAlpha() * 1.2);
    printf("Set movement speed to %f\n", this->app->GetMovementAlpha());
    return;
  }

  // Print keyboard shortcuts with "h" (help)
  if (key == "h") {
    printf("Keyboard keybindings:\n");
    printf("  * Move camera with arrow keys\n");
    printf("  * Look around with 'w,a,s,d'\n");
    printf("  * Switch between temperature and cluster with 'c' and 't'\n");
    printf("  * Change amount of steps taken for moving to a timestep with '[' and ']'\n");
    printf("  * '0' to show all particles\n");
    printf("  * '9' to show no particles\n");
    printf("  * '8' to toggle baryon particles\n");
    printf("  * '7' to toggle dark matter particles\n");
    printf("  * '6' to toggle baryon wind particles\n");
    printf("  * '5' to toggle baryon star particles\n");
    printf("  * '4' to toggle baryon star forming\n");
    printf("  * '2' to toggle AGN particles\n");
    printf("  * 'n' to print the current position\n");
    printf("  * 'z' to decrease the movement speed\n");
    printf("  * 'x' to INCREASE the movement speed\n");
    printf("  * Quit with 'q'\n");
  }

  if (key == "n") {
    double pos[3];
    this->camera->GetPosition(pos);

    printf("Current position is %f %f %f\n", pos[0], pos[1], pos[2]);
    return;
  }

  if (key == "q" || key == "Super_L") return;

  // Output the key that was pressed
  printf("Pressed unhandled %s\n", key.c_str());

  // Forward events
  vtkInteractorStyleTrackballCamera::OnKeyPress();
}

vtkStandardNewMacro(KeyPressInteractorStyle);
