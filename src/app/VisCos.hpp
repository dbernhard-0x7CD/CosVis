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
#include "../processing/AssignClusterFilter.hxx"
#include "../processing/CalculateTemperatureFilter.hxx"

class vtkCamera;
class KeyPressInteractorStyle;

enum ParticleType { ALL, DARK_MATTER, BARYON };

class VisCos {
private:
  bool loaded = false;
  bool setup = false;
  int active_timestep;

  // Number of steps to take when the slider was moved
  int steps = 100;

  std::string background_color;
  std::string data_folder_path;
  std::string cluster_path;
  double tempRange[2];
  std::map<int, vtkXMLPolyDataReader *> dataset_readers;
  std::map<int, int> clusters;
  vtkXMLPolyDataReader *activeReader;
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkPointSource> ptSource;
  vtkNew<vtkPolyDataMapper> dataMapper;
  vtkNew<vtkLookupTable> tempLUT = GetTemperatureLUT();
  vtkNew<vtkLookupTable> clusterLUT = GetClusterLUT();
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkActor> actor;
  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  vtkNew<vtkScalarBarWidget> scalarBarWidget;
  vtkNew<vtkScalarBarActor> scalarBarActor;
  vtkNew<vtkSliderRepresentation2D> timeSliderRepr;
  vtkNew<vtkSliderWidget> timeSliderWidget;
  vtkNew<vtkProgrammableFilter> temperatureFilter;
  vtkNew<vtkProgrammableFilter> clusterFilter;
  vtkNew<vtkGlyph3D> glyph3D;
  vtkNew<vtkCamera> camera;
  vtkNew<KeyPressInteractorStyle> keyboardInteractorStyle;

  TempFilterParams temperatureFilterParams;
  AssignClusterParams clusterFilterParams;
  vtkNew<TimeSliderCallback> timeSliderCallback;

public:
  VisCos(int initial_active_timestep, std::string data_folder_path,
         std::string cluster_path);
  void Load();
  void MoveForward(int steps);
  void MoveBackward(int steps);
  void MoveToTimestep(int step);

  void ShowTemperature();
  void ShowClusters();

  void SetupPipeline();

  void SetBackgroundColor(std::string color);

  void moreSteps();
  void lessSteps();

  void Run();
};
