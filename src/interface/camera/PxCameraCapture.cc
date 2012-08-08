/*=====================================================================

 PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

 (c) 2009, 2010 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

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
 *   @brief Implementation of libdc1394v2 based camera interface
 *
 *   @author Pascal Dufour <mavteam@student.ethz.ch>
 *
 */

#include <inttypes.h>
#include <dc1394.h>
#include <unistd.h>

#include "PxCameraCapture.h"

int nDMAbuffers = 10;

typedef union _fint32
{
	uint32_t reg;
	float freg;
} fint32_t;

PxCameraError PxInitializeStereoCapture(PxCameraCapture &capture1, PxCameraCapture &capture2, uint32_t serialNr1, uint32_t serialNr2, bool extTrigger, bool automode, uint32_t exposure, uint32_t gain, uint32_t gamma)
{
	if (capture1.init(serialNr1, false) != CAMERA_SUCCESS)
	{
		capture1.destroy();
		return CAMERA_FAILURE;
	}
	if (capture2.init(serialNr2, false) != CAMERA_SUCCESS)
	{
		capture1.destroy();
		capture2.destroy();
		return CAMERA_FAILURE;
	}

	PxCameraMode mode = CAMERA_MANUAL_MODE;
	if (automode)
	{
		printf("Auto brightness/gain/exposure/gamma\n");
		mode = CAMERA_AUTO_MODE;
	}
	else
	{
		printf("Set fixed brightness/gain/exposure/gamma\n");
	}
	PxCameraSettings settings(mode, exposure, gain, gamma);
	capture1.setSettings(settings);
	capture2.setSettings(settings);

	float fps = 60.0f;
	if (extTrigger)
	{
		fps = 30.0f;

		capture1.setExternalTrigger();
		capture2.setExternalTrigger();

		printf("Sync Cable can be used, setting up camera 1 sending strobe and camera 2 as slave listening on GPIO_0 for external trigger\n");

		printf("Activating strobe on GPIO_2\n");
		if (capture1.setStrobe(2) != CAMERA_SUCCESS)
		{
			capture1.destroy();
			capture2.destroy();
			return CAMERA_FAILURE;
		}
	}
	else
	{
		printf("No external trigger, cameras will be synchronized as good as possible, no guarantee though...\n");
	}

	capture1.setFrameRate(fps);
	capture2.setFrameRate(fps);

	//start transmission - always start slave camera first
	printf("Start transmission\n");
	if (capture2.start() != CAMERA_SUCCESS)
	{
		printf("Error starting transmission for slave camera.\n");
		capture1.destroy();
		capture2.destroy();
		return CAMERA_FAILURE;
	}
	if (capture1.start() != CAMERA_SUCCESS)
	{
		printf("Error starting transmission for master camera.\n");
		capture1.destroy();
		capture2.destroy();
		return CAMERA_FAILURE;
	}

	usleep(250000);

	printf("Cameras initialized successfully!\n");
	return CAMERA_SUCCESS;
}

dc1394error_t convertToCvMat(dc1394video_frame_t* frame, cv::Mat& image)
{
	if (!frame)
	{
		printf("NULL pointer in convertToCvMat()\n");
		return DC1394_FAILURE;
	}

	cv::Mat temp(cv::Size(frame->size[0], frame->size[1]), CV_8UC1, frame->image, frame->stride);

	//if the given image has not the same format release its data and allocate new one
	temp.copyTo(image);

	return DC1394_SUCCESS;
}

PxCameraError PxGrabStereoFrames(PxCameraCapture &capture1, PxCameraCapture &capture2, cv::Mat& image1, cv::Mat& image2, uint32_t *skipped_frames, uint32_t *image_sequence, bool extTrigger)
{
	dc1394error_t error;
	dc1394video_frame_t *frame1, *frame2;

	error = dc1394_capture_dequeue(capture1.camera, DC1394_CAPTURE_POLICY_WAIT, &frame1);
	if (error != DC1394_SUCCESS)
	{
		printf("Error dequeuing frame (1): %s\n", dc1394_error_get_string(error));
		return CAMERA_FAILURE;
	}
	error = dc1394_capture_dequeue(capture2.camera, DC1394_CAPTURE_POLICY_WAIT, &frame2);
	if (error != DC1394_SUCCESS)
	{
		error = dc1394_capture_enqueue(capture1.camera, frame1); //enqueue the dequeued frame of camera 1 - perhaps we can continue...
		if (error != DC1394_SUCCESS)
		{
			printf("Error enqueuing frame (1) after dequeue error (2), now we're totally screwed, sorry: %s\n", dc1394_error_get_string(error));
			return CAMERA_FAILURE;
		}
		else
		{
			printf("Error dequeuing frame (2): %s\n", dc1394_error_get_string(error));
			return CAMERA_FAILURE;
		}
	}
	//now check the field frames_behind to see if there are newer images in the buffer and skip to the newest image pair
	uint32_t skipped_frames1 = 0;
	uint32_t skipped_frames2 = 0;

	//printf("%u/%u behind\n", frame1->frames_behind, frame2->frames_behind);

	if (extTrigger)
	{
		while (std::min(frame1->frames_behind, frame2->frames_behind) > 0)
		{
			error = dc1394_capture_enqueue(capture1.camera, frame1);
			if (error != DC1394_SUCCESS)
			{
				printf("Error enqueuing frame (1)\n");
				return CAMERA_FAILURE;
			}
			error = dc1394_capture_dequeue(capture1.camera, DC1394_CAPTURE_POLICY_POLL, &frame1);
			if (error != DC1394_SUCCESS)
			{
				printf("Error dequeuing frame (1)\n");
				return CAMERA_FAILURE;
			}
			error = dc1394_capture_enqueue(capture2.camera, frame2);
			if (error != DC1394_SUCCESS)
			{
				printf("Error enqueuing frame (1)\n");
				return CAMERA_FAILURE;
			}
			error = dc1394_capture_dequeue(capture2.camera, DC1394_CAPTURE_POLICY_POLL, &frame2);
			if (error != DC1394_SUCCESS)
			{
				printf("Error dequeuing frame (1)\n");
				return CAMERA_FAILURE;
			}

			skipped_frames1++;
			skipped_frames2++;
		}
	}
	else
	{
		while (frame1->frames_behind > 0)
		{
			error = dc1394_capture_enqueue(capture1.camera, frame1);
			if (error != DC1394_SUCCESS)
			{
				printf("Error enqueuing frame (1)\n");
				return CAMERA_FAILURE;
			}
			error = dc1394_capture_dequeue(capture1.camera, DC1394_CAPTURE_POLICY_POLL, &frame1);
			if (error != DC1394_SUCCESS)
			{
				printf("Error dequeuing frame (1)\n");
				return CAMERA_FAILURE;
			}
			skipped_frames1++;
		}
		while (frame2->frames_behind > 0)
		{
			error = dc1394_capture_enqueue(capture2.camera, frame2);
			if (error != DC1394_SUCCESS)
			{
				printf("Error enqueuing frame (1)\n");
				return CAMERA_FAILURE;
			}
			error = dc1394_capture_dequeue(capture2.camera, DC1394_CAPTURE_POLICY_POLL, &frame2);
			if (error != DC1394_SUCCESS)
			{
				printf("Error dequeuing frame (1)\n");
				return CAMERA_FAILURE;
			}
			skipped_frames2++;
		}
	}

	if (skipped_frames != NULL)
		*skipped_frames = skipped_frames1;

	if (image_sequence != NULL)
		*image_sequence = frame1->image[0] << 24 | frame1->image[1] << 16 | frame1->image[2] << 8 | frame1->image[3];

	//convert to cv::Mat
	error = convertToCvMat(frame1, image1);
	if (error != DC1394_SUCCESS)
	{
		printf("Error converting frame (1) to cv::Mat.\n");
		return CAMERA_FAILURE;
	}
	error = convertToCvMat(frame2, image2);
	if (error != DC1394_SUCCESS)
	{
		printf("Error converting frame (2) to cv::Mat.\n");
		return CAMERA_FAILURE;
	}

	//now enqueue the processed images
	error = dc1394_capture_enqueue(capture1.camera, frame1);
	if (error != DC1394_SUCCESS)
	{
		printf("Error enqueuing the frame (1)\n");
		return CAMERA_FAILURE;
	}
	error = dc1394_capture_enqueue(capture2.camera, frame2);
	if (error != DC1394_SUCCESS)
	{
		printf("Error enqueuing the frame (2)\n");
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::init(uint32_t serialNum, bool special)
{
	this->special = special;

	dc1394error_t error;
	dc1394camera_list_t* camList;
	//dc1394video_frame_t* frame; //belongs to the DMA buffer, do NOT free!

	//create a new context to search and use cameras
	context = dc1394_new();

	if (context == NULL)
	{
		printf("libdc1394 init error.\n");
		dc1394_free(context);
	}

	//get all connected cameras
	printf("enumerating cameras...\n");
	error = dc1394_camera_enumerate(context, &camList);
	if (error != DC1394_SUCCESS)
	{
		printf ("Error during enumeration: %s\n", dc1394_error_get_string(error));
		return CAMERA_FAILURE;
	}

	printf("found %u camera(s)\n", camList->num);

	camera = NULL;

	// List cameras
	for (unsigned int i = 0; i < camList->num; i++)
	{
		printf("Camera %u, serial %u \n", i, (uint32_t)camList->ids[i].guid);
		//camera->guid is funny, the upper 32 bit are SOMETHING no idea what and the lower 32 bit represent the printed serial on the camera
	}

	// Open camera
	for (unsigned int i = 0; i < camList->num; i++)
	{
		//camera->guid is funny, the upper 32 bit are SOMETHING no idea what and the lower 32 bit represent the printed serial on the camera
		if ((uint32_t)camList->ids[i].guid == serialNum || serialNum == 0)
		{
			//this camera's serial nr matches the requested one of the first capture
			//set the capture structure accordingly
			camera = dc1394_camera_new (context, camList->ids[i].guid);
			printf("Opening camera %u with serial %u for capture.. ", i, (uint32_t)camList->ids[i].guid);
			if (camera == NULL)
			{
				printf("could not be created.\n");
				dc1394_camera_free_list(camList);
				destroy();
				return CAMERA_FAILURE;
			}
			serialNum = camList->ids[i].guid;
			busIdx = i;
			printf("created.\n");

			//we found a camera, stop iterating here
			break;
		}
	}
	dc1394_camera_free_list(camList);

	//check if camera creation failed
	if (camera == NULL)
	{
		printf("Error: Could not find camera #%u\n", (serialNum));
		destroy();
		return CAMERA_FAILURE;
	}

	//reset camera
	printf("Reset camera\n");
	error = dc1394_camera_reset(camera);
	if (error != DC1394_SUCCESS) { printf("Could not reset camera 1\n"); destroy(); return CAMERA_FAILURE; }
	usleep(250000);

	//reset bus - this is very rude and may disrupt other cameras from working
	printf("Reset bus\n");
	error = dc1394_reset_bus(camera);
	if (error != DC1394_SUCCESS) { printf("Could not reset bus of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	usleep(250000);

	//set video mode to 640x480 Mono 8
	printf("Set video mode to 640x480x8\n");
	error = dc1394_video_set_mode(camera, DC1394_VIDEO_MODE_640x480_MONO8);
	if (error != DC1394_SUCCESS) { printf("Could not set video mode for camera 1\n"); destroy(); return CAMERA_FAILURE; }

	if (special)
	{
		//set framerate to 7.5 Hz
		printf("Set framerate to 7.5 Hz\n");
		setFrameRate(7.5f);
	}
	else
	{
		//set framerate to 60 Hz
		printf("Set framerate to 60 Hz\n");
		setFrameRate(60.0f);
	}
	if (error != DC1394_SUCCESS) { printf("Could not setup framerate for camera 1\n"); destroy(); return CAMERA_FAILURE; }

	//set capture flags
	printf("Set DMA buffer size to %d images\n", nDMAbuffers);
	error = dc1394_capture_setup(camera, nDMAbuffers, DC1394_CAPTURE_FLAGS_DEFAULT);
	if (error != DC1394_SUCCESS) { printf("Could not setup flags for camera 1\n"); destroy(); return CAMERA_FAILURE; }

	//turn on the embedding of the timestamp in each frame
	//set 0x12F8 FRAME_INFO register (bit 31 to 1)
	printf("Set embedded frame counter\n");
	error = dc1394_set_control_register(camera, 0x12f8, 0x80000040);
	if (error != DC1394_SUCCESS) { printf("could not write to control register 0x12f8 of camera 1\n"); destroy(); return CAMERA_FAILURE; }

	//read the absolute shutter value
	fint32_t fancyReg;
	error = dc1394_get_control_register(camera, 0x918, &fancyReg.reg);
	if (error != DC1394_SUCCESS) { printf("Could not read absolute shutter time of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	printf("Shutter time: %f ms\n", fancyReg.freg * 1000.f);

	error = dc1394_get_control_register(camera, 0x928, &fancyReg.reg);
	if (error != DC1394_SUCCESS) { printf("Could not read absolute brightness of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	printf("Gain: %f dB\n", fancyReg.freg);

	error = dc1394_get_control_register(camera, 0x938, &fancyReg.reg);
	if (error != DC1394_SUCCESS) { printf("Could not read absolute gain of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	printf("Brightness: %f%%\n", fancyReg.freg);

	error = dc1394_get_control_register(camera, 0x948, &fancyReg.reg);
	if (error != DC1394_SUCCESS) { printf("Could not read absolute gamma of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	printf("Gamma: %f\n", fancyReg.freg);


	//debugging information
	error = dc1394_get_control_register(camera, 0x1028, &fancyReg.reg);
	if (error != DC1394_SUCCESS) { printf("Could not read extended shutter register of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	printf("Extended shutter available: %s\n", (fancyReg.reg&1)?"yes":"no");

	error = dc1394_get_control_register(camera, 0x1104, &fancyReg.reg);
	if (error != DC1394_SUCCESS) { printf("Could not read gpio extra register of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	else
	{
		error = dc1394_set_control_register(camera, 0x1104, fancyReg.reg | 0x2);
		if (error != DC1394_SUCCESS) { printf("Could not write gpio extra register of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	}
	error = dc1394_get_control_register(camera, 0x1104, &fancyReg.reg);
	if (error != DC1394_SUCCESS) { printf("Could not read gpio extra register of camera 1\n"); destroy(); return CAMERA_FAILURE; }
	printf("Trigger is %s if arrived during exposure\n", (fancyReg.reg&2)?"dropped":"queued");

	// clear the image buffer
	dc1394video_frame_t *frame1;
	error = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame1);
	if (error != DC1394_SUCCESS)
	{
		printf("Error dequeuing frame: %s\n", dc1394_error_get_string(error));
		return CAMERA_FAILURE;
	}
	if (frame1)
	{
		uint32_t i = 1;
		printf("%u.. ", i++); fflush(stdout);
		uint32_t frames_to_skip = frame1->frames_behind;
		while (frames_to_skip > 0)
		{
			error = dc1394_capture_enqueue(camera, frame1);
			if (error != DC1394_SUCCESS)
			{
				printf("Error enqueuing frame\n");
				return CAMERA_FAILURE;
			}
			error = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame1);
			if (error != DC1394_SUCCESS)
			{
				printf("Error dequeuing frame\n");
				return CAMERA_FAILURE;
			}
			printf("%u.. ", i++); fflush(stdout);
			frames_to_skip = frame1->frames_behind;
		}
		error = dc1394_capture_enqueue(camera, frame1);
		if (error != DC1394_SUCCESS)
		{
			printf("Error enqueuing frame\n");
			return CAMERA_FAILURE;
		}
		printf("\n");
	}

	printf("Cameras initialized successfully!\n");
	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::start(void)
{
	//start transmission
	printf("Start transmission\n");
	if (startTransmission() != CAMERA_SUCCESS)
	{
		printf("Error starting transmission\n");
		destroy();
		return CAMERA_FAILURE;
	}

	usleep(250000);

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::destroy(void)
{
	printf("Reset camera and free resources\n");
	if (camera != NULL)
	{
		dc1394_camera_reset(camera);
		dc1394_camera_free(camera);
		camera = NULL;
	}
	if (context != NULL)
	{
		dc1394_free(context);
		context = NULL;
	}
	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::grabFrame(cv::Mat& image, uint32_t *skipped_frames, uint32_t *image_sequence)
{
	dc1394video_frame_t *frame;

	dc1394error_t error = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
	if (error != DC1394_SUCCESS)
	{
		printf("Error dequeuing frame: %s\n", dc1394_error_get_string(error));
		return CAMERA_FAILURE;
	}

	//now check the field frames_behind to see if there are newer images in the buffer and skip to the newest image
	uint32_t skipped_frames1 = 0;
	while (frame->frames_behind > 0)
	{
		error = dc1394_capture_enqueue(camera, frame);
		if (error != DC1394_SUCCESS)
		{
			printf("Error enqueuing frame\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame);
		if (error != DC1394_SUCCESS)
		{
			printf("Error dequeuing frame\n");
			return CAMERA_FAILURE;
		}

		skipped_frames1++;
	}

	if (skipped_frames != NULL)
	{
		*skipped_frames = skipped_frames1;
	}

	if (image_sequence != NULL)
	{
		*image_sequence = frame->image[0] << 24 | frame->image[1] << 16 | frame->image[2] << 8 | frame->image[3];
	}

	//convert to cv::Mat
	error = convertToCvMat(frame, image);
	if (error != DC1394_SUCCESS)
	{
		printf("Error converting frame (1) to cv::Mat.\n");
		return CAMERA_FAILURE;
	}

	//now enqueue the processed images
	error = dc1394_capture_enqueue(camera, frame);
	if (error != DC1394_SUCCESS)
	{
		printf("Error enqueuing the frame (1)\n");
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setSettings(PxCameraSettings& settings)
{
	if (setMode(settings.getMode()) != CAMERA_SUCCESS)
	{
		return CAMERA_FAILURE;
	}
	if (setShutter(settings.getShutter()) != CAMERA_SUCCESS)
	{
		return CAMERA_FAILURE;
	}
	if (setGain(settings.getGain()) != CAMERA_SUCCESS)
	{
		return CAMERA_FAILURE;
	}
	if (setGamma(settings.getGamma()) != CAMERA_SUCCESS)
	{
		return CAMERA_FAILURE;
	}
	if (setBrightness(settings.getBrightness()) != CAMERA_SUCCESS)
	{
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

dc1394error_t
PxCameraCapture::setFrameRate(float frameRate)
{
	dc1394error_t error = DC1394_INVALID_ARGUMENT_VALUE;
	if (frameRate == 7.5f)
	{
		error = dc1394_video_set_framerate(camera, DC1394_FRAMERATE_7_5);
	}
	else if (frameRate == 30.0f)
	{
		error = dc1394_video_set_framerate(camera, DC1394_FRAMERATE_30);
	}
	else if (frameRate == 60.0f)
	{
		error = dc1394_video_set_framerate(camera, DC1394_FRAMERATE_60);
	}
	else
	{
		printf("Unknown frame rate.\n");
	}

	return error;
}

PxCameraError
PxCameraCapture::setExternalTrigger(void)
{
	uint32_t desiredPinDirection[4] = {0, 1, 1, 0};

	// check, and if necessary, correct pin direction settings
	// GPIO0 and GPIO3 should be set as inputs while GPIO1 and GPIO2 should be set as outputs
	for (int i = 0; i < 4; ++i)
	{
		uint32_t pinDirection = 0;
		if (getGPIOPinDirection(i, pinDirection) == CAMERA_SUCCESS)
		{
			if (pinDirection != desiredPinDirection[i])
			{
				if (setGPIOPinDirection(i, desiredPinDirection[i]) != CAMERA_SUCCESS)
				{
					return CAMERA_FAILURE;
				}
			}
		}
		else
		{
			return CAMERA_FAILURE;
		}
	}

	printf("Activating external trigger on GPIO_0 for camera\n");
	dc1394error_t error = dc1394_external_trigger_set_mode(camera, DC1394_TRIGGER_MODE_0);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not set trigger for camera 1\n");
		destroy();
		return CAMERA_FAILURE;
	}
	error = dc1394_external_trigger_set_source(camera, DC1394_TRIGGER_SOURCE_0);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not set trigger for camera 1\n");
		destroy();
		return CAMERA_FAILURE;
	}
	error = dc1394_external_trigger_set_polarity(camera, DC1394_TRIGGER_ACTIVE_LOW);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not set trigger for camera 1\n");
		destroy();
		return CAMERA_FAILURE;
	}
	error = dc1394_external_trigger_set_power(camera, DC1394_ON);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not set trigger for camera 1\n");
		destroy();
		return CAMERA_FAILURE;
	}

	// recheck that the pin directions are set correctly
	for (int i = 0; i < 4; ++i)
	{
		uint32_t pinDirection = 0;
		if (getGPIOPinDirection(i, pinDirection) == CAMERA_SUCCESS)
		{
			if (pinDirection != desiredPinDirection[i])
			{
				if (setGPIOPinDirection(i, desiredPinDirection[i]) != CAMERA_SUCCESS)
				{
					return CAMERA_FAILURE;
				}
			}
		}
		else
		{
			return CAMERA_FAILURE;
		}
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setStrobe(uint32_t pin)
{
	bool flag = false;
	PxCameraError error = isStrobeEnabled(pin, flag);
	if (error != CAMERA_SUCCESS)
	{
		printf("Could not read strobe control inquiry register of camera.\n");
		return CAMERA_FAILURE;
	}
	if (!flag)
	{
		printf("Strobe feature not available on GPIO%d on camera.\n", pin);
		return CAMERA_FAILURE;
	}

	error = isStrobeConfigurable(pin, flag);
	if (error != CAMERA_SUCCESS)
	{
		printf("Could not read strobe control inquiry register of camera.\n");
		return CAMERA_FAILURE;
	}
	if (!flag)
	{
		printf("Strobe feature for GPIO%d not configurable on camera.\n", pin);
		return CAMERA_FAILURE;
	}

	error = setStrobeSource(pin);
	if (error != CAMERA_SUCCESS)
	{
		printf("Could not set strobe for GPIO%d of camera.\n", pin);
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::startTransmission(void)
{
	dc1394error_t error = dc1394_video_set_transmission(camera, DC1394_ON);
	if (error != DC1394_SUCCESS)
	{
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setMode(PxCameraMode mode)
{
	dc1394error_t error;

	if (mode == CAMERA_MANUAL_MODE)
	{
		//set manual brightness
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_BRIGHTNESS, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable manual brightness feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_BRIGHTNESS, DC1394_FEATURE_MODE_MANUAL);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn off auto brightness mode\n");
			return CAMERA_FAILURE;
		}

		//set manual shutter
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_SHUTTER, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable manual shutter feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_SHUTTER, DC1394_FEATURE_MODE_MANUAL);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn off auto shutter\n");
			return CAMERA_FAILURE;
		}

		//set manual gain
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_GAIN, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable manual gain feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_GAIN, DC1394_FEATURE_MODE_MANUAL);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn off auto gain\n");
			return CAMERA_FAILURE;
		}

		//set manual gamma
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_GAMMA, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable manual gamma feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_GAMMA, DC1394_FEATURE_MODE_MANUAL);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn off auto gamma\n");
			return CAMERA_FAILURE;
		}
	}
	else if (mode == CAMERA_AUTO_MODE)
	{
		//set auto brightness
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_BRIGHTNESS, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable auto brightness feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_BRIGHTNESS, DC1394_FEATURE_MODE_AUTO);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn on auto brightness mode\n");
			return CAMERA_FAILURE;
		}

		//set auto shutter
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_SHUTTER, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable auto shutter feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_SHUTTER, DC1394_FEATURE_MODE_AUTO);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn on auto shutter\n");
			return CAMERA_FAILURE;
		}

		//set auto gain
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_GAIN, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable auto gain feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_GAIN, DC1394_FEATURE_MODE_AUTO);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn on auto gain\n");
			return CAMERA_FAILURE;
		}

		//set auto gamma
		error = dc1394_feature_set_power(camera, DC1394_FEATURE_GAMMA, DC1394_ON);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not enable auto gamma feature\n");
			return CAMERA_FAILURE;
		}
		error = dc1394_feature_set_mode(camera, DC1394_FEATURE_GAMMA, DC1394_FEATURE_MODE_AUTO);
		if (error != DC1394_SUCCESS)
		{
			printf("Could not turn on auto gamma\n");
			return CAMERA_FAILURE;
		}
	}
	else
	{
		printf("Unknown camera mode\n");
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setShutter(uint32_t shutter)
{
	// shutter is given in microseconds, compute the correct register value
	// (the constant factor can change with different camera models, this one is correct for PTGrey Firefly MV)
	if (special)
	{
		shutter *= 0.004096;
	}
	else
	{
		shutter *= 0.032768;
	}

	dc1394error_t error = dc1394_feature_set_value(camera, DC1394_FEATURE_SHUTTER, shutter);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not set shutter exposure\n");
		return CAMERA_FAILURE;
	}
	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setGain(uint32_t gain)
{
	dc1394error_t error = dc1394_feature_set_value(camera, DC1394_FEATURE_GAIN, gain);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not set gain\n");
		return CAMERA_FAILURE;
	}
	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setGamma(uint32_t gamma)
{
	dc1394error_t error = dc1394_feature_set_value(camera, DC1394_FEATURE_GAMMA, gamma);
	if (error != DC1394_SUCCESS) {
		printf("Could not set gamma\n");
		return CAMERA_FAILURE;
	}
	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setBrightness(uint32_t brightness)
{
	dc1394error_t error = dc1394_feature_set_value(camera, DC1394_FEATURE_BRIGHTNESS, brightness);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not set brightness\n");
		return CAMERA_FAILURE;
	}
	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::getGPIOPinDirection(uint32_t pin, uint32_t& pDirection)
{
	uint32_t val = 0;
	dc1394error_t error = dc1394_get_control_register(camera, (uint64_t) 0x11F8, &val);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not read pin direction register of camera.\n");
		destroy();
		return CAMERA_FAILURE;
	}

	pDirection = (val >> (31 - pin)) & 0x1;

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setGPIOPinDirection(uint32_t pin, uint32_t pDirection)
{
	uint32_t val = 0;
	dc1394error_t error = dc1394_get_control_register(camera, (uint64_t) 0x11F8, &val);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not read pin direction register of camera.\n");
		destroy();
		return CAMERA_FAILURE;
	}

	// ‘0’ indicates it is a input/trigger, and ‘1’ indicates it is an output/strobe
	if (pDirection == 0)
	{
		uint32_t mask = ~(0x1 << (31 - pin));
		val &= mask;
	}
	else
	{
		uint32_t mask = 0x1 << (31 - pin);
		val |= mask;
	}

	error = dc1394_set_control_register(camera, (uint64_t) 0x11F8, val);
	if (error != DC1394_SUCCESS)
	{
		printf("Could not write correct pin direction register of camera.\n");
		destroy();
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::isStrobeEnabled(uint32_t pin, bool& flag)
{
	uint32_t val = 0;

	dc1394error_t error = dc1394_get_control_register(camera, (uint64_t) 0x1300, &val);
	if (error != DC1394_SUCCESS)
	{
		return CAMERA_FAILURE;
	}

	flag = (val >> (31 - pin)) & 0x1;
	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::isStrobeConfigurable(uint32_t pin, bool& flag)
{
	uint64_t registerOffset[4] = {0x1400, 0x1404, 0x1408, 0x140C};

	uint32_t val = 0;
	dc1394error_t error = dc1394_get_control_register(camera, registerOffset[pin], &val);
	if (error != DC1394_SUCCESS)
	{
		return CAMERA_FAILURE;
	}

	if ((val & 0x8E000000) != 0x8E000000)
	{
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}

PxCameraError
PxCameraCapture::setStrobeSource(uint32_t pin)
{
	uint64_t registerOffset[4] = {0x1500, 0x1504, 0x1508, 0x150C};

	dc1394error_t error = dc1394_set_control_register(camera, registerOffset[pin], (uint32_t) 0x82000800);
	if (error != DC1394_SUCCESS)
	{
		return CAMERA_FAILURE;
	}

	return CAMERA_SUCCESS;
}
