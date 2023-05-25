#include "VisCos.hpp"

#include <filesystem>
#include <stdio.h>
#include <utility>
#include <stdint.h>
#include <algorithm>
// IWYU pragma: no_include <bits/chrono.h>

#include <vtkCamera.h>
#include <vtkColor.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkGlyph3D.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkStructuredGrid.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProgrammableFilter.h>
#include <vtkProperty.h>
#include <vtkImageData.h>
#include <vtkSPHInterpolator.h>
#include <vtkSPHQuinticKernel.h>
#include <vtkSliderRepresentation.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkType.h>
#include <vtkTypeInt64Array.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeCollection.h>
#include <vtkXMLPolyDataReader.h>

#include "../data/Loader.h"
#include "../interactive/KeyPressInteractorStyle.hxx"
#include "../interactive/TimeSliderCallback.hxx"
#include "../processing/AssignClusterFilter.hxx"
#include "../processing/CalculateTemperatureFilter.hxx"
#include "../processing/ParticleTypeFilter.hxx"
#include "../processing/PolyDataToImageDataAlgorithm.hxx"
#include "../processing/StarFilter.hxx"
#include "../processing/BaryonFilter.hxx"
#include "../helper/helper.hxx"
#include "../interactive/ResizeWindowCallback.hxx"

namespace fs = std::filesystem;

VisCos::VisCos(int initial_active_timestep, std::string data_folder_path,
               std::string cluster_path) {
  this->active_timestep = initial_active_timestep;
  this->data_folder_path = data_folder_path;
  this->cluster_path = cluster_path;

  this->timeSliderCallback->app = this;
  this->resizeCallback->app = this;

  this->keyboardInteractorStyle->app = this;
  this->keyboardInteractorStyle->renderWindow = this->renderWindow;
  this->keyboardInteractorStyle->dataMapper = this->dataMapper;
  this->keyboardInteractorStyle->SetCurrentRenderer(this->renderer);

  // Makes the point appear at their exact location
  this->singlePointSource->SetRadius(0.0);
  this->singlePointSource->SetNumberOfPoints(1);

  this->sphereSource->SetRadius(0.01);

  this->colors->SetColor("DisabledParticleTypeColor", "#A9A9A9");

  opacityFunction->AddPoint(20, 0.0);
  opacityFunction->AddPoint(700, .4);
  opacityFunction->AddPoint(800, 0.8);
  opacityFunction->AddPoint(2000, 0.999);
  opacityFunction->AddPoint(184026, 1.0);
  opacityFunction->ClampingOn();

  double spacing[3];
  spacing[0] = sphVolumeLengths[0] / dimensions[0];
  spacing[1] = sphVolumeLengths[1] / dimensions[1];
  spacing[2] = sphVolumeLengths[2] / dimensions[2];

  source = GetSPHStructuredGrid(dimensions, spacing, sphOrigin);
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

    // map to positive values as negative values do not work with the 
    // color lookup table (LUT)
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

  this->dataMapper->SetScalarRange(0, 26);
  this->dataMapper->SetLookupTable(this->clusterLUT);
  this->dataMapper->InterpolateScalarsBeforeMappingOff();

  this->scalarBarActor->SetTitle("Clusters");
  // this->manyParticlesActor->GetProperty()->RenderPointsAsSpheresOn();
  this->manyParticlesActor->GetProperty()->SetAmbient(2.3);
  this->manyParticlesActor->GetProperty()->SetPointSize(2.0);
  this->manyParticlesActor->GetProperty()->SetOpacity(0.3);
  this->manyParticlesActor->Modified();

  this->camera->Modified();

  this->dataMapper->Update();
  this->scalarBarWidget->Off();
  this->renderWindow->Render();
}

void VisCos::ShowTemperature() {
  this->dataMapper->SelectColorArray("Temperature");
  this->dataMapper->InterpolateScalarsBeforeMappingOn();
  this->dataMapper->SetLookupTable(this->tempLUT);
  this->dataMapper->SetScalarRange(0, 7000);
  this->dataMapper->Modified();

  this->manyParticlesActor->GetProperty()->SetAmbient(2.3);
  this->manyParticlesActor->GetProperty()->SetPointSize(2.0);
  this->manyParticlesActor->GetProperty()->SetOpacity(0.3);
  this->manyParticlesActor->Modified();

  this->scalarBarActor->SetTitle("Temperature");
  this->scalarBarActor->SetLookupTable(tempLUT);
  this->scalarBarActor->Modified();
  this->scalarBarWidget->Modified();
  this->scalarBarWidget->On();
  this->camera->Modified();

  this->temperatureFilterParams.filter->Update();
  this->dataMapper->Update();
  this->scalarBarWidget->Render();
  this->renderWindow->Render();
}

void VisCos::ShowPhi() {
  this->dataMapper->SelectColorArray("phi");
  this->dataMapper->InterpolateScalarsBeforeMappingOn();
  this->dataMapper->SetScalarRange(this->phiLUT->GetRange());
  this->dataMapper->SetLookupTable(this->phiLUT);
  this->dataMapper->Modified();
  this->dataMapper->Update();

  this->manyParticlesActor->GetProperty()->SetAmbient(1.0);
  this->manyParticlesActor->GetProperty()->SetPointSize(1.5);
  this->manyParticlesActor->GetProperty()->SetOpacity(0.07);

  this->scalarBarActor->SetTitle("Phi (gravitational potential)");
  this->scalarBarActor->SetLookupTable(this->phiLUT);
  this->scalarBarActor->Modified();
  this->scalarBarWidget->Modified();
  this->scalarBarWidget->On();

  this->camera->Modified();

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

  // Filter for stars
  starFilterParams.data = static_cast<vtkPolyData *>(clusterFilter->GetOutput());
  starFilterParams.filter = starFilter;

  starFilter->SetInputConnection(clusterFilter->GetOutputPort());
  starFilter->SetExecuteMethod(StarType, &starFilterParams);
  starFilter->Update();

  // Filter for baryons
  baryonFilterParams.data = static_cast<vtkPolyData *>(clusterFilter->GetOutput());
  baryonFilterParams.filter = baryonFilter;

  baryonFilter->SetInputConnection(clusterFilter->GetOutputPort());
  baryonFilter->SetExecuteMethod(BaryonFilter, &baryonFilterParams);
  baryonFilter->Update();

  // Glyph for many particles
  glyph3D->SetSourceConnection(singlePointSource->GetOutputPort());
  glyph3D->SetInputConnection(particleTypeFilter->GetOutputPort());
  glyph3D->Update();

  // Glyph for stars
  starGlyph3D->SetSourceConnection(sphereSource->GetOutputPort());
  starGlyph3D->SetInputConnection(starFilter->GetOutputPort());
  starGlyph3D->Update();

  // Glyph for marked stuff (dev mode)
  markedData->SetPoints(markedPoints);
  markedPoints->InsertNextPoint(19.75, 42.56, 36.71);

  markedGlyph3D->SetSourceConnection(singlePointSource->GetOutputPort());
  markedGlyph3D->SetInputData(markedData);
  markedGlyph3D->Update();

  // Data Mapper for many particles
  dataMapper->SetInputConnection(glyph3D->GetOutputPort());
  dataMapper->SetScalarModeToUsePointFieldData();

  // Data Mapper for stars
  starDataMapper->SetInputConnection(starGlyph3D->GetOutputPort());

  // Data Mapper for marked particles
  markedDataMapper->SetInputConnection(markedGlyph3D->GetOutputPort());
  markedDataMapper->SetResolveCoincidentTopology(0);
  markedParticlesActor->SetMapper(markedDataMapper);
  markedParticlesActor->GetProperty()->SetPointSize(20.0);
  markedParticlesActor->GetProperty()->SetColor(0, 255, 0); // green

  manyParticlesActor->SetMapper(dataMapper);

  // starParticlesActor->SetOrigin(0,0,0);
  starParticlesActor->SetMapper(starDataMapper);
  starParticlesActor->GetProperty()->SetColor(255, 255, 0); // (255,255,0) is yellow

  // Set up data mapper for interesting particles
  vtkPolyData* baryonFilterOutput = static_cast<vtkPolyData*> (baryonFilter->GetOutput());
  kernel->SetSpatialStep(0.04);
  kernel->SetDimension(3);
  kernel->SetMassArray(baryonFilterOutput->GetPointData()->GetArray("mass"));
  kernel->SetDensityArray(baryonFilterOutput->GetPointData()->GetArray("rho"));

  source->SetDimensions(dimensions);

  vtkNew<vtkStructuredGrid> actualSource;
  actualSource->DeepCopy(source);
  double sourcePoint[3];
  actualSource->GetPoint(0, sourcePoint);
  printf("Source is at %lf %lf %lf\n", sourcePoint[0], sourcePoint[1], sourcePoint[2]);

  // Onto what we interpolate
  interpolator->SetInputData(actualSource);

  // Actual input to interpolate
  // interpolator->SetSourceData(baryonFilter->GetOutput());
  interpolator->SetSourceConnection(baryonFilter->GetOutputPort());
  interpolator->AddExcludedArray("vx");
  interpolator->AddExcludedArray("vz");
  interpolator->AddExcludedArray("vy");
  interpolator->AddExcludedArray("id");

  interpolator->SetMassArrayName("mass");
  interpolator->SetDensityArrayName("rho");
  interpolator->SetKernel(kernel);
  interpolator->Update();

  vtkNew<vtkGlyph3D> sphGlyph;
  sphGlyph->SetSourceConnection(singlePointSource->GetOutputPort());
  sphGlyph->SetInputConnection(interpolator->GetOutputPort());
  sphGlyph->Update();

  vtkNew<vtkSmoothPolyDataFilter> smoothFilter;
  smoothFilter->SetInputConnection(sphGlyph->GetOutputPort());
  smoothFilter->SetNumberOfIterations(50);
  smoothFilter->SetConvergence(0.4);
  smoothFilter->SetRelaxationFactor(0.1);
  smoothFilter->FeatureEdgeSmoothingOff();
  smoothFilter->BoundarySmoothingOn();
  smoothFilter->Update();

  // Convert the vtkPolyData to vtkImageData
  // the vtkImageData has its origin at the same point as the structued points have their origin
  polyDataToImageDataAlgorithm->SetInputConnection(smoothFilter->GetOutputPort());
  std::copy(dimensions, dimensions + 3, polyDataToImageDataAlgorithm->dimensions);
  std::copy(sphOrigin, sphOrigin + 3, polyDataToImageDataAlgorithm->sphOrigin);
  std::copy(sphVolumeLengths, sphVolumeLengths + 3, polyDataToImageDataAlgorithm->volumeLengths);
  polyDataToImageDataAlgorithm->Update();

  volumeProperty->SetColor(GetSPHLUT());
  volumeProperty->SetScalarOpacity(opacityFunction);
  volumeProperty->SetInterpolationTypeToLinear();

  volumeMapper->SetInputConnection(polyDataToImageDataAlgorithm->GetOutputPort());
  volumeMapper->SetInterpolationModeToCubic();
  volumeMapper->ComputeNormalFromOpacityOff();
  volumeMapper->InteractiveAdjustSampleDistancesOff();
  volumeMapper->AutoAdjustSampleDistancesOff();

  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);
  volume->SetOrigin(sphOrigin);

  renderer->AddVolume(volume);

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
  renderer->AddActor(starParticlesActor);
  renderer->AddActor(markedParticlesActor);

  // Scalar bar for the particle colors when showing the temperature
  scalarBarActor->SetOrientationToVertical();
  scalarBarActor->SetLookupTable(tempLUT);
  scalarBarActor->SetPosition2(0.2, 1.5);
  scalarBarActor->SetPosition(1, 1.5);
  scalarBarActor->SetWidth(2);
  scalarBarActor->Modified();

  // This is the camera
  float scale = 50;
  // camera->SetPosition(scale * 3.24, scale * 2.65, scale * 4.09);
  camera->SetPosition(28, 67.7, 45.6);
  camera->SetFocalPoint(scale * 0.51, scale * 0.71, scale * 0.78);
  camera->SetViewAngle(30);
  camera->SetFocalDisk(1.0);
  camera->SetEyeAngle(2);
  camera->SetFocalDistance(0.0);
  camera->SetViewUp(-0.27, 0.91, -0.31);

  renderer->SetActiveCamera(camera);
  renderWindowInteractor->SetInteractorStyle(keyboardInteractorStyle);
  keyboardInteractorStyle->EnabledOn();
  keyboardInteractorStyle->camera = this->camera;
  keyboardInteractorStyle->renderWindow = this->renderWindow;
  renderWindowInteractor->SetRenderWindow(this->renderWindow);

  // Add the focal point as marked point
  UpdateFP();

  // Set initial value on the sliderWidget
  reinterpret_cast<vtkSliderRepresentation *>(
      this->timeSliderWidget->GetRepresentation())
      ->SetValue((double)this->active_timestep);
}

void VisCos::UpdateFP() {
  markedPoints->SetPoint(0, camera->GetFocalPoint());
  markedDataMapper->Modified();
  markedParticlesActor->Modified();
  markedGlyph3D->Modified();
  markedGlyph3D->Update();
}

void VisCos::AddMarkedPoint(double pos[3]) {
  markedPoints->InsertNextPoint(pos);
  markedDataMapper->Modified();
  markedParticlesActor->Modified();
  markedGlyph3D->Modified();
  markedGlyph3D->Update();
}

double *VisCos::GetSPHOrientation() {
  return this->volume->GetOrientation();
}

bool VisCos::IsSPHOn() {
  return this->renderer->GetVolumes()->GetNumberOfItems() > 0;
}

void VisCos::EnableSPH() {
  this->renderer->AddVolume(volume);

  this->sphParticlesActor->SetVisibility(1);
  this->renderer->Modified();
  this->renderWindow->Render();
}

void VisCos::DisableSPH() {
  this->renderer->RemoveVolume(volume);
  this->renderer->Modified();
  this->renderWindow->Render();
}

void VisCos::UpdateSPH() {
  double spacing[3];
  spacing[0] = sphVolumeLengths[0] / dimensions[0];
  spacing[1] = sphVolumeLengths[1] / dimensions[1];
  spacing[2] = sphVolumeLengths[2] / dimensions[2];

  source = GetSPHStructuredGrid(dimensions, spacing, sphOrigin);
  std::copy(sphOrigin, sphOrigin + 3, polyDataToImageDataAlgorithm->sphOrigin);
  polyDataToImageDataAlgorithm->Modified();

  interpolator->SetInputData(source);

  interpolator->Update();
  renderer->Modified();
  renderer->Render();
}

void VisCos::moreSteps() {
  this->steps = std::max(this->steps * 1.1, this->steps + 1.0);
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
    this->starParticlesActor->VisibilityOff();
    hideBaryonStar();
  } else {
    this->starParticlesActor->VisibilityOn();
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
  // 8
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON)) {
    this->textBaryon->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());
  } else {
    this->textBaryon->GetTextProperty()->SetColor(colors->GetColor3d("DisabledParticleTypeColor").GetData());
  }
  this->textBaryon->Modified();

  // 7
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::DARK_MATTER)) {
    this->textDarkMatter->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());
  } else {
    this->textDarkMatter->GetTextProperty()->SetColor(colors->GetColor3d("DisabledParticleTypeColor").GetData());
  }
  this->textDarkMatter->Modified();

  // 6
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_WIND)) {
    this->textBaryonWind->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());
  } else {
    this->textBaryonWind->GetTextProperty()->SetColor(colors->GetColor3d("DisabledParticleTypeColor").GetData());
  }
  this->textBaryonWind->Modified();

  // 5
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_STAR)) {
    this->textBaryonStar->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());
  } else {
    this->textBaryonStar->GetTextProperty()->SetColor(colors->GetColor3d("DisabledParticleTypeColor").GetData());
  }
  this->textBaryonStar->Modified();

  // 4
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::BARYON_STAR_FORMING)) {
    this->textBaryonStarForming->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());
  } else {
    this->textBaryonStarForming->GetTextProperty()->SetColor(colors->GetColor3d("DisabledParticleTypeColor").GetData());
  }
  this->textBaryonStarForming->Modified();

  // 2
  if (this->particleFilterParams.current_filter & static_cast<uint16_t>(Selector::DARK_AGN)) {
    this->textAGN->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());
  } else {
    this->textAGN->GetTextProperty()->SetColor(colors->GetColor3d("DisabledParticleTypeColor").GetData());
  }
  this->textAGN->Modified();

  // Update visible text
  this->renderer->Render();
}

void VisCos::UpdateGUIElements() {
  double* d = this->textVisibleParticles->GetPosition();
  int* size = this->renderWindow->GetActualSize();

  this->textBaryon->SetPosition(d[0] * size[0], d[1] * size[1] - 24);
  this->textDarkMatter->SetPosition(d[0] * size[0], d[1] * size[1] - 2*24);
  this->textBaryonWind->SetPosition(d[0] * size[0], d[1] * size[1] - 3*24);
  this->textBaryonStar->SetPosition(d[0] * size[0], d[1] * size[1] - 4*24);
  this->textBaryonStarForming->SetPosition(d[0] * size[0], d[1] * size[1] - 5*24);
  this->textAGN->SetPosition(d[0] * size[0], d[1] * size[1] - 6*24);

  this->scalarBarActor->Modified();
  this->scalarBarWidget->Modified();

  this->camera->Modified();

  this->scalarBarWidget->Render();
  this->renderWindow->Render();
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
  vtkNew<vtkTextActor> textVisibleParticles;
  this->textVisibleParticles = textVisibleParticles;

  this->textVisibleParticles->SetInput("Visible particles: ");
  this->textVisibleParticles->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
  this->textVisibleParticles->SetPosition(0.02, 0.95);

  double* textVisibleParticlesPosition = this->textVisibleParticles->GetPosition();
  int* size = this->renderWindow->GetActualSize();

  this->textVisibleParticles->GetTextProperty()->SetFontSize(24);

  vtkNew<vtkTextActor> textBaryon;
  this->textBaryon = textBaryon;

  this->textBaryon->SetInput("[8] Baryons");
  this->textBaryon->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
  this->textBaryon->SetPosition(textVisibleParticlesPosition[0] * size[0], textVisibleParticlesPosition[1] * size[1] - 24);
  this->textBaryon->GetTextProperty()->SetFontSize(24);

  vtkNew<vtkTextActor> textDarkMatter;
  this->textDarkMatter = textDarkMatter;

  this->textDarkMatter->SetInput("[7] Dark Matter");
  this->textDarkMatter->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
  this->textDarkMatter->SetPosition(textVisibleParticlesPosition[0] * size[0], textVisibleParticlesPosition[1] * size[1] - 24 * 2);
  this->textDarkMatter->GetTextProperty()->SetFontSize(24);

  vtkNew<vtkTextActor> textBaryonWind;
  this->textBaryonWind = textBaryonWind;

  this->textBaryonWind->SetInput("[6] Wind");
  this->textBaryonWind->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
  this->textBaryonWind->SetPosition(textVisibleParticlesPosition[0] * size[0], textVisibleParticlesPosition[1] * size[1] - 24 * 3);
  this->textBaryonWind->GetTextProperty()->SetFontSize(24);

  vtkNew<vtkTextActor> textBaryonStar;
  this->textBaryonStar = textBaryonStar;

  this->textBaryonStar->SetInput("[5] Stars");
  this->textBaryonStar->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
  this->textBaryonStar->SetPosition(textVisibleParticlesPosition[0] * size[0], textVisibleParticlesPosition[1] * size[1] - 24 * 4);
  this->textBaryonStar->GetTextProperty()->SetFontSize(24);

  vtkNew<vtkTextActor> textBaryonStarForming;
  this->textBaryonStarForming = textBaryonStarForming;

  this->textBaryonStarForming->SetInput("[4] Star Forming");
  this->textBaryonStarForming->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
  this->textBaryonStarForming->SetPosition(textVisibleParticlesPosition[0] * size[0], textVisibleParticlesPosition[1] * size[1] - 24 * 5);
  this->textBaryonStarForming->GetTextProperty()->SetFontSize(24);
  this->textBaryonStarForming->GetTextProperty()->SetColor(colors->GetColor3d("White").GetData());

  vtkNew<vtkTextActor> textAGN;
  this->textAGN = textAGN;

  this->textAGN->SetInput("[2] AGN");
  this->textAGN->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
  this->textAGN->SetPosition(textVisibleParticlesPosition[0] * size[0], textVisibleParticlesPosition[1] * size[1] - 24 * 6);
  this->textAGN->GetTextProperty()->SetFontSize(24);

  renderer->AddActor2D(this->textVisibleParticles);
  renderer->AddActor2D(this->textBaryon);
  renderer->AddActor2D(this->textAGN);
  renderer->AddActor2D(this->textBaryonStar);
  renderer->AddActor2D(this->textBaryonStarForming);
  renderer->AddActor2D(this->textBaryonWind);
  renderer->AddActor2D(this->textDarkMatter);

  // Register callback
  timeSliderWidget->AddObserver(vtkCommand::InteractionEvent, timeSliderCallback);

  // Update the GUI when the window is resized
  renderWindow->AddObserver(vtkCommand::WindowResizeEvent, resizeCallback);

  this->UpdateVisbleParticlesText();

  // This starts the event loop and as a side effect causes an initial render.
  renderWindow->Render();

  printf("Visualizing visuals\n");
  this->renderWindowInteractor->Start();
}

float VisCos::GetMovementAlpha() {
  return this->movementAlpha;
}

void VisCos::SetMovementAlpha(float movementAlpha) {
  this->movementAlpha = movementAlpha;
}

void VisCos::SetSPHCenter(double pos[3]) {
  this->sphOrigin[0] = pos[0] - 0.5 * sphVolumeLengths[0];
  this->sphOrigin[1] = pos[1] - 0.5 * sphVolumeLengths[1];
  this->sphOrigin[2] = pos[2] - 0.5 * sphVolumeLengths[2];
}
