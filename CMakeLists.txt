cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(PROJECT_NAME VisCos)

project(${PROJECT_NAME})

add_compile_options(-O3 -ffunction-sections -flto)


find_package(VTK COMPONENTS
  CommonColor
  CommonCore
  CommonDataModel
  FiltersProgrammable
  IOXML
  InteractionStyle
  RenderingAnnotation
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  FiltersModeling
  FiltersSources
  InteractionStyle
  InteractionWidgets
)

add_executable(${PROJECT_NAME}
  ./src/main.cxx
  ./src/processing/CalculateTemperatureFilter.cxx
  ./src/processing/AssignClusterFilter.cxx
  ./src/interactive/KeyPressEvents.cxx
  ./src/data/Loader.cxx)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
vtk_module_autoinit(
  TARGETS VisCos
  MODULES ${VTK_LIBRARIES})
