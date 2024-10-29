project(
  test-stereo_camera
  VERSION 0.1
  LANGUAGES CXX)

find_package(Qt6 COMPONENTS Gui 3DCore 3DRender  REQUIRED)

add_executable(${PROJECT_NAME} tst_stereo_camera.cpp)

target_link_libraries(${PROJECT_NAME} doctest::doctest shared Qt6::Gui Qt6::3DCore Qt6::3DRender)

add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
