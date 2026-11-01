cmake_minimum_required(VERSION 3.16)
project(ProGambler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options(/utf-8 /W4)
endif()

add_executable(ProGambler
  main.cpp
  app.cpp
  bridge.cpp
)

target_include_directories(ProGambler PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_definitions(ProGambler PRIVATE WEBVIEW_STATIC)

if(WIN32)
  target_link_libraries(ProGambler PRIVATE ole32 shell32 shlwapi user32 version)
endif()

add_custom_command(TARGET ProGambler POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/main.html
    $<TARGET_FILE_DIR:ProGambler>/main.html
  COMMENT "Copy main.html next to executable"
)
