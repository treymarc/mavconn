##======================================================================
#
# PIXHAWK Micro Air Vehicle Flying Robotics Toolkit
# Please see our website at <http://pixhawk.ethz.ch>
# 
#
# Original Authors:
#   @author Reto Grieder <www.orxonox.net>
#   @author Fabian Landau <www.orxonox.net>
# Contributing Authors (in alphabetical order):
#  
# Todo:
#
#
# (c) 2009 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>
# 
# This file is part of the PIXHAWK project
# 
#     PIXHAWK is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
# 
#     PIXHAWK is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public License
#     along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.
# 
##========================================================================

INCLUDE(CompareVersionStrings)
INCLUDE(FindPackageHandleStandardArgs)

# Prevent CMake from finding libraries in the installation folder on Windows.
# There might already be an installation from another compiler
IF(DEPENDENCY_PACKAGE_ENABLE)
  LIST(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH  "${CMAKE_INSTALL_PREFIX}")
  LIST(REMOVE_ITEM CMAKE_SYSTEM_LIBRARY_PATH "${CMAKE_INSTALL_PREFIX}/bin")
ENDIF()

############### Library finding #################
# Performs the search and sets the variables    #

FIND_PACKAGE(MAVLINK   REQUIRED)
FIND_PACKAGE(OpenCV    REQUIRED)
FIND_PACKAGE(ZLIB      REQUIRED)
FIND_PACKAGE(GSL       REQUIRED)
FIND_PACKAGE(DC1394)
FIND_PACKAGE(GLIB2     REQUIRED)
FIND_PACKAGE(GTHREAD2  REQUIRED)
FIND_PACKAGE(Threads   REQUIRED)
FIND_PACKAGE(LCM       REQUIRED)
FIND_PACKAGE(GPS)
FIND_PACKAGE(GLIBTOP)
FIND_PACKAGE(GLIBMM2)
FIND_PACKAGE(SIGC++)
FIND_PACKAGE(JPEG_TURBO)
FIND_PACKAGE(VICON)

FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem program_options system)

