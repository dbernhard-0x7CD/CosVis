#include <array>
#include <filesystem>
#include <iostream>
#include <list>
#include <regex>
#include <string>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkGlyph2D.h>
#include <vtkInteractorStyleTrackball.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <vtkPolyDataReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vtkSliderRepresentation3D.h>
#include <vtkSliderWidget.h>

#include "data/Loader.h"

namespace fs = std::filesystem;

namespace {
// The callback does the work.
// The callback keeps a pointer to the sphere whose resolution is
// controlled. After constructing the callback, the program sets the
// SphereSource of the callback to
// the object to be controlled.
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

  virtual void Execute(vtkObject *caller, unsigned long, void*) {
    vtkSliderWidget *sliderWidget = reinterpret_cast<vtkSliderWidget *>(caller);
    double dvalue = reinterpret_cast<vtkSliderRepresentation *>(
                        sliderWidget->GetRepresentation())
                        ->GetValue();

    // We only have readers for even timesteps
    int val = (((int)dvalue) / 2) * 2;

    vtkXMLPolyDataReader *activeReader;
    activeReader = this->readers->at(val);
    activeReader->Update();

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

  printf("Loaded %d files.\n", files.size());

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

  int active = 8;

  // Set the background color.
  colors->SetColor("BkgColor", background);

  // Set the active reader and get its output to be the polydata
  activeReader = dataset_readers.at(active);
  activeReader->Update();
  displaySourcePolyData->ShallowCopy(activeReader->GetOutput());

  vtkNew<vtkGlyph3D> glyph3D;
  glyph3D->SetSourceConnection(ptSource->GetOutputPort());
  glyph3D->SetInputData(displaySourcePolyData);
  glyph3D->Update();

  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes are
  // defined.
  vtkNew<vtkPolyDataMapper> dataMapper;
  dataMapper->SetInputConnection(glyph3D->GetOutputPort());

  // The actor is a grouping mechanism: besides the geometry (mapper), it
  // also has a property, transformation matrix, and/or texture map.
  // Here we set its color and rotate it around the X and Y axes.
  vtkNew<vtkActor> actor;
  actor->SetMapper(dataMapper);
  actor->GetProperty()->SetColor(colors->GetColor4d("white").GetData());
  actor->GetProperty()->SetOpacity(0.5);

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

  // Set actor to Trackball (like in paraview)
  vtkNew<vtkInteractorStyleSwitch> style;
  style->SetCurrentStyleToTrackballActor();

  vtkNew<vtkSliderRepresentation3D> sliderRep;
  sliderRep->SetMinimumValue(1.0);
  sliderRep->SetMaximumValue(313);
  sliderRep->SetValue(0.0);
  sliderRep->SetTitleText("Timestep");
  sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
  sliderRep->GetPoint1Coordinate()->SetValue(40, 50, 1);
  sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
  sliderRep->GetPoint2Coordinate()->SetValue(200, 50, 1);

  vtkNew<vtkSliderWidget> sliderWidget;
  sliderWidget->SetInteractor(renderWindowInteractor);
  sliderWidget->SetRepresentation(sliderRep);
  sliderWidget->SetAnimationModeToAnimate();

  // Create the callback; I do not know how to pass displayData and readers via a constructor
  vtkNew<vtkSliderCallback> callback;
  callback.GetPointer()->displayData = displaySourcePolyData.GetPointer();
  callback.GetPointer()->readers = &dataset_readers;

  sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

  renderWindowInteractor->SetInteractorStyle(style);
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // This starts the event loop and as a side effect causes an initial render.
  renderWindow->Render();

  sliderWidget->EnabledOn();
  sliderWidget.GetPointer()->InvokeEvent(vtkCommand::InteractionEvent);

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
