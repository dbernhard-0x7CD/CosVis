cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(PROJECT_NAME VisCos)

project(${PROJECT_NAME})

find_package(VTK COMPONENTS 
  CommonColor
  CommonCore
  IOXML
  InteractionStyle
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

add_executable(${PROJECT_NAME} MACOSX_BUNDLE src/main.cxx)

target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})
vtk_module_autoinit(
  TARGETS VisCos
  MODULES ${VTK_LIBRARIES})
