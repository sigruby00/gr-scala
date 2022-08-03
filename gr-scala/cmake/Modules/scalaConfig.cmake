INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_SCALA scala)

FIND_PATH(
    SCALA_INCLUDE_DIRS
    NAMES scala/api.h
    HINTS $ENV{SCALA_DIR}/include
        ${PC_SCALA_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SCALA_LIBRARIES
    NAMES gnuradio-scala
    HINTS $ENV{SCALA_DIR}/lib
        ${PC_SCALA_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/scalaTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SCALA DEFAULT_MSG SCALA_LIBRARIES SCALA_INCLUDE_DIRS)
MARK_AS_ADVANCED(SCALA_LIBRARIES SCALA_INCLUDE_DIRS)
