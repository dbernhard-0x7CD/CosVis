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
#include <vtkType.h>
#include <vtkTypeInt64Array.h>
#include <vtkXMLPolyDataReader.h>

#include "../data/Loader.h"
#include "../interactive/KeyPressInteractorStyle.hxx"
#include "../interactive/TimeSliderCallback.hxx"
#include "../processing/AssignClusterFilter.hxx"
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
  this->keyboardInteractorStyle->actor = this->actor;
  this->keyboardInteractorStyle->dataMapper = this->dataMapper;
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
  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes are
  // defined.
  this->dataMapper->ScalarVisibilityOn();
  this->dataMapper->SelectColorArray("Cluster");

  this->temperatureFilterParams.updateScalarRange = false;
  this->dataMapper->SetScalarRange(0, 26);
  this->dataMapper->SetLookupTable(this->clusterLUT);
  this->dataMapper->InterpolateScalarsBeforeMappingOff();

  this->actor->GetProperty()->SetPointSize(1.0);
  this->actor->GetProperty()->SetOpacity(0.5);
  this->actor->GetProperty()->SetLighting(1.0);
  this->actor->GetProperty()->SetAmbient(0.2);
  // this->actor->GetProperty()->SetDiffuse(0.5);
  this->actor->GetProperty()->RenderPointsAsSpheresOn();
  this->scalarBarWidget->Off();

  this->camera->Modified();

  this->dataMapper->Update();
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

  // this->actor->GetProperty()->SetLighting(0.0);
  this->actor->GetProperty()->SetPointSize(1.2);
  this->actor->GetProperty()->SetAmbient(2.3);
  this->actor->GetProperty()->SetPointSize(0.3);
  this->actor->GetProperty()->SetOpacity(0.7);
  this->actor->Modified();

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
  // Set the background color.
  this->colors->SetColor("BkgColor", color);

  this->renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
}

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
  // activeReader->Update();

  temperatureFilter->SetExecuteMethod(CalculateTemperature,
                                      &temperatureFilterParams);
  temperatureFilter->Update();

  // This filter adds a column with the cluster index
  clusterFilter->SetInputData(temperatureFilter->GetOutput());

  clusterFilterParams.data =
      static_cast<vtkPolyData *>(temperatureFilter->GetOutput());
  clusterFilterParams.filter = clusterFilter;
  clusterFilterParams.clustering = &clusters;

  clusterFilter->SetExecuteMethod(AssignCluster, &clusterFilterParams);
  clusterFilter->Update();

  glyph3D->SetSourceConnection(ptSource->GetOutputPort());
  glyph3D->SetInputConnection(clusterFilter->GetOutputPort());
  glyph3D->Update();

  dataMapper->SetInputConnection(glyph3D->GetOutputPort());

  dataMapper->SetScalarModeToUsePointFieldData();
  dataMapper->SelectColorArray("mass");

  actor->SetMapper(dataMapper);

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

  renderer->AddActor(actor);

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

void VisCos::Run() {
  timeSliderRepr->SetMinimumValue(0.0);
  timeSliderRepr->SetMaximumValue(625);
  timeSliderRepr->SetValue((double)this->active_timestep);
  timeSliderRepr->SetTitleText("Timestep");
  timeSliderRepr->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
  timeSliderRepr->GetPoint1Coordinate()->SetValue(40, 80);
  timeSliderRepr->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
  timeSliderRepr->GetPoint2Coordinate()->SetValue(320, 80);

  timeSliderWidget->SetInteractor(renderWindowInteractor);
  timeSliderWidget->SetRepresentation(timeSliderRepr);
  timeSliderWidget->SetNumberOfAnimationSteps(10);
  timeSliderWidget->SetAnimationModeToAnimate();
  timeSliderWidget->On();

  // Register callback
  timeSliderWidget->AddObserver(vtkCommand::InteractionEvent,
                                timeSliderCallback);

  // This starts the event loop and as a side effect causes an initial render.
  renderWindow->Render();

  printf("Visualizing visuals\n");
  this->renderWindowInteractor->Start();
}