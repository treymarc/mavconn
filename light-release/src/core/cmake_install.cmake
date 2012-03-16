# Install script for directory: /home/pixhawk/pixhawk/mavconn/src/core

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/bin/mavconn-sysctrl" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/mavconn-sysctrl")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/local/bin/mavconn-sysctrl"
         RPATH "/usr/local/lib:/usr/local/lib")
  ENDIF()
  FILE(INSTALL DESTINATION "/usr/local/bin" TYPE EXECUTABLE FILES "/home/pixhawk/pixhawk/mavconn/light-release/bin/mavconn-sysctrl")
  IF(EXISTS "$ENV{DESTDIR}/usr/local/bin/mavconn-sysctrl" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/bin/mavconn-sysctrl")
    FILE(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/local/bin/mavconn-sysctrl"
         OLD_RPATH "/home/pixhawk/pixhawk/mavconn/light-release/lib:/usr/local/lib:"
         NEW_RPATH "/usr/local/lib:/usr/local/lib")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/bin/mavconn-sysctrl")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "/usr/local/include/mavconn" TYPE FILE FILES
    "/home/pixhawk/pixhawk/mavconn/src/mavconn.h"
    "/home/pixhawk/pixhawk/mavconn/src/mavconn.hpp"
    "/home/pixhawk/pixhawk/mavconn/light-release/src/MAVCONNConfig.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "/usr/local/include/mavconn/comm/lcm" TYPE FILE FILES
    "/home/pixhawk/pixhawk/mavconn/src/comm/lcm/mavconn_mavlink_message_t.h"
    "/home/pixhawk/pixhawk/mavconn/src/comm/lcm/mavconn_mavlink_msg_container_t.h"
    "/home/pixhawk/pixhawk/mavconn/src/comm/lcm/camera_image_message_t.h"
    "/home/pixhawk/pixhawk/mavconn/src/comm/lcm/rgbd_camera_image_message_t.h"
    "/home/pixhawk/pixhawk/mavconn/src/comm/lcm/virtual_scan_message_t.h"
    "/home/pixhawk/pixhawk/mavconn/src/comm/lcm/mavlink_message_t.hpp"
    "/home/pixhawk/pixhawk/mavconn/src/comm/lcm/mavlink_msg_container_t.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "/usr/local/include/mavconn/interface/shared_mem" TYPE FILE FILES
    "/home/pixhawk/pixhawk/mavconn/src/interface/shared_mem/PxSharedMemClient.h"
    "/home/pixhawk/pixhawk/mavconn/src/interface/shared_mem/PxSharedMemServer.h"
    "/home/pixhawk/pixhawk/mavconn/src/interface/shared_mem/PxSHM.h"
    "/home/pixhawk/pixhawk/mavconn/src/interface/shared_mem/PxSHMImageServer.h"
    "/home/pixhawk/pixhawk/mavconn/src/interface/shared_mem/PxSHMImageClient.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "/usr/local/include/mavconn/core" TYPE FILE FILES
    "/home/pixhawk/pixhawk/mavconn/src/core/MAVConnParamClient.h"
    "/home/pixhawk/pixhawk/mavconn/src/core/ParamClientCallbacks.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "/usr/local/include/mavconn/core/timer" TYPE FILE FILES
    "/home/pixhawk/pixhawk/mavconn/src/core/timer/OgreTimer.h"
    "/home/pixhawk/pixhawk/mavconn/src/core/timer/OgreTimerImp.GLX.h"
    "/home/pixhawk/pixhawk/mavconn/src/core/timer/Timer.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "/usr/local/lib" TYPE FILE FILES
    "/home/pixhawk/pixhawk/mavconn/light-release/lib/libmvBlueFOX.so"
    "/home/pixhawk/pixhawk/mavconn/light-release/lib/libmvBlueFOX.so.1.12.40"
    "/home/pixhawk/pixhawk/mavconn/light-release/lib/libmvDeviceManager.so"
    "/home/pixhawk/pixhawk/mavconn/light-release/lib/libmvDeviceManager.so.1.12.40"
    "/home/pixhawk/pixhawk/mavconn/light-release/lib/libmvPropHandling.so"
    "/home/pixhawk/pixhawk/mavconn/light-release/lib/libmvPropHandling.so.1.12.40"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

