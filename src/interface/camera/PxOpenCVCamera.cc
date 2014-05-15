/*=====================================================================

 PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

 (c) 2011 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

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
 *   @brief Definition of Point Grey Firefly MV interface
 *
 *   @author Pascal Dufour <mavteam@student.ethz.ch>
 *   @author Lionel Heng <hengli@inf.ethz.ch>
 *
 */

#include "PxOpenCVCamera.h"

PxOpenCVCamera::PxOpenCVCamera(cv::VideoCapture* _camera)
 : camera(_camera)
{

}

PxOpenCVCamera::~PxOpenCVCamera()
{

}

bool
PxOpenCVCamera::init(void)
{
	return true;
}

void
PxOpenCVCamera::destroy(void)
{
	if (verbose)
	{
		fprintf(stderr, "# INFO: Reset camera and free resources\n");
	}
	if (camera != NULL)
	{
		camera->release();
		delete camera;
		camera = NULL;
	}
}

bool
PxOpenCVCamera::setConfig(const PxCameraConfig& config, bool master)
{
	if (!setMode(config.getMode()))
	{
		return false;
	}
	if (!setFrameRate(config.getFrameRate()))
	{
		return false;
	}
	if (!setExposureTime(config.getExposureTime()))
	{
		return false;
	}
	if (!setGain(config.getGain()))
	{
		return false;
	}
	if (!setGamma(config.getGamma()))
	{
		return false;
	}
	if (!setBrightness(config.getBrightness()))
	{
		return false;
	}

	return true;
}

bool
PxOpenCVCamera::start(void)
{
	//start transmission
	return true;
}

bool
PxOpenCVCamera::stop(void)
{
	return true;
}

bool
PxOpenCVCamera::grabFrame(cv::Mat& image, uint32_t& skippedFrames,
						   uint32_t& sequenceNum)
{
	bool res = camera->grab();
	if(res)
	{
		res = camera->retrieve(image);
	}
	return res;
}

bool
PxOpenCVCamera::setFrameRate(float frameRate)
{
	
	return camera->set(CV_CAP_PROP_FPS,frameRate);
}

bool
PxOpenCVCamera::setMode(PxCameraConfig::Mode mode)
{
	return true;
}

bool
PxOpenCVCamera::setExposureTime(uint32_t exposureTime)
{

	return true;
}

bool
PxOpenCVCamera::setGain(uint32_t gain)
{

	return true;
}

bool
PxOpenCVCamera::setGamma(uint32_t gamma)
{

	return true;
}

bool
PxOpenCVCamera::setBrightness(uint32_t brightness)
{

	return true;
}


