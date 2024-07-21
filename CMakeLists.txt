cmake_minimum_required(VERSION 3.29)

project(Lumiax)

add_executable(Lumiax)

target_sources(Lumiax PRIVATE
  src/debugRenderer.cpp
  src/level.cpp
  src/levelParser.cpp
  src/levelRenderer.cpp
  src/main.cpp
  src/ship.cpp
  src/shipRenderer.cpp
)

set_target_properties(Lumiax PROPERTIES
  CXX_STANDARD 23
  CXX_EXTENSIONS OFF
)

if(MSVC)
  target_compile_options(Lumiax PRIVATE /W4 /WX)
else()
  target_compile_options(Lumiax PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(Lumiax PRIVATE -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
  target_link_options(Lumiax PRIVATE -fsanitize=address -fsanitize=undefined)
endif()

set(SFML_ENABLE_SANITIZERS ON)
add_subdirectory(./external/sfml/)
target_link_libraries(Lumiax PUBLIC sfml-graphics)

option(BOX2D_BUILD_UNIT_TESTS "" OFF)
option(BOX2D_BUILD_DOCS "" OFF)
option(BOX2D_USER_SETTINGS "" OFF)
option(BOX2D_BUILD_TESTBED "Build the Box2D testbed" OFF)
add_subdirectory(./external/box2d)
target_link_libraries(Lumiax PUBLIC box2d)

add_library(
  imgui
  external/imgui/imgui.cpp
  external/imgui/imgui_demo.cpp
  external/imgui/imgui_draw.cpp
  external/imgui/imgui_tables.cpp
  external/imgui/imgui_widgets.cpp
  external/imgui-sfml/imgui-SFML.cpp
)
target_include_directories(imgui PUBLIC
  external/imgui
  external/imgui-sfml
)
target_link_libraries(imgui PUBLIC sfml-graphics)
target_link_libraries(Lumiax PUBLIC imgui)

#json
target_include_directories(Lumiax PUBLIC ./external/json/include/)

