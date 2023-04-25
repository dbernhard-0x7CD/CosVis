#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h> // for vtkTypeMacro

#include "../processing/CalculateTemperatureFilter.hxx"
class vtkCamera;

// Define interaction style
class KeyPressInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
  static KeyPressInteractorStyle *New();

  vtkLookupTable* tempLUT;
  vtkLookupTable* clusterLUT;

  tempFilterParams* tempFilterParameters;
  vtkScalarBarWidget* tempScalarBarWidget;
  vtkScalarBarActor* tempScalarBarActor;

  vtkActor* actor;
  vtkRenderWindow *renderWindow;
  vtkPolyDataMapper* dataMapper;

  vtkTypeMacro(KeyPressInteractorStyle, vtkInteractorStyleTrackballCamera);
  vtkCamera *camera;
  virtual void OnKeyPress() override;
};
