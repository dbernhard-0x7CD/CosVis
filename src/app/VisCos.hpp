#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkGlyph3D.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProgrammableFilter.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkXMLPolyDataReader.h>

#include "../helper/helper.hxx"
#include "../interactive/KeyPressInteractorStyle.hxx"
#include "../interactive/TimeSliderCallback.hxx"
#include "../interactive/ResizeWindowCallback.hxx"
#include "../processing/AssignClusterFilter.hxx"
#include "../processing/ParticleTypeFilter.hxx"
#include "../processing/CalculateTemperatureFilter.hxx"

class vtkCamera;
class KeyPressInteractorStyle;

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

  // The mapper is responsible for pushing the geometry into the graphics
  // library. It may also do color mapping, if scalars or other attributes are
  // defined.
  vtkNew<vtkPolyDataMapper> dataMapper;
  vtkNew<vtkPolyDataMapper> starDataMapper;

  vtkNew<vtkPolyData> importantPointsData;
  vtkNew<vtkPolyData> baryonData;
  vtkNew<vtkPolyData> starData;

  vtkNew<vtkLookupTable> tempLUT = GetTemperatureLUT();
  vtkNew<vtkLookupTable> clusterLUT = GetClusterLUT();
  vtkNew<vtkLookupTable> phiLUT = GetPhiLUT();

  vtkNew<vtkRenderer> renderer;

  // Actors
  vtkNew<vtkActor> manyParticlesActor;
  vtkNew<vtkActor> starParticlesActor;

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

  particleTypeFilterParams particleFilterParams;
  vtkNew<vtkProgrammableFilter> particleTypeFilter;

  // Various
  vtkNew<vtkGlyph3D> glyph3D;
  vtkNew<vtkGlyph3D> starGlyph3D;
  vtkNew<vtkCamera> camera;
  vtkNew<KeyPressInteractorStyle> keyboardInteractorStyle;

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

  void UpdateGUIElements();

  void Run();
};
