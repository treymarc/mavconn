/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009-2011 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

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
*   @brief Shared memory interface for reading images.
*
*   This interface is a wrapper around PxSHM.
*
*   @author Lionel Heng  <hengli@inf.ethz.ch>
*
*/

#include "SHMImageClient.h"

namespace px
{

SHMImageClient::SHMImageClient()
 : mCam1(SHM::CAMERA_NONE)
 , mCam2(SHM::CAMERA_NONE)
{
	
}

bool
SHMImageClient::init(bool subscribeLatest,
					 SHM::Camera cam1, SHM::Camera cam2)
{
	mSubscribeLatest = subscribeLatest;
	mCam1 = cam1;
	mCam2 = cam2;

	std::string cameras = "";

	if (cam1 == SHM::CAMERA_DOWNWARD_LEFT)
	{
		cameras += " #1: DOWN LEFT";
	}
	if (cam1 == SHM::CAMERA_FORWARD_LEFT)
	{
		cameras += " #1: FORWARD LEFT";
	}
	if (cam2 == SHM::CAMERA_DOWNWARD_RIGHT)
	{
		cameras += " #2: DOWN RIGHT";
	}

	if (cam2 == SHM::CAMERA_FORWARD_RIGHT)
	{
		cameras += " #2: FORWARD RIGHT";
	}

	printf("\t # INFO: Shared mem client initialized for cameras:%s\n", cameras.c_str());

	mData.reserve(1024 * 1024);

	if (!mSHM.init(cam1 | cam2, SHM::CLIENT_TYPE, 128, 1, 1024 * 1024, 9))
	{
		return false;
	}

	return true;
}

uint64_t
SHMImageClient::getTimestamp(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return 0;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.timestamp;
	}
}

uint64_t
SHMImageClient::getValidUntil(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return 0;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.valid_until;
	}
}

uint64_t
SHMImageClient::getCameraID(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return -1;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.cam_id;
	}
}

uint32_t
SHMImageClient::getCameraNo(const mavlink_message_t* msg)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return -1;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);
		return img.cam_no;
	}
}

bool
SHMImageClient::getRollPitch(const mavlink_message_t* msg, float& roll, float& pitch)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		roll = img.roll;
		pitch = img.pitch;

		return true;
	}
}

bool
SHMImageClient::getRollPitchYaw(const mavlink_message_t* msg, float& roll, float& pitch, float& yaw)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		roll = img.roll;
		pitch = img.pitch;
		yaw = img.yaw;

		return true;
	}
}

bool
SHMImageClient::getLocalHeight(const mavlink_message_t* msg, float& height)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		height = img.local_z;

		return true;
	}
}

bool
SHMImageClient::getGPS(const mavlink_message_t* msg, float& lon, float& lat, float& alt)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		lon = img.lon;
		lat = img.lat;
		alt = img.alt;

		return true;
	}
}

bool
SHMImageClient::getGroundTruth(const mavlink_message_t* msg, float& ground_x, float& ground_y, float& ground_z)
{
	// Decode message
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	else
	{
		// Extract the image meta information and pointer location from the image
		mavlink_image_available_t img;
		mavlink_msg_image_available_decode(msg, &img);

		ground_x = img.ground_x;
		ground_y = img.ground_y;
		ground_z = img.ground_z;

		return true;
	}
}

int
SHMImageClient::getCameraConfig(void) const
{
	return (mCam1 | mCam2);
}

bool
SHMImageClient::readMonoImage(const mavlink_message_t* msg, cv::Mat& img, bool verbose)
{
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	
	if (!mSHM.bytesWaiting())
	{
		if (verbose) printf("NO DATA WAITING in MONO IMAGE SHM CLIENT, RETURNING.\n");
		return false;
	}
	
	do
	{
		SHM::CameraType cameraType;
		if (!readCameraType(cameraType))
		{
			if (verbose) printf("\t # ERROR SHM CLIENT: CANNOT READ CAMERA TYPEn");
			return false;
		}

//		if (cameraType != SHM::CAMERA_MONO_8 && cameraType != SHM::CAMERA_MONO_24)
//		{
//			if (verbose) printf("\t # ERROR SHM CLIENT: WRONG IMAGE CHANNEL DEPTH, EXPECTED %d OR %d BUT GOT %d\n", SHM::CAMERA_MONO_8, SHM::CAMERA_MONO_24, cameraType);
//			return false;
//		}

		if (!readImage(img))
		{
			if (verbose) printf("\t # ERROR SHM CLIENT: FAILED TO COPY FRAME FROM MEMORY\n");
			return false;
		}
	}
	while (mSHM.bytesWaiting() && mSubscribeLatest);
	
	return true;
}

bool
SHMImageClient::readStereoImage(const mavlink_message_t* msg, cv::Mat& imgLeft, cv::Mat& imgRight)
{
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}
	
	if (!mSHM.bytesWaiting())
	{
		return false;
	}
	
	do
	{
		SHM::CameraType cameraType;
		if (!readCameraType(cameraType))
		{
			return false;
		}

		if (cameraType != SHM::CAMERA_STEREO_8 && cameraType != SHM::CAMERA_STEREO_24)
		{
			return false;
		}

		if (!readImage(imgLeft, imgRight))
		{
			return false;
		}
	}
	while (mSHM.bytesWaiting() && mSubscribeLatest);
	
	return true;
}

bool
SHMImageClient::readKinectImage(const mavlink_message_t* msg, cv::Mat& imgBayer, cv::Mat& imgDepth)
{
	if (msg->msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE)
	{
		// Instantly return if MAVLink message did not contain an image
		return false;
	}

	if (!mSHM.bytesWaiting())
	{
		return false;
	}

	do
	{
		SHM::CameraType cameraType;
		if (!readCameraType(cameraType))
		{
			return false;
		}

		if (cameraType != SHM::CAMERA_KINECT)
		{
			return false;
		}

		if (!readImage(imgBayer, imgDepth))
		{
			return false;
		}
	}
	while (mSHM.bytesWaiting() && mSubscribeLatest);

	return true;
}

bool
SHMImageClient::readRGBDImage(cv::Mat& img, cv::Mat& imgDepth,
							  uint64_t& timestamp,
							  float& roll, float& pitch, float& yaw,
							  float& lon, float& lat, float& alt,
							  float& ground_x, float& ground_y, float& ground_z,
							  cv::Mat& cameraMatrix, cv::Rect& roi)
{
	if (!mSHM.bytesWaiting())
	{
		return false;
	}

	do
	{
		SHM::CameraType cameraType;
		if (!readCameraType(cameraType))
		{
			return false;
		}

		if (cameraType != SHM::CAMERA_RGBD)
		{
			return false;
		}

		if (!readImageWithCameraInfo(timestamp, roll, pitch, yaw,
									 lon, lat, alt,
									 ground_x, ground_y, ground_z,
									 cameraMatrix, roi, img, imgDepth))
		{
			return false;
		}
	}
	while (mSHM.bytesWaiting() && mSubscribeLatest);

	return true;
}

bool
SHMImageClient::readCameraType(SHM::CameraType& cameraType)
{
	uint32_t dataLength = mSHM.readDataPacket(mData, 4);
	if (dataLength < 4)
	{
		return false;
	}

	memcpy(&cameraType, &(mData[0]), 4);

	return true;
}

bool
SHMImageClient::readImage(cv::Mat& img)
{
	uint32_t dataLength = mSHM.readDataPacket(mData);
	if (dataLength <= 20)
	{
		return false;
	}

	int rows, cols, type;
	uint32_t step;

//	memcpy(&cameraType, &(mData[0]), 4);
	memcpy(&cols, &(mData[4]), 4);
	memcpy(&rows, &(mData[8]), 4);
	memcpy(&step, &(mData[12]), 4);
	memcpy(&type, &(mData[16]), 4);

	if (dataLength != 20 + rows * step)
	{
		// data length is not consistent with image type
		return false;
	}

	cv::Mat temp(rows, cols, type, &(mData[20]), step);
	temp.copyTo(img);

	return true;
}

bool
SHMImageClient::readImage(cv::Mat& img, cv::Mat& img2)
{
	uint32_t dataLength = mSHM.readDataPacket(mData);
	if (dataLength <= 28)
	{
		return false;
	}

	int rows, cols, type, type2;
	uint32_t step, step2;

//	memcpy(&cameraType, &(mData[0]), 4);
	memcpy(&cols, &(mData[4]), 4);
	memcpy(&rows, &(mData[8]), 4);
	memcpy(&step, &(mData[12]), 4);
	memcpy(&type, &(mData[16]), 4);
	memcpy(&step2, &(mData[20]), 4);
	memcpy(&type2, &(mData[24]), 4);

	if (dataLength != 28 + rows * step + rows * step2)
	{
		// data length is not consistent with image type
		return false;
	}

	cv::Mat temp(rows, cols, type, &(mData[28]), step);
	temp.copyTo(img);

	cv::Mat temp2(rows, cols, type2, &(mData[28 + rows * step]), step2);
	temp2.copyTo(img2);

	return true;
}

bool
SHMImageClient::readImageWithCameraInfo(uint64_t& timestamp,
										float& roll, float& pitch, float& yaw,
										float& lon, float& lat, float& alt,
										float& ground_x, float& ground_y, float& ground_z,
										cv::Mat& cameraMatrix, cv::Rect& roi,
										cv::Mat& img, cv::Mat& img2)
{
	uint32_t dataLength = mSHM.readDataPacket(mData);
	if (dataLength <= 124)
	{
		return false;
	}

	int rows, cols, type, type2;
	uint32_t step, step2;

//	memcpy(&cameraType, &(mData[0]), 4);
	memcpy(&timestamp, &(mData[4]), 8);
	memcpy(&roll, &(mData[12]), 4);
	memcpy(&pitch, &(mData[16]), 4);
	memcpy(&yaw, &(mData[20]), 4);
	memcpy(&lon, &(mData[24]), 4);
	memcpy(&lat, &(mData[28]), 4);
	memcpy(&alt, &(mData[32]), 4);
	memcpy(&ground_x, &(mData[36]), 4);
	memcpy(&ground_y, &(mData[40]), 4);
	memcpy(&ground_z, &(mData[44]), 4);

	int mark = 48;
	cameraMatrix = cv::Mat(3, 3, CV_32F);
	for (int i = 0; i < cameraMatrix.rows; ++i)
	{
		for (int j = 0; j < cameraMatrix.cols; ++j)
		{
			memcpy(&(cameraMatrix.at<float>(i,j)), &(mData[mark]), 4);
			mark += 4;
		}
	}

	memcpy(&(roi.x), &(mData[mark]), 4);
	memcpy(&(roi.y), &(mData[mark+4]), 4);
	memcpy(&(roi.width), &(mData[mark+8]), 4);
	memcpy(&(roi.height), &(mData[mark+12]), 4);

	memcpy(&cols, &(mData[mark+16]), 4);
	memcpy(&rows, &(mData[mark+20]), 4);
	memcpy(&step, &(mData[mark+24]), 4);
	memcpy(&type, &(mData[mark+28]), 4);
	memcpy(&step2, &(mData[mark+32]), 4);
	memcpy(&type2, &(mData[mark+36]), 4);

	if (dataLength != 124 + rows * step + rows * step2)
	{
		// data length is not consistent with image type
		return false;
	}

	cv::Mat temp(rows, cols, type, &(mData[124]), step);
	temp.copyTo(img);

	cv::Mat temp2(rows, cols, type2, &(mData[124 + rows * step]), step2);
	temp2.copyTo(img2);

	return true;
}

}
