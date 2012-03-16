INCLUDE(FindPackageHandleStandardArgs)
INCLUDE(HandleLibraryTypes)

SET(VICON_LibrarySearchPaths
  /usr/lib/
  /usr/local/lib/
  /opt/local/lib/
)

FIND_LIBRARY(VICON_LIBRARY_OPTIMIZED
  NAMES ViconDataStreamSDK_CPP
  PATHS ${VICON_LibrarySearchPaths}
)

# Handle the REQUIRED argument and set the <UPPERCASED_NAME>_FOUND variable
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VICON "Could NOT find VICON library"
  VICON_LIBRARY_OPTIMIZED
)

# Collect optimized and debug libraries
HANDLE_LIBRARY_TYPES(VICON)

MARK_AS_ADVANCED(
  VICON_LIBRARY_OPTIMIZED
)
