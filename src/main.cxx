#include <filesystem>
#include <iostream>
#include <map> // for map, operator!=
#include <stdio.h> // for printf
#include <stdlib.h>
#include <string>
#include <utility> // for pair

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
#include "interactive/KeyPressEvents.hxx"

namespace fs = std::filesystem;

namespace {

class vtkSliderCallback : public vtkCommand {
public:
  vtkSliderCallback(){};

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

    printf("Switching to timestep: %d\n", val);
  }

  vtkPolyData *displayData;
  std::map<int, vtkXMLPolyDataReader *> *readers;
};
} // namespace

// Some program constants
const std::string background("#2a2e32");

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
    // reader->Update();

    dataset_readers.insert_or_assign(path.first, reader);
  }
  printf("Finished creating data loaders\n");

  vtkXMLPolyDataReader *activeReader;
  vtkNew<vtkPolyData> displaySourcePolyData;
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkPointSource> ptSource;
  vtkNew<vtkPolyDataMapper> dataMapper;

  // active timestep
  int active = 0;

  // Set the background color.
  colors->SetColor("BkgColor", background);

  // LUT for coloring the particles
  vtkNew<vtkLookupTable> lut;

  lut->SetHueRange(0.667, 0.0);
  lut->SetAlphaRange(0.2, 0.7);
  // lut->SetTableRange(1.1, 11.4);
  lut->SetNumberOfColors(256);
  // TODO: we should probably move to a logarithm scale
  // lut->SetScaleToLog10();
  lut->Build();

  // Set the active reader and get its output to be the polydata
  activeReader = dataset_readers.at(active);
  activeReader->Update();
  displaySourcePolyData->ShallowCopy(activeReader->GetOutput());

  // This filter calculates the temperature
  vtkNew<vtkProgrammableFilter> temperatureFilter;
  temperatureFilter->SetInputData(displaySourcePolyData);

  params temperatureFilterParams;
  temperatureFilterParams.data = displaySourcePolyData;
  temperatureFilterParams.filter = temperatureFilter;
  temperatureFilterParams.mapper = dataMapper;
  temperatureFilterParams.updateScalarRange = true;

  temperatureFilter->SetExecuteMethod(CalculateTemperature,
                                      &temperatureFilterParams);
  temperatureFilter->Update();

  vtkNew<vtkGlyph3D> glyph3D;
  glyph3D->SetSourceConnection(ptSource->GetOutputPort());
  glyph3D->SetInputConnection(temperatureFilter->GetOutputPort());
  glyph3D->Update();

  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes are
  // defined.
  dataMapper->SetLookupTable(lut);
  dataMapper->ScalarVisibilityOn();
  dataMapper->SelectColorArray("Temperature");
  dataMapper->SetScalarModeToUsePointFieldData();
  dataMapper->InterpolateScalarsBeforeMappingOn();
  dataMapper->SetInputConnection(glyph3D->GetOutputPort());

  // The actor is a grouping mechanism: besides the geometry (mapper), it
  // also has a property, transformation matrix, and/or texture map.
  // Here we set its color and rotate it around the X and Y axes.
  vtkNew<vtkActor> actor;
  actor->SetMapper(dataMapper);
  actor->GetProperty()->SetOpacity(0.2);
  actor->SetDragable(1);

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

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());


  // Scalar bar for the particle colors
  vtkNew<vtkScalarBarActor> scalarBar;
  scalarBar->SetOrientationToHorizontal();
  scalarBar->SetLookupTable(lut);
  scalarBar->SetPosition2(0.2, 1.5);
  scalarBar->SetPosition(1, 1.5);
  scalarBar->SetWidth(2);

  // create the scalarBarWidget
  vtkNew<vtkScalarBarWidget> scalarBarWidget;
  scalarBarWidget->SetInteractor(renderWindowInteractor);
  scalarBarWidget->SetScalarBarActor(scalarBar);
  scalarBarWidget->On();

  // Create the callback; I do not know how to pass displayData and readers via
  // a constructor
  vtkNew<vtkSliderCallback> callback;
  callback.GetPointer()->displayData = displaySourcePolyData.GetPointer();
  callback.GetPointer()->readers = &dataset_readers;
  
  // This is the camera
  vtkNew<vtkCamera> camera;
  float scale = 50;
  camera->SetPosition(scale * 3.24, scale * 2.65, scale * 4.09);
  camera->SetFocalPoint(scale * 0.51, scale * 0.71, scale * 0.78);
  camera->SetFocalDisk(1.0);
  camera->SetEyeAngle(2);
  camera->SetEyeAngle(30);
  camera->SetFocalDistance(0.0);
  camera->SetViewUp(-0.27, 0.91, -0.31);

  vtkNew<KeyPressInteractorStyle> style;
  style->camera = camera;
  style->renderWindow = renderWindow;
  
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
  sliderRep->DragableOn();
  
  // sliderRep->SetTubeWidth(0.05);
  sliderRep->SetPlaceFactor(2.0);

  sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
  sliderRep->GetPoint1Coordinate()->SetValue(40, 80);
  sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
  sliderRep->GetPoint2Coordinate()->SetValue(300, 80);

  vtkNew<vtkSliderWidget> sliderWidget;
  sliderWidget->SetInteractor(renderWindowInteractor);
  sliderWidget->SetRepresentation(sliderRep);
  sliderWidget->On();
  sliderWidget->SetAnimationModeToAnimate();

  // Register callback
  sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);


  sliderWidget->On();
  // sliderWidget.GetPointer()->InvokeEvent(vtkCommand::InteractionEvent);

  renderWindowInteractor->Initialize();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
