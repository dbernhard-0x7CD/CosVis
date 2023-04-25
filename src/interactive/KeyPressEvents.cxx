#include <stdio.h>                             // for printf
#include <string>                              // for operator==, allocator
#include <vtkCamera.h>                         // for vtkCamera
#include <vtkInteractorStyleTrackballCamera.h> // for vtkInteractorStyleTra...
#include <vtkMath.h>
#include <vtkObjectFactory.h> // for vtkStandardNewMacro
#include <vtkProgrammableFilter.h>
#include <vtkRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h> // for vtkRenderWindowIntera...

#include "KeyPressEvents.hxx"
#include "../processing/CalculateTemperatureFilter.hxx"

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
    vec[0] *= 3;
    vec[1] *= 3;
    vec[2] *= 3;
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

    double up[] = {0, -1, 0};
    double vec[3];
    double newPt[3];
    double newFpt[3];

    vtkMath::Subtract(fpt, pt, vec); // vec <- fpt - pt

    vtkMath::Normalize(vec);
    vec[0] *= 3;
    vec[1] *= 3;
    vec[2] *= 3;
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

    double up[] = {0, 1, 0};
    double vec[3];
    double newPt[3];
    double newFpt[3];

    vtkMath::Subtract(fpt, pt, vec); // vec <- fpt - pt

    vtkMath::Normalize(vec);
    vec[0] *= 3;
    vec[1] *= 3;
    vec[2] *= 3;
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
    this->tempFilterParameters->updateScalarRange = false;
    this->dataMapper->SelectColorArray("Cluster");
    this->dataMapper->SetScalarRange(0, 26);
    this->dataMapper->SetLookupTable(this->clusterLUT);
    this->dataMapper->InterpolateScalarsBeforeMappingOff();

    this->actor->GetProperty()->SetPointSize(1.0);
    this->actor->GetProperty()->SetOpacity(0.5);
    this->actor->GetProperty()->SetLighting(1.0);
    this->actor->GetProperty()->SetAmbient(0.2);
    // this->actor->GetProperty()->SetDiffuse(0.5);
    this->actor->GetProperty()->RenderPointsAsSpheresOn();
    this->tempScalarBarWidget->Off();

    this->camera->Modified();

    this->dataMapper->Update();
    renderWindow->Render();
    return;
  }

  if (key == "t") {
    this->tempFilterParameters->updateScalarRange = true;
    this->tempScalarBarActor->SetLookupTable(this->tempLUT);
    this->tempScalarBarActor->Modified();

    this->dataMapper->SelectColorArray("Temperature");
    this->dataMapper->InterpolateScalarsBeforeMappingOn();
    this->dataMapper->SetLookupTable(this->tempLUT);
    // this->dataMapper->SetScalarRange();
    // this->dataMapper->SetScalarRange(0, 1e3);

    // this->actor->GetProperty()->SetLighting(0.0);
    this->actor->GetProperty()->SetPointSize(1.2);
    this->actor->GetProperty()->SetAmbient(2.3);
    this->actor->GetProperty()->SetPointSize(0.3);
    this->actor->GetProperty()->SetOpacity(0.7);
    this->actor->Modified();

    this->tempScalarBarWidget->On();
    this->tempScalarBarWidget->Modified();
    this->tempFilterParameters->filter->Modified();
    this->camera->Modified();

    this->tempFilterParameters->filter->Update();
    this->dataMapper->Update();
    this->tempScalarBarWidget->Render();
    renderWindow->Render();
    // enable legends and stuff
    return;
  }

  if (key == "k") {
    double pos[3];
    camera->GetPosition(pos);
    pos[0] = pos[0] + 1;
    camera->SetPosition(pos);
  }

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

vtkStandardNewMacro(KeyPressInteractorStyle);
