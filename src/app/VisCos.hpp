#pragma once

#include <string>
#include <map>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkGlyph3D.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProgrammableFilter.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkStructuredGrid.h>
#include <vtkSPHInterpolator.h>
#include <vtkSPHQuinticKernel.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include "../helper/helper.hxx"
#include "../interactive/KeyPressInteractorStyle.hxx"
#include "../interactive/TimeSliderCallback.hxx"
#include "../interactive/ResizeWindowCallback.hxx"
#include "../processing/AssignClusterFilter.hxx"
#include "../processing/ParticleTypeFilter.hxx"
#include "../processing/StarFilter.hxx"
#include "../processing/BaryonFilter.hxx"
#include "../processing/CalculateTemperatureFilter.hxx"
#include "../processing/PolyDataToImageDataAlgorithm.hxx"

class vtkTextActor;
class vtkXMLPolyDataReader;

enum ParticleType { ALL, DARK_MATTER, BARYON };

class VisCos {
private:
  bool loaded = false;
  bool setup = false;

  // The currently active timestep of the data
  int active_timestep;

  // Number of steps to take when the slider was moved
  int steps = 40;

  // Multiplicator for movement and angle operations
  float movementAlpha = 0.8;

  // SPH properties (found with paraview)
  double sphOrigin[3] = { 19.4783, 42.6025, 36.4189 };
  double sphVolumeLengths[3] = { 2, 2, 2 };
  double sphOrientation[3] = { 0, 0, 1.0 };
  // Resolution of the SPH box
  int dimensions[3] = { 140, 140, 140 };

  std::string background_color;
  std::string data_folder_path;
  std::string cluster_path;
  double tempRange[2];
  std::map<int, vtkXMLPolyDataReader *> dataset_readers;

  // Maps point IDs to their cluster ID
  std::map<int, int> clusters;
  vtkXMLPolyDataReader *activeReader;
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkPointSource> singlePointSource;
  vtkNew<vtkSphereSource> sphereSource;

  // For debugging stuff
  vtkNew<vtkPoints> markedPoints; // the 0th point is FP
  vtkNew<vtkPolyData> markedData;

  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes are
  // defined.
  vtkNew<vtkPolyDataMapper> dataMapper;
  vtkNew<vtkPolyDataMapper> markedDataMapper;
  vtkNew<vtkPolyDataMapper> sphDataMapper;
  vtkNew<vtkPolyDataMapper> starDataMapper;

  vtkSmartPointer<vtkLookupTable> tempLUT = GetTemperatureLUT();
  vtkSmartPointer<vtkLookupTable> clusterLUT = GetClusterLUT();
  vtkSmartPointer<vtkLookupTable> phiLUT = GetPhiLUT();

  vtkNew<vtkRenderer> renderer;

  // Actors
  vtkNew<vtkActor> manyParticlesActor;
  vtkNew<vtkActor> sphParticlesActor;
  vtkNew<vtkActor> starParticlesActor;
  vtkNew<vtkActor> markedParticlesActor;

  // Visual stuff
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkNew<vtkScalarBarWidget> scalarBarWidget;
  vtkNew<vtkScalarBarActor> scalarBarActor;
  vtkNew<vtkSliderRepresentation2D> timeSliderRepr;
  vtkNew<vtkSliderWidget> timeSliderWidget;

  // Filters
  TempFilterParams temperatureFilterParams;
  vtkNew<vtkProgrammableFilter> temperatureFilter;

  AssignClusterParams clusterFilterParams;
  vtkNew<vtkProgrammableFilter> clusterFilter;

  ParticleTypeFilterParams particleFilterParams;
  vtkNew<vtkProgrammableFilter> particleTypeFilter;

  StarFilterParams starFilterParams;
  vtkNew<vtkProgrammableFilter> starFilter;

  BaryonFilterParams baryonFilterParams;
  vtkNew<vtkProgrammableFilter> baryonFilter;

  // Various
  vtkNew<vtkGlyph3D> glyph3D;
  vtkNew<vtkGlyph3D> starGlyph3D;
  vtkNew<vtkGlyph3D> markedGlyph3D;
  vtkNew<vtkCamera> camera;
  vtkNew<KeyPressInteractorStyle> keyboardInteractorStyle;

  // Used for SPH
  vtkNew<vtkSPHQuinticKernel> kernel;
  vtkSmartPointer<vtkStructuredGrid> source;
  vtkNew<vtkSPHInterpolator> interpolator;
  vtkNew<vtkVolumeProperty> volumeProperty;
  vtkNew<vtkSmartVolumeMapper> volumeMapper;
  vtkNew<vtkVolume> volume;
  vtkNew<PolyDataToImageDataAlgorithm> polyDataToImageDataAlgorithm;
  vtkNew<vtkImageData> imageData;
  vtkNew<vtkPiecewiseFunction> opacityFunction;

  vtkNew<TimeSliderCallback> timeSliderCallback;
  vtkNew<ResizeWindowCallback> resizeCallback;

  vtkTextActor* textVisibleParticles;
  vtkTextActor* textBaryon;
  vtkTextActor* textDarkMatter;
  vtkTextActor* textBaryonWind;
  vtkTextActor* textBaryonStar;
  vtkTextActor* textBaryonStarForming;
  vtkTextActor* textAGN;


public:
  VisCos(int initial_active_timestep, std::string data_folder_path,
         std::string cluster_path);
  void Load();
  void MoveForward(int steps);
  void MoveBackward(int steps);
  void MoveToTimestep(int step);

  void ShowTemperature();
  void ShowClusters();
  void ShowPhi();

  void SetupPipeline();

  void SetBackgroundColor(std::string color);

  void moreSteps();
  void lessSteps();

  void showAll();
  void hideAll();

  void showBaryon();
  void hideBaryon();
  void toggleBaryon();

  void showDarkMatter();
  void hideDarkMatter();
  void toggleDarkMatter();

  void showBaryonStar();
  void hideBaryonStar();
  void toggleBaryonStar();

  void showBaryonWind();
  void hideBaryonWind();
  void toggleBaryonWind();

  void showBaryonSF();
  void hideBaryonSF();
  void toggleBaryonSF();

  void showDarkAGN();
  void hideDarkAGN();
  void toggleDarkAGN();

  void UpdateVisbleParticlesText();

  float GetMovementAlpha();
  void SetMovementAlpha(float movementAlpha);

  void SetSPHCenter(double pos[3]);
  bool IsSPHOn();
  void EnableSPH();
  void DisableSPH();
  void UpdateSPH();

  void UpdateFP();
  void AddMarkedPoint(double pos[3]);
  void SetSPHOrientation(double *orient);
  double *GetSPHOrientation();

  void UpdateGUIElements();

  void Run();
};
