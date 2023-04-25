#include <filesystem>
#include <iostream>
#include <map> // for map, operator!=
#include <stdio.h> // for printf
#include <stdlib.h>
#include <string>
#include <utility> // for pair
#include <list>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkColor.h>      // for vtkColor3d
#include <vtkCommand.h>    // for vtkCommand
#include <vtkCoordinate.h> // for vtkCoordinate
#include <vtkCylinderSource.h>
#include <vtkDataArray.h> // for vtkDataArray
#include <vtkGlyph2D.h>
#include <vtkGlyph3D.h>               // for vtkGlyph3D
#include <vtkInteractorStyleSwitch.h> // for vtkInteractorSt...
#include <vtkInteractorStyleTrackball.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPointSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataReader.h>
#include <vtkProgrammableFilter.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkSliderRepresentation.h> // for vtkSliderRepres...
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>

#include "data/Loader.h"
#include "processing/CalculateTemperatureFilter.hxx"
#include "processing/AssignClusterFilter.hxx"
#include "interactive/KeyPressInteractorStyle.hxx"
#include "helper/helper.hxx"

namespace fs = std::filesystem;

namespace {

class vtkSliderCallback : public vtkCommand {
public:
  vtkSliderCallback(){};

  // Defines the filter that comes after this
  vtkProgrammableFilter* nextFilter;

  vtkSliderCallback(vtkPolyData *displayData,
                    std::map<int, vtkXMLPolyDataReader *> *readers) {
    this->readers = readers;
    this->displayData = displayData;
  }

  static vtkSliderCallback *New() { return new vtkSliderCallback; }
  static vtkSliderCallback *
  New(vtkPolyData *displayData,
      std::map<int, vtkXMLPolyDataReader *> *readers) {
    return new vtkSliderCallback(displayData, readers);
  }

  /*
   Updates the data to match the data at timestep set by the caller which is
   a vtkSliderWidget.
   Updates the displayData.
  */
  virtual void Execute(vtkObject *caller, unsigned long, void *) {
    vtkSliderWidget *sliderWidget = reinterpret_cast<vtkSliderWidget *>(caller);
    double dvalue = reinterpret_cast<vtkSliderRepresentation *>(
                        sliderWidget->GetRepresentation())
                        ->GetValue();

    // We only have readers for even timesteps
    int val = (((int)dvalue) / 2) * 2;

    vtkXMLPolyDataReader *activeReader;
    activeReader = this->readers->at(val);
    activeReader->Update();
    reinterpret_cast<vtkSliderRepresentation *>(
                        sliderWidget->GetRepresentation())->SetValue((double)val);

    this->displayData->ShallowCopy(activeReader->GetOutput());

    nextFilter->Update();

    printf("Switching to timestep: %d\n", val);
  }

  vtkPolyData *displayData;
  std::map<int, vtkXMLPolyDataReader *> *readers;
};
} // namespace

// Some program constants
const std::string background("#111111");

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage is %s DATA_FOLDER_PATH\n", argv[0]);
    return 0;
  }
  std::string data_folder_path;
  data_folder_path = argv[1];

  std::map<int, fs::__cxx11::path> files =
      load_cosmology_dataset(data_folder_path);

  printf("Loaded %lu files.\n", files.size());

  std::map<int, vtkXMLPolyDataReader *> dataset_readers;
  for (auto path : files) {
    vtkXMLPolyDataReader *reader = vtkXMLPolyDataReader::New();
    int index = path.first;
    reader->SetFileName(path.second.c_str());

    dataset_readers.insert_or_assign(path.first, reader);
  }
  printf("Finished creating data loaders\n");

  // Load cluster assignments
  std::map<int, int> clusters;
  vtkXMLPolyDataReader *clusterReader = vtkXMLPolyDataReader::New();
  std::string path = data_folder_path.append("/clusters.vtp");
  if (!std::filesystem::exists(path)) {
    printf("The clusters file is missing at %s. Please read the README.md on how to generate it.\n", path.c_str());
    return 0;
  }
  clusterReader->SetFileName(path.c_str());
  clusterReader->Update();

  vtkIdType num = clusterReader->GetNumberOfPoints();

  vtkPolyData *data = clusterReader->GetOutput();
  vtkTypeInt64Array *ids = static_cast<vtkTypeInt64Array*>(data->GetPointData()->GetArray("id"));
  vtkTypeInt64Array *carr = static_cast<vtkTypeInt64Array*>(data->GetPointData()->GetArray("cluster_id"));
 
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
      printf("Missing point in cluster ass: %ld\n", i);
    }
  }
  printf("Finished reading %d clusters\n", num);

  vtkXMLPolyDataReader *activeReader;
  vtkNew<vtkPolyData> displaySourcePolyData;
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkPointSource> ptSource;
  vtkNew<vtkPolyDataMapper> dataMapper;
  vtkNew<vtkLookupTable> tempLUT = GetTemperatureLUT();
  vtkNew<vtkLookupTable> clusterLUT = GetClusterLUT();

  // active timestep
  int active = 624;

  // Set the background color.
  colors->SetColor("BkgColor", background);

  // Set the active reader and get its output to be the polydata
  activeReader = dataset_readers.at(active);
  activeReader->Update();
  displaySourcePolyData->ShallowCopy(activeReader->GetOutput());

  // This filter calculates the temperature
  vtkNew<vtkProgrammableFilter> temperatureFilter;
  temperatureFilter->SetInputData(displaySourcePolyData);

  tempFilterParams temperatureFilterParams;
  temperatureFilterParams.data = displaySourcePolyData;
  temperatureFilterParams.filter = temperatureFilter;
  temperatureFilterParams.mapper = dataMapper;
  temperatureFilterParams.updateScalarRange = false;

  temperatureFilter->SetExecuteMethod(CalculateTemperature,
                                      &temperatureFilterParams);
  temperatureFilter->Update();

  // This filter adds a column with the cluster index
  vtkNew<vtkProgrammableFilter> clusterFilter;
  clusterFilter->SetInputData(temperatureFilter->GetOutput());

  ac_params clusterFilterParams;
  clusterFilterParams.data = static_cast<vtkPolyData*>(temperatureFilter->GetOutput());
  clusterFilterParams.filter = clusterFilter;
  clusterFilterParams.clustering = &clusters;

  clusterFilter->SetExecuteMethod(AssignCluster, &clusterFilterParams);
  clusterFilter->Update();

  vtkNew<vtkGlyph3D> glyph3D;
  glyph3D->SetSourceConnection(ptSource->GetOutputPort());
  glyph3D->SetInputConnection(clusterFilter->GetOutputPort());
  glyph3D->Update();

  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes are
  // defined.
  dataMapper->SetLookupTable(clusterLUT);
  dataMapper->ScalarVisibilityOn();
  dataMapper->SelectColorArray("Cluster");
  dataMapper->SetScalarModeToUsePointFieldData();
  dataMapper->SetInputConnection(glyph3D->GetOutputPort());

  // The actor is a grouping mechanism: besides the geometry (mapper), it
  // also has a property, transformation matrix, and/or texture map.
  vtkNew<vtkActor> actor;
  actor->SetMapper(dataMapper);

  // The renderer generates the image
  // which is then displayed on the render window.
  // It can be thought of as a scene to which the actor is added
  vtkNew<vtkRenderer> renderer;

  // The render window is the actual GUI window
  // that appears on the computer screen
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("VisCos");

  // The render window interactor captures mouse events
  // and will perform appropriate camera or actor manipulation
  // depending on the nature of the events.
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderWindowInteractor->Initialize();

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

  // Scalar bar for the particle colors when showing the temperature
  vtkNew<vtkScalarBarActor> scalarBar;
  scalarBar->SetOrientationToHorizontal();
  scalarBar->SetLookupTable(tempLUT);
  scalarBar->SetPosition2(0.2, 1.5);
  scalarBar->SetPosition(1, 1.5);
  scalarBar->SetWidth(2);
  scalarBar->Modified();

  // create the scalarBarWidget
  vtkNew<vtkScalarBarWidget> scalarBarWidget;
  scalarBarWidget->SetInteractor(renderWindowInteractor);
  scalarBarWidget->SetScalarBarActor(scalarBar);
  // scalarBarWidget->On();

  // Create the callback; I do not know how to pass displayData and readers via
  // a constructor
  vtkNew<vtkSliderCallback> callback;
  callback->nextFilter = temperatureFilter;

  callback.GetPointer()->displayData = displaySourcePolyData.GetPointer();
  callback.GetPointer()->readers = &dataset_readers;
  
  // This is the camera
  vtkNew<vtkCamera> camera;
  float scale = 50;
  camera->SetPosition(scale * 3.24, scale * 2.65, scale * 4.09);
  camera->SetFocalPoint(scale * 0.51, scale * 0.71, scale * 0.78);
  camera->SetFocalDisk(1.0);
  camera->SetEyeAngle(30);
  camera->SetFocalDistance(1.0);
  camera->SetViewUp(-0.27, 0.91, -0.31);

  vtkNew<KeyPressInteractorStyle> style;
  style->camera = camera;
  style->renderWindow = renderWindow;
  style->dataMapper = dataMapper;
  style->actor = actor;
  style->tempLUT = tempLUT;
  style->clusterLUT = clusterLUT;
  style->tempScalarBarWidget = scalarBarWidget;
  style->tempScalarBarActor = scalarBar;
  style->tempFilterParameters = &temperatureFilterParams;
  
  renderWindowInteractor->SetInteractorStyle(style);
  style->EnabledOn();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->SetActiveCamera(camera);
  // This starts the event loop and as a side effect causes an initial render.
  renderWindow->Render();

  vtkNew<vtkSliderRepresentation2D> sliderRep;
  sliderRep->SetMinimumValue(1.0);
  sliderRep->SetMaximumValue(625);
  sliderRep->SetValue((double)active);
  sliderRep->SetTitleText("Timestep");
  sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
  sliderRep->GetPoint1Coordinate()->SetValue(40, 80);
  sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
  sliderRep->GetPoint2Coordinate()->SetValue(320, 80);

  vtkNew<vtkSliderWidget> sliderWidget;
  sliderWidget->SetInteractor(renderWindowInteractor);
  sliderWidget->SetRepresentation(sliderRep);
  sliderWidget->SetNumberOfAnimationSteps(10);
  sliderWidget->SetAnimationModeToAnimate();
  sliderWidget->On();

  // Register callback
  sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

  // Switch to 'active' timestep
  // sliderWidget.GetPointer()->InvokeEvent(vtkCommand::InteractionEvent);

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
