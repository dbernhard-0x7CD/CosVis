#include "VisCos.hpp"

#include <filesystem>
#include <stdio.h>
#include <utility>

#include <vtkCamera.h>
#include <vtkColor.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkGlyph3D.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProgrammableFilter.h>
#include <vtkProperty.h>
#include <vtkSliderRepresentation.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkType.h>
#include <vtkTypeInt64Array.h>
#include <vtkXMLPolyDataReader.h>

#include "../data/Loader.h"
#include "../interactive/KeyPressInteractorStyle.hxx"
#include "../interactive/TimeSliderCallback.hxx"
#include "../processing/AssignClusterFilter.hxx"
#include "../processing/ParticleTypeFilter.hxx"
#include "../processing/CalculateTemperatureFilter.hxx"

namespace fs = std::filesystem;

VisCos::VisCos(int initial_active_timestep, std::string data_folder_path,
               std::string cluster_path) {
  this->active_timestep = initial_active_timestep;
  this->data_folder_path = data_folder_path;
  this->cluster_path = cluster_path;
  this->timeSliderCallback->app = this;
  this->keyboardInteractorStyle->app = this;
  this->keyboardInteractorStyle->renderWindow = this->renderWindow;
  this->keyboardInteractorStyle->dataMapper = this->dataMapper;

  this->singlePointSource->SetCenter(0, 0, 0);
  this->singlePointSource->SetNumberOfPoints(1);
}

void VisCos::Load() {
  std::map<int, fs::path> files =
      load_cosmology_dataset(data_folder_path);
  printf("Loaded %lu files.\n", files.size());

  for (auto path : files) {
    vtkXMLPolyDataReader *reader = vtkXMLPolyDataReader::New();
    int index = path.first;
    reader->SetFileName(path.second.c_str());

    this->dataset_readers.insert_or_assign(path.first, reader);
  }
  printf("Finished creating %ld data loaders.\n", this->dataset_readers.size());

  // Load cluster assignments
  vtkNew<vtkXMLPolyDataReader> clusterReader;
  clusterReader->SetFileName(cluster_path.c_str());
  clusterReader->Update();

  vtkIdType num = clusterReader->GetNumberOfPoints();

  vtkPolyData *data = clusterReader->GetOutput();
  vtkTypeInt64Array *ids =
      static_cast<vtkTypeInt64Array *>(data->GetPointData()->GetArray("id"));
  vtkTypeInt64Array *carr = static_cast<vtkTypeInt64Array *>(
      data->GetPointData()->GetArray("cluster_id"));

  for (vtkIdType i = 0; i < num; i++) {
    int id = ids->GetValue(i);
    int cluster = carr->GetValue(i);
    if (cluster == -1) {
      clusters.insert_or_assign(id, 26);
    } else {
      clusters.insert_or_assign(id, cluster);
    }
  }

  // print point ids which have no cluster assigned
  for (vtkIdType i = 0; i < num; i++) {
    if (clusters.find(i) == clusters.end()) {
      printf("Missing point in cluster ass: %lld\n", i);
    }
  }
  printf("Finished reading %lld clusters\n", num);
}

void VisCos::MoveToTimestep(int step) {
  if (step % 2 == 1) {
    printf("Tried to move to timestep %d which is invalid. We only have even "
           "steps (for now).\n",
           step);
    return;
  }
  if (step == this->active_timestep) return;

  // Set the active reader and get its output to be the polydata
  activeReader = dataset_readers.at((vtkIdType)step);

  activeReader->Update();
  reinterpret_cast<vtkSliderRepresentation *>(
      this->timeSliderWidget->GetRepresentation())
      ->SetValue((double)step);
  this->timeSliderRepr->Modified();
  this->timeSliderWidget->Modified();

  this->temperatureFilter->SetInputConnection(activeReader->GetOutputPort());
  this->temperatureFilterParams.data = activeReader->GetOutput();

  // Update the filter which comes next
  this->temperatureFilter->Update();

  printf("Switching to timestep: %d\n", step);
  this->camera->Modified();

  this->renderWindow->Render();
  this->active_timestep = step;
}

void VisCos::ShowClusters() {
  this->dataMapper->ScalarVisibilityOn();
  this->dataMapper->SelectColorArray("Cluster");

  this->temperatureFilterParams.updateScalarRange = false;
  this->dataMapper->SetScalarRange(0, 26);
  this->dataMapper->SetLookupTable(this->clusterLUT);
  this->dataMapper->InterpolateScalarsBeforeMappingOff();

  this->manyParticlesActor->GetProperty()->SetLighting(1.0);
  this->manyParticlesActor->GetProperty()->RenderPointsAsSpheresOn();
  this->manyParticlesActor->GetProperty()->SetAmbient(0.2);
  this->manyParticlesActor->GetProperty()->SetPointSize(1.0);
  this->manyParticlesActor->GetProperty()->SetOpacity(0.5);
  this->manyParticlesActor->Modified();

  this->camera->Modified();

  this->dataMapper->Update();
  this->scalarBarWidget->Off();
  this->renderWindow->Render();
}

void VisCos::ShowTemperature() {
  this->temperatureFilterParams.updateScalarRange = true;
  this->scalarBarActor->Modified();

  this->dataMapper->SelectColorArray("Temperature");
  this->dataMapper->InterpolateScalarsBeforeMappingOn();
  this->dataMapper->SetLookupTable(this->tempLUT);
  this->dataMapper->Modified();
  this->dataMapper->Update();

  // this->manyParticlesActor->GetProperty()->SetLighting(0.0);
  this->manyParticlesActor->GetProperty()->SetAmbient(2.3);
  this->manyParticlesActor->GetProperty()->SetPointSize(0.3);
  this->manyParticlesActor->GetProperty()->SetOpacity(0.7);
  this->manyParticlesActor->Modified();

  this->scalarBarWidget->On();
  this->scalarBarWidget->Modified();
  this->temperatureFilterParams.filter->Modified();
  this->camera->Modified();

  this->temperatureFilterParams.filter->Update();
  this->dataMapper->Update();
  this->scalarBarWidget->Render();
  this->renderWindow->Render();
}

void VisCos::SetBackgroundColor(std::string color) {
  this->colors->SetColor("BkgColor", color);

  this->renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
}

/*
  Current pipeline:
    * dataset_readers
    * activeReader      [chosen timestep]
    * temperatureFilter
    * clusterFilter
    * particleTypeFilter
    * glyph3D
    * 
*/
void VisCos::SetupPipeline() {
  // First we set up the data pipeline
  vtkXMLPolyDataReader *activeReader;
  activeReader = this->dataset_readers.at(this->active_timestep);

  // This filter calculates the temperature
  temperatureFilter->SetInputConnection(activeReader->GetOutputPort());
  this->temperatureFilterParams.data = activeReader->GetOutput();

  this->temperatureFilterParams.filter = temperatureFilter;
  this->temperatureFilterParams.mapper = dataMapper;
  this->temperatureFilterParams.updateScalarRange = false;

  temperatureFilter->SetExecuteMethod(CalculateTemperature,
                                      &temperatureFilterParams);
  temperatureFilter->Update();

  // This filter adds a column with the cluster index
  clusterFilter->SetInputData(temperatureFilter->GetOutput());

  clusterFilterParams.data = static_cast<vtkPolyData *>(temperatureFilter->GetOutput());
  clusterFilterParams.filter = clusterFilter;
  clusterFilterParams.clustering = &clusters;

  clusterFilter->SetExecuteMethod(AssignCluster, &clusterFilterParams);
  clusterFilter->Update();

  // Filters on the different types of particles
  particleTypeFilter->SetInputConnection(clusterFilter->GetOutputPort());

  particleFilterParams.data = static_cast<vtkPolyData *>(clusterFilter->GetOutput());
  particleFilterParams.filter = particleTypeFilter;
  particleFilterParams.current_filter = static_cast<uint16_t>(Selector::ALL);

  particleTypeFilter->SetExecuteMethod(FilterType, &particleFilterParams);
  particleTypeFilter->Update();

  glyph3D->SetSourceConnection(singlePointSource->GetOutputPort());
  glyph3D->SetInputConnection(particleTypeFilter->GetOutputPort());
  glyph3D->Update();

  dataMapper->SetInputConnection(glyph3D->GetOutputPort());
  dataMapper->SetScalarModeToUsePointFieldData();
  dataMapper->SelectColorArray("mass");

  manyParticlesActor->SetMapper(dataMapper);

  // Set up data mapper for interesting particles
  vtkNew<vtkPoints> interestingPoints;
  vtkNew<vtkPolyData> importantPointsData;
  vtkNew<vtkGlyph3D> importantGlyph;

  interestingPoints->InsertNextPoint(3, 36, 18);
  importantPointsData->SetPoints(interestingPoints);


  importantGlyph->SetSourceConnection(singlePointSource->GetOutputPort());
  importantGlyph->SetInputData(importantPointsData);
  importantGlyph->Update();

  interestingDataMapper->SetInputConnection(importantGlyph->GetOutputPort());
  interestingDataMapper->SetScalarModeToUsePointFieldData();

  interestingParticlesActor->GetProperty()->RenderPointsAsSpheresOn();
  interestingParticlesActor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
  interestingParticlesActor->GetProperty()->SetPointSize(15.0);
  
  interestingParticlesActor->SetMapper(interestingDataMapper);

  // Now setup some GUI/Interaction elements
  // create the scalarBarWidget
  scalarBarWidget->SetInteractor(renderWindowInteractor);
  scalarBarWidget->SetScalarBarActor(scalarBarActor);
  scalarBarWidget->On();

  // The render window is the actual GUI window
  // that appears on the computer screen
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("VisCos");

  // The render window interactor captures mouse events
  // and will perform appropriate camera or actor manipulation
  // depending on the nature of the events.
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->Initialize();

  renderer->AddActor(manyParticlesActor);
  renderer->AddActor(interestingParticlesActor);

  // Scalar bar for the particle colors when showing the temperature
  scalarBarActor->SetOrientationToVertical();
  scalarBarActor->SetLookupTable(tempLUT);
  scalarBarActor->SetPosition2(0.2, 1.5);
  scalarBarActor->SetPosition(1, 1.5);
  scalarBarActor->SetWidth(2);
  scalarBarActor->Modified();

  // This is the camera
  float scale = 50;
  // camera->SetFocalDistance(0.1);
  camera->SetPosition(scale * 3.24, scale * 2.65, scale * 4.09);
  camera->SetFocalPoint(scale * 0.51, scale * 0.71, scale * 0.78);
  camera->SetFocalDisk(1.0);
  camera->SetEyeAngle(30);
  camera->Zoom(1.0);
  // camera->SetFocalDistance(1.0);
  camera->SetViewUp(-0.27, 0.91, -0.31);

  renderer->SetActiveCamera(camera);
  renderWindowInteractor->SetInteractorStyle(keyboardInteractorStyle);
  keyboardInteractorStyle->EnabledOn();
  keyboardInteractorStyle->camera = this->camera;
  keyboardInteractorStyle->renderWindow = this->renderWindow;
  renderWindowInteractor->SetRenderWindow(this->renderWindow);

  // Set initial value on the sliderWidget
  reinterpret_cast<vtkSliderRepresentation *>(
      this->timeSliderWidget->GetRepresentation())
      ->SetValue((double)this->active_timestep);
}

void VisCos::moreSteps() {
  this->steps *= 1.1;
  this->timeSliderWidget->SetNumberOfAnimationSteps(this->steps);
  printf("New steps: %d\n", this->steps);
}

void VisCos::lessSteps() {
  this->steps *= 0.9;
  this->timeSliderWidget->SetNumberOfAnimationSteps(this->steps);
  printf("New steps: %d\n", this->steps);
}

void VisCos::showAll() {
  this->particleFilterParams.current_filter = static_cast<uint16_t>(Selector::ALL);
  this->particleTypeFilter->Modified();
  printf("Showing all particles.\n");
}

void VisCos::hideAll() {
  this->particleFilterParams.current_filter = static_cast<uint16_t>(Selector::NONE);
  this->particleTypeFilter->Modified();
  printf("Hiding all particles.\n");
}

void VisCos::showBaryon() {
  this->particleFilterParams.current_filter |= static_cast<uint16_t>(Selector::BARYON);
  this->particleTypeFilter->Modified();
  printf("Showing all baryon particles.\n");
}
void VisCos::hideBaryon() {
  this->particleFilterParams.current_filter &= ~static_cast<uint16_t>(Selector::BARYON);
  this->particleTypeFilter->Modified();
  printf("Hiding all baryon particles.\n");
}

void VisCos::toggleBaryon() {
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON)) {
    hideBaryon();
  } else {
    showBaryon();
  }
}

void VisCos::showDarkMatter() {
  this->particleFilterParams.current_filter |= static_cast<uint16_t>(Selector::DARK_MATTER);
  this->particleTypeFilter->Modified();
  printf("Showing DarkMatter particles.\n");
}

void VisCos::hideDarkMatter() {
  this->particleFilterParams.current_filter &= ~static_cast<uint16_t>(Selector::DARK_MATTER);
  this->particleTypeFilter->Modified();
  printf("Hiding DarkMatter particles.\n");
}

void VisCos::toggleDarkMatter() {
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::DARK_MATTER)) {
    hideDarkMatter();
  } else {
    showDarkMatter();
  }
}

void VisCos::showBaryonStar() {
  this->particleFilterParams.current_filter |= static_cast<uint16_t>(Selector::BARYON_STAR);
  this->particleTypeFilter->Modified();
  printf("Showing Baryon Star particles.\n");
}

void VisCos::hideBaryonStar() {
  this->particleFilterParams.current_filter &= ~static_cast<uint16_t>(Selector::BARYON_STAR);
  this->particleTypeFilter->Modified();
  printf("Hiding Baryon Star particles.\n");
}

void VisCos::toggleBaryonStar() {
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_STAR)) {
    hideBaryonStar();
  } else {
    showBaryonStar();
  }
}

void VisCos::showBaryonWind() {
  this->particleFilterParams.current_filter |= static_cast<uint16_t>(Selector::BARYON_WIND);
  this->particleTypeFilter->Modified();
  printf("Showing Baryon Wind particles.\n");
}

void VisCos::hideBaryonWind() {
  this->particleFilterParams.current_filter &= ~static_cast<uint16_t>(Selector::BARYON_WIND);
  this->particleTypeFilter->Modified();
  printf("Hiding Baryon Wind particles.\n");
}

void VisCos::toggleBaryonWind() {
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_WIND)) {
    hideBaryonWind();
  } else {
    showBaryonWind();
  }
}

void VisCos::showBaryonSF() {
  this->particleFilterParams.current_filter |= static_cast<uint16_t>(Selector::BARYON_STAR_FORMING);
  this->particleTypeFilter->Modified();
  printf("Showing baryon star forming particles\n");
}

void VisCos::hideBaryonSF() {
  this->particleFilterParams.current_filter &= ~static_cast<uint16_t>(Selector::BARYON_STAR_FORMING);
  this->particleTypeFilter->Modified();
  printf("Hiding baryon star forming particles\n");
}

void VisCos::toggleBaryonSF() {
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_STAR_FORMING)) {
    hideBaryonSF();
  } else {
    showBaryonSF();
  }
}

void VisCos::showDarkAGN() {
  this->particleFilterParams.current_filter |= static_cast<uint16_t>(Selector::DARK_AGN);
  this->particleTypeFilter->Modified();
  printf("Showing dark matter AGN particles\n");
}

void VisCos::hideDarkAGN() {
  this->particleFilterParams.current_filter &= ~static_cast<uint16_t>(Selector::DARK_AGN);
  this->particleTypeFilter->Modified();
  printf("Hiding dark matter AGN particles\n");
}

void VisCos::toggleDarkAGN() {
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::DARK_AGN)) {
    hideDarkAGN();
  } else {
    showDarkAGN();
  }
}

void VisCos::UpdateVisbleParticlesText() {
  if (this->particleFilterParams.current_filter == static_cast<uint16_t>(Selector::ALL)) {
    this->visibleParticlesText->SetInput("Visible particles: ALL");
    this->visibleParticlesText->Modified();
    return;
  }
  if (this->particleFilterParams.current_filter == static_cast<uint16_t>(Selector::NONE)) {
    this->visibleParticlesText->SetInput("Visible particles: NONE");
    this->visibleParticlesText->Modified();
    return;
  }

  std::string visParticles("Visible particles: ");

  // 8
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON)) {
    visParticles += "Baryons, ";
  }

  // 7
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::DARK_MATTER)) {
    visParticles += "Dark matter, ";
  }

  // 6
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_WIND)) {
    visParticles += "Wind, ";
  }

  // 5
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_STAR)) {
    visParticles += "Stars, ";
  }

  // 4
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_STAR_FORMING)) {
    visParticles += "Star forming, ";
  }

  // 2
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::DARK_AGN)) {
    visParticles += "AGN, ";
  }

  // Remove the last two characters
  visParticles.pop_back();
  visParticles.pop_back();

  // Update visible text
  this->visibleParticlesText->SetInput(visParticles.c_str());
  this->visibleParticlesText->Modified();
}

void VisCos::Run() {
  timeSliderRepr->SetMinimumValue(0.0);
  timeSliderRepr->SetMaximumValue(625);
  timeSliderRepr->SetValue((double)this->active_timestep);
  timeSliderRepr->SetTitleText("Timestep");
  timeSliderRepr->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  timeSliderRepr->GetPoint1Coordinate()->SetValue(0.05, 0.10);
  timeSliderRepr->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  timeSliderRepr->GetPoint2Coordinate()->SetValue(0.35, 0.10);

  timeSliderWidget->SetInteractor(renderWindowInteractor);
  timeSliderWidget->SetRepresentation(timeSliderRepr);
  timeSliderWidget->SetNumberOfAnimationSteps(this->steps);
  timeSliderWidget->SetAnimationModeToAnimate();
  timeSliderWidget->On();

  // Show which particles are visible
  vtkNew<vtkTextActor> visibleParticlesText;
  this->visibleParticlesText = visibleParticlesText;

  this->visibleParticlesText->SetInput("Visible particles: ");
  this->visibleParticlesText->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
  this->visibleParticlesText->SetPosition(0.02, 0.95);
  this->visibleParticlesText->GetTextProperty()->SetFontSize(24);
  this->visibleParticlesText->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());
  renderer->AddActor2D(visibleParticlesText);

  // Register callback
  timeSliderWidget->AddObserver(vtkCommand::InteractionEvent,
                                timeSliderCallback);

  this->UpdateVisbleParticlesText();

  // This starts the event loop and as a side effect causes an initial render.
  renderWindow->Render();

  printf("Visualizing visuals\n");
  this->renderWindowInteractor->Start();
}