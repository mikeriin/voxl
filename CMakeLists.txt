cmake_minimum_required(VERSION 3.20)

project(voxl CXX)

# nécessaire pour clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# standard c++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# trouver les librairies via vcpkg
find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(enkiTS CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# récupérer tous les fichier .cpp du projet
file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# créer l'exécutable
add_executable(voxl 
  ${PROJECT_SOURCES}
  main.cpp
)

# permet de faire #include <header.h>
target_include_directories(voxl PRIVATE "${CMAKE_SOURCE_DIR}/include")

# lier les librairies
target_link_libraries(voxl PRIVATE 
  SDL3::SDL3
  glad::glad
  glm::glm
  EnTT::EnTT
  enkiTS::enkiTS
  nlohmann_json::nlohmann_json
  OpenGL::GL
)

# copier 'assets' vers le dossier de l'exécutable après la compilation
add_custom_command(TARGET voxl POST_BUILD
  # supprimer le dossier assets destination (nettoyage)
  COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:voxl>/assets"
  
  # copier le nouveau dossier assets à jour
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:voxl>/assets"
)