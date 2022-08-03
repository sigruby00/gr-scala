#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "gnuradio::gnuradio-scala" for configuration "Release"
set_property(TARGET gnuradio::gnuradio-scala APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(gnuradio::gnuradio-scala PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libgnuradio-scala.so.b110ae02"
  IMPORTED_SONAME_RELEASE "libgnuradio-scala.so.1.0.0git"
  )

list(APPEND _IMPORT_CHECK_TARGETS gnuradio::gnuradio-scala )
list(APPEND _IMPORT_CHECK_FILES_FOR_gnuradio::gnuradio-scala "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libgnuradio-scala.so.b110ae02" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
