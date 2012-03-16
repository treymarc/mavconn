IF(GLIBMM2_FOUND)
IF(SIGC++_FOUND)
IF(RTI_FOUND)
INCLUDE_DIRECTORIES(
  ${GLIB2_MAIN_INCLUDE_DIR}
  ${GLIB2_INTERNAL_INCLUDE_DIR}
  ${GLIBMM2_MAIN_INCLUDE_DIR}
  ${GLIBMM2_INTERNAL_INCLUDE_DIR}
  ${SIGC++_INCLUDE_DIR}
  ${RTI_INCLUDE_DIR}
  ${RTI_INCLUDE_DIR}/ndds
  ${ZLIB_INCLUDE_DIRS}
)

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]|x86_64")
    IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
      # Linux 32-bit
      SET(CMAKE_CXX_FLAGS "-DRTI_UNIX -DRTI_LINUX -DRTI_32BIT -m32")
    ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
      # Linux 64-bit
      SET(CMAKE_CXX_FLAGS "-DRTI_UNIX -DRTI_LINUX -DRTI_64BIT -m64")
    ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]|x86_64")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

SET_SOURCE_FILES(DDS_CORE_SRC_FILES
  DDSTopicManager.cc
  TopicManagerFactory.cc
  Middleware.cc
)

SET_SOURCE_FILES(DDS_INTERFACE_SRC_FILES
  interface/image/dds_image_message_t.cxx
  interface/image/dds_image_message_tPlugin.cxx
  interface/image/dds_image_message_tSupport.cxx
  interface/image/image_interface.cc
  interface/mavlink/dds_mavlink_message_t.cxx
  interface/mavlink/dds_mavlink_message_tPlugin.cxx
  interface/mavlink/dds_mavlink_message_tSupport.cxx
  interface/mavlink/mavlink_interface.cc
  interface/perception/dds_obstacle_map_message_t.cxx
  interface/perception/dds_obstacle_map_message_tPlugin.cxx
  interface/perception/dds_obstacle_map_message_tSupport.cxx
  interface/perception/perception_interface.cc
  interface/rgbd_image/dds_rgbd_image_message_t.cxx
  interface/rgbd_image/dds_rgbd_image_message_tPlugin.cxx
  interface/rgbd_image/dds_rgbd_image_message_tSupport.cxx
  interface/rgbd_image/rgbd_image_interface.cc
)

PIXHAWK_LIBRARY(mavconn_dds SHARED ${DDS_CORE_SRC_FILES} ${DDS_INTERFACE_SRC_FILES})
PIXHAWK_LINK_LIBRARIES(mavconn_dds
  dl
  nsl
  pthread
  rt
  ${GLIB2_LIBRARY}
  ${GLIBMM2_LIBRARY}
  ${GTHREAD2_LIBRARY}
  ${RTI_LIBRARIES}
  ${SIGC++_LIBRARY}
  ${ZLIB_LIBRARIES}
)
ENDIF(RTI_FOUND)
ENDIF(SIGC++_FOUND)
ENDIF(GLIBMM2_FOUND)
