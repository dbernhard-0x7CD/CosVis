#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointSource.h>
#include <vtkGlyph2D.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackball.h>
#include <vtkRenderer.h>

#include <array>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkPolyData> input1;

  // Set the background color.
  std::array<unsigned char, 4> bkg{{26, 51, 102, 255}};
  colors->SetColor("BkgColor", bkg.data());

  // This creates a polygonal cylinder model with eight circumferential facets
  // (i.e, in practice an octagonal prism).
  vtkNew<vtkCylinderSource> cylinder;
  cylinder->SetResolution(8);

  vtkNew<vtkPointSource> ptSource;

  vtkNew<vtkXMLPolyDataReader> reader1;
  reader1->SetFileName("../data/Full.cosmo.000.vtp");
  reader1->Update();

  input1->ShallowCopy(reader1->GetOutput());

  vtkNew<vtkGlyph3D> glyph3D;
  glyph3D->SetSourceConnection(ptSource->GetOutputPort());
  glyph3D->SetInputData(input1);
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
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

  // Zoom in a little by accessing the camera and invoking its "Zoom" method.
  // renderer->ResetCamera();
  // renderer->GetActiveCamera()->Zoom(1.5);

  // The render window is the actual GUI window
  // that appears on the computer screen
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(300, 300);
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Cylinder");

  // The render window interactor captures mouse events
  // and will perform appropriate camera or actor manipulation
  // depending on the nature of the events.
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;

  // Have trackball style interaction by default (is like pressing T at the beginning)
  vtkNew<vtkInteractorStyleSwitch> style;
  style->SetCurrentStyleToTrackballActor();

  renderWindowInteractor->SetInteractorStyle(style);
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // This starts the event loop and as a side effect causes an initial render.
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
