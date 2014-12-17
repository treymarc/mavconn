/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit
Please see our website at <http://pixhawk.ethz.ch>

(c) 2009 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Definition of the class PxCameraCalibrationStandard.
 *
 *   @author Petri Tanskanen <mavteam@student.ethz.ch>
 *   @author David de Bos <mavteam@student.ethz.ch>
 *
 */

/** @addtogroup camera_calibration */
/*@{*/


#ifndef _PX_CAMERA_CALIBRATION_STANDARD_H_
#define _PX_CAMERA_CALIBRATION_STANDARD_H_

#include <opencv2/core/core.hpp>

#include "PxCameraCalibration.h"

class PxCameraStereoCalibration;

/**
 * @brief The calibration data wrapper classes for standard cameras.
 *
 * PxCameraCalibrationStandard loads calibration data for standard cameras and offers functions to undistort images and points.
 *
 */

class PxCameraCalibrationStandard : public PxCameraCalibration
{
public:
	PxCameraCalibrationStandard(const char *filename);
	virtual ~PxCameraCalibrationStandard(){};
	/** @brief Creates an undistortion mapping for cvRemap. */
	virtual void initUndistortMap(cv::Mat &rMapX, cv::Mat &rMapY) const;

	/** @brief Undistorts one or more points. */
	virtual void undistortPoints(const CvPoint2D32f *pSrc, CvPoint2D32f *pDest, int count) const;

public:
	/** @brief Distorts one or more points. */
	void distortPoints(const CvPoint2D32f* pSrc, CvPoint2D32f* pDest, int count) const;

	const std::vector<float>& focalLength(void) const;
	const std::vector<float>& principalPoint(void) const;
	const std::vector<float>& distortionCoeffs(void) const;
	const cv::Mat& distortion(void) const;

protected:
	std::vector<float> m_focal;								///< The focal length
	std::vector<float> m_cc; 									///< The principal point location
	std::vector<float> m_kc;									///< Distortion coefficients
	cv::Mat m_distortion;								///< CvMat structure for distortion
};

#endif //_PX_CAMERA_CALIBRATION_STANDARD_H_

/*@}*/
