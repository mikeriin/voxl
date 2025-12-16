#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "enkiTS::enkiTS" for configuration "Release"
set_property(TARGET enkiTS::enkiTS APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(enkiTS::enkiTS PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/enkiTS.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/enkiTS.dll"
  )

list(APPEND _cmake_import_check_targets enkiTS::enkiTS )
list(APPEND _cmake_import_check_files_for_enkiTS::enkiTS "${_IMPORT_PREFIX}/lib/enkiTS.lib" "${_IMPORT_PREFIX}/bin/enkiTS.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
