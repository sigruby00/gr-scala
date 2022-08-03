#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "gnuradio::gnuradio-lora" for configuration "Release"
set_property(TARGET gnuradio::gnuradio-lora APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(gnuradio::gnuradio-lora PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libgnuradio-lora.so.ae79d726"
  IMPORTED_SONAME_RELEASE "libgnuradio-lora.so.1.0.0git"
  )

list(APPEND _IMPORT_CHECK_TARGETS gnuradio::gnuradio-lora )
list(APPEND _IMPORT_CHECK_FILES_FOR_gnuradio::gnuradio-lora "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libgnuradio-lora.so.ae79d726" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
