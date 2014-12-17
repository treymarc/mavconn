#include <inttypes.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <glibmm.h>
#include <time.h>
// BOOST includes
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
// OpenCV includes
#include <opencv2/highgui/highgui.hpp>

#include "interface/shared_mem/SHMImageServer.h"
#include "mavconn.h"

namespace config = boost::program_options;
namespace bfs = boost::filesystem;

std::string logfile;
std::string imagepath_left;
std::string imagepath_right;
Glib::ustring orientation;
bool publishExtended = false;
uint64_t camid_left = 0;  ///< Left Camera unique id
uint64_t camid_right = 0; ///< Right camera unique id
bool silent, verbose;
int sysid = getSystemID();
int compid = PX_COMP_ID_CAMERA;

int main(int argc, char* argv[])
{
	// parse run-time arguments
		Glib::OptionGroup optGroup("options", "options", "Configuration options");

		Glib::OptionEntry optClassifierFile;
		optClassifierFile.set_short_name('m');
		optClassifierFile.set_long_name("logfile");
		optClassifierFile.set_description("Path of MAVLink log file (*.mavlink)");

		Glib::OptionEntry optLeft;
		optLeft.set_short_name('l');
		optLeft.set_long_name("images_left");
		optLeft.set_description("Folder with the left camera images (or images for single camera setups)");

		Glib::OptionEntry optLeftSerial;
		optLeftSerial.set_long_name("serial_left");
		optLeftSerial.set_description("serial # of left camera)");

		Glib::OptionEntry optRight;
		optRight.set_short_name('r');
		optRight.set_long_name("images_right");
		optRight.set_description("Folder with the right camera images (or empty for single camera setups)");

		Glib::OptionEntry optRightSerial;
		optRightSerial.set_long_name("serial_right");
		optRightSerial.set_description("serial # of right camera)");

		Glib::OptionEntry optStartFrom;
		optStartFrom.set_long_name("start_image");
		optStartFrom.set_description("Timestamp of the image from which to start the replay");

		Glib::OptionEntry optVerbose;
		optVerbose.set_short_name('v');
		optVerbose.set_long_name("verbose");
		optVerbose.set_description("Verbose output");

		Glib::OptionEntry optSilent;
		optSilent.set_short_name('s');
		optSilent.set_long_name("silent");
		optSilent.set_description("No output");

		Glib::OptionEntry optOrientation;
		optOrientation.set_long_name("orientation");
		optOrientation.set_description("Orientation of the camera (forward|downward)");

		Glib::OptionEntry optPublishExtended;
		optPublishExtended.set_long_name("publish_extended");
		optPublishExtended.set_description("Publish extended MAVLINK messages");

		bool verbose = false;
		optGroup.add_entry_filename(optClassifierFile, logfile);
		optGroup.add_entry_filename(optLeft, imagepath_left);
		int32_t idleft;
		optGroup.add_entry(optLeftSerial, idleft);
		optGroup.add_entry_filename(optRight, imagepath_right);
		int32_t idright;
		optGroup.add_entry(optRightSerial, idright);
		Glib::ustring fromTimestampString;
		optGroup.add_entry(optStartFrom, fromTimestampString);
		optGroup.add_entry(optVerbose, verbose);
		optGroup.add_entry(optSilent, silent);
		optGroup.add_entry(optOrientation, orientation);
		optGroup.add_entry(optPublishExtended, publishExtended);

		Glib::OptionContext optContext("");
		optContext.set_help_enabled(true);
		optContext.set_ignore_unknown_options(true);
		optContext.set_main_group(optGroup);

		try
		{
			if (!optContext.parse(argc, argv))
			{
				fprintf(stderr, "# ERROR: Cannot parse options.\n");
				return 1;
			}
		}
		catch (Glib::OptionError& error)
		{
			fprintf(stderr, "# ERROR: Cannot parse options.\n");
			return 1;
		}

		// Try to guess if only option is the filename
		if (argc == 2)
		{
			logfile = std::string(argv[1]);
		}

		camid_left = idleft;
		camid_right = idright;
		uint64_t startTimestamp = strtoull(fromTimestampString.c_str(), NULL, 0);

	std::ifstream mavlinkLog;
	mavlinkLog.open(logfile.c_str(), std::ios::binary | std::ios::in);
	if (!mavlinkLog)
	{
		printf("mavconn-replay: Error opening MAVLINK log file: %s\n", logfile.c_str());
		return EXIT_FAILURE;
	}

	//try standard folders for left and right images if they were not defined
	if(imagepath_left.length() == 0)
	{
		imagepath_left = logfile.substr(0, logfile.length()-8) + "/left";
		printf("mavconn-replay: Left images stream path not specified, trying %s\n", imagepath_left.c_str());
	}

	if(imagepath_right.length() == 0)
	{
		imagepath_right = logfile.substr(0, logfile.length()-8) + "/right";
		printf("mavconn-replay: Right images stream path not specified, trying %s\n", imagepath_right.c_str());
	}

	// ----- Setting up communication and data for images
	// Creating LCM network provider
	lcm_t* lcmImage = NULL;
	lcm_t* lcmMavlink = lcm_create ("udpm://");
	if (!lcmMavlink)
		exit(EXIT_FAILURE);

	bool do_images = false;
	bool do_stereo = false;
	px::SHMImageServer cam;
	std::map<uint64_t, std::string> images_left;
	std::map<uint64_t, std::string> images_right;
	typedef std::pair<const uint64_t, std::string> image_type;
	bfs::path ipath_left(imagepath_left);
	bfs::path ipath_right(imagepath_right);
	if (bfs::exists(ipath_left) && bfs::is_directory(ipath_left))
	{
		do_images = true;

		lcmImage = lcm_create ("udpm://");
		if (!lcmImage)
			exit(EXIT_FAILURE);

		//cam = new PxSharedMemServer(sysid, compid, 640, 480, 8, 1, 2011);
		//mavconn_mavlink_msg_container_t_subscription_t * img_sub  = mavlink_message_t_subscribe (lcmImage, MAVLINK_IMAGES, &image_handler, cam);
		//mavconn_mavlink_msg_container_t_subscription_t * comm_sub = mavlink_message_t_subscribe (lcmMavlink, MAVLINK_MAIN, &mavlink_handler, lcmMavlink);

		printf("mavconn-replay: Found left camera image stream, loading image list...\n");

		if (imagepath_left.size() > 0 && imagepath_left[imagepath_left.size() - 1] != '/')
			imagepath_left += '/';

		try
		{
			bfs::directory_iterator file(imagepath_left);
			bfs::directory_iterator end;

			std::string extension;
			std::string filename;

			while (file != end)
			{
				extension = bfs::extension(*file);
				if (extension == ".bmp")
				{
					filename = bfs::basename(*file);
					uint64_t timestamp = strtoull(filename.c_str(), NULL, 10);
					image_type val(timestamp, file->path().string());
					images_left.insert(val);
					if (verbose) printf("found %s\n", file->path().string().c_str());
				}
				++file;
			}
		}
		catch (...) {}

		//check for right images
		if (bfs::exists(ipath_right) && bfs::is_directory(ipath_right))
		{
			printf("mavconn-replay: Found right camera image stream, loading image list...\n");

			if (imagepath_right.size() > 0 && imagepath_right[imagepath_right.size() - 1] != '/')
				imagepath_right += '/';

			try
			{
				bfs::directory_iterator file(imagepath_right);
				bfs::directory_iterator end;

				std::string extension;
				std::string filename;

				while (file != end)
				{
					extension = bfs::extension(*file);
					if (extension == ".bmp")
					{
						filename = bfs::basename(*file);
						uint64_t timestamp = strtoull(filename.c_str(), NULL, 10);
						image_type val(timestamp, file->path().string());
						images_right.insert(val);
						if (verbose) printf("found %s\n", file->path().string().c_str());
					}
					++file;
				}
			}
			catch (...) {}
		}

		//if we found any right camera images activate stereo mode
		if (!images_right.empty())
		{
			printf("Outputting stereo stream\n");
			do_stereo = true;
		}
		else
		{
			printf("Outputting mono stream\n");
			do_stereo = false;
		}

	}
	else
	{
		printf("mavconn-replay: No images available\n");
	}

	if (do_stereo)
	{
		if (orientation == "forward")
		{
			cam.init(sysid, compid, lcmImage, px::SHM::CAMERA_FORWARD_LEFT, px::SHM::CAMERA_FORWARD_RIGHT);
			printf("mavconn-replay: Initializing stereo FRONT camera replay\n");
		}
		else
		{
			cam.init(sysid, compid, lcmImage, px::SHM::CAMERA_DOWNWARD_LEFT, px::SHM::CAMERA_DOWNWARD_RIGHT);
			printf("mavconn-replay: Initializing stereo DOWN camera replay\n");
		}
	}
	else
	{
		if (orientation == "forward")
		{
			cam.init(sysid, compid, lcmImage, px::SHM::CAMERA_FORWARD_LEFT);
			printf("mavconn-replay: Initializing monocular FRONT camera replay\n");
		}
		else
		{
			cam.init(sysid, compid, lcmImage, px::SHM::CAMERA_DOWNWARD_LEFT);
			printf("mavconn-replay: Initializing monocular DOWN camera replay\n");
		}
	}

	std::map<uint64_t, std::string>::iterator sync_image_left_it = images_left.begin();
	std::map<uint64_t, std::string>::iterator sync_image_right_it = images_right.begin();

	const int len = MAVLINK_MAX_PACKET_LEN+sizeof(uint64_t);
	unsigned char buf[len];
	uint64_t last_time = 0;

	printf("mavconn-replay: Start playing logfile %s...\n", logfile.c_str());

	if(startTimestamp > 0)
		printf("trying to skip to timestamp %lu...\n", startTimestamp);

	GTimeVal gtime;
	g_get_current_time(&gtime);
	uint64_t current_time = ((uint64_t)gtime.tv_sec)*G_USEC_PER_SEC + ((uint64_t)gtime.tv_usec);
	uint64_t last_current_time = 0;

	cv::Mat image_left;
	cv::Mat image_right;

	bool found_correct_timestamp = (startTimestamp == 0);

	while(!mavlinkLog.eof())
	{
		bool il = false;
		bool ir = false;

		mavlinkLog.read((char *)buf, len);

		//first 8 bytes are the timestamp
		uint64_t time;
		memcpy((void*)&time, buf, sizeof(uint64_t));

		mavlink_message_t msg;
		memcpy(&(msg.magic), buf + sizeof(uint64_t), MAVLINK_MAX_PACKET_LEN);

		//the checksums follow directly after the payload, so copy them to their fields in mavlink_message_t
		//msg.ck_a = *(sizeof(uint64_t) + buf + msg.len + MAVLINK_CORE_HEADER_LEN + 1);
		//msg.ck_b = *(sizeof(uint64_t) + buf + msg.len + MAVLINK_CORE_HEADER_LEN + 2);

		//printf("%llu\n", time);

		//check for image triggered message, load the image and put it to the shared memory
		if (do_images && msg.msgid == MAVLINK_MSG_ID_IMAGE_TRIGGERED)
		{
			//printf("image triggered: %llu\n", (long long unsigned) time);
			std::map<uint64_t,std::string>::iterator it;
			mavlink_image_triggered_t itrg;
			mavlink_msg_image_triggered_decode(&msg, &itrg);

			if(itrg.timestamp >= startTimestamp)
			{
				if (found_correct_timestamp == false)
				{
					printf("Found starting timestamp, start replaying now from here...\n");
					found_correct_timestamp = true;
				}

				it = images_left.find(itrg.timestamp);
				if (it == images_left.end())
				{
					// Image not found
					if (verbose) printf("Have triggering message, but missing LEFT frame for timestamp %lu!\n", itrg.timestamp);
				}
				else
				{
					// Image found
					if (verbose) printf("[%llu] loading left image %s\n", (long long unsigned) camid_left, it->second.c_str());
					image_left = cv::imread(it->second.c_str(), -1);
					il = true;
				}

				if (do_stereo)
				{
					it = images_right.find(itrg.timestamp);
					if (it == images_right.end())
					{
						// Image not found
						if (verbose) printf("Have triggering message and stereo ON, but missing RIGHT frame for timestamp %lu!\n", itrg.timestamp);
					}
					else
					{
						// Image found
						if (verbose) printf("[%llu] loading right image %s\n", (long long unsigned) camid_right, it->second.c_str());
						image_right = cv::imread(it->second.c_str(), -1);
						ir = true;
					}
				}

				//publish images (write to shared memory and send message on IMAGES channel)
				if(il)
				{
					if(ir)
					{
						cam.writeStereoImage(image_left, camid_left, image_right, camid_right, itrg.timestamp, itrg, 0);
					}
					else
					{
						cam.writeMonoImage(image_left, camid_left, itrg.timestamp, itrg, 0);
					}
				}
			}
		}
		else if (do_images)
		{
			if (( (images_left.empty() || sync_image_left_it != images_left.end()) && (images_right.empty() ||  sync_image_right_it != images_right.end()) && !(images_right.empty() && images_left.empty())) &&
				(sync_image_left_it->first >= startTimestamp || sync_image_right_it->first >= startTimestamp))
			{
				if (found_correct_timestamp == false)
				{
					printf("Found starting timestamp, start replaying now from here...\n");
					found_correct_timestamp = true;
				}

				uint64_t timestamp = sync_image_left_it->first;
				if (sync_image_left_it->first < time)
				{
					// Image found
					if (verbose) printf("[%llu] loading left image %s\n", (long long unsigned) camid_left, sync_image_left_it->second.c_str());
					image_left = cv::imread(sync_image_left_it->second.c_str(), -1);
					il = true;
					++sync_image_left_it;
				}

				if (do_stereo)
				{
					if (sync_image_right_it->first < time)
					{
						// Image found
						if (verbose) printf("[%llu] loading right image %s\n", (long long unsigned) camid_right, sync_image_right_it->second.c_str());
						image_right = cv::imread(sync_image_right_it->second.c_str(), -1);
						ir = true;
						++sync_image_right_it;
					}
				}

				//publish images (write to shared memory and send message on IMAGES channel)
				if(il)
				{
					mavlink_image_triggered_t itrg;
					if(ir)
					{
						cam.writeStereoImage(image_left, camid_left, image_right, camid_right, timestamp, itrg, 0);
					}
					else
					{
						cam.writeMonoImage(image_left, camid_left, timestamp, itrg, 0);
					}
				}
			}
			else
			{
				if (!images_left.empty())
					++sync_image_left_it;
				if (!images_right.empty())
					++sync_image_right_it;
			}
		}

		//don't publish IMAGE_AVAILABLE messages or if we did not find the right timestamped image yet
		if (msg.msgid != MAVLINK_MSG_ID_IMAGE_AVAILABLE && found_correct_timestamp)
		{
			uint64_t usecs_to_wait = 0;
			uint64_t usecs_gone = current_time - last_current_time;
			if (last_time > 0)
			{
				usecs_to_wait = time-last_time;
				if (usecs_gone < usecs_to_wait)
				{
					usleep(usecs_to_wait - usecs_gone);
				}
			}

			if (msg.msgid == MAVLINK_MSG_ID_EXTENDED_MESSAGE)
			{
				// Pack a new container
				mavconn_mavlink_msg_container_t container;
				container.link_component_id = 0;
				container.link_network_source = MAVCONN_LINK_TYPE_LCM;

				memcpy(&(container.msg), &msg, sizeof(container.msg));

				// read extended header
				uint8_t* payload = reinterpret_cast<uint8_t*>(msg.payload64);

				unsigned int extended_payload_len;
				memcpy(&extended_payload_len, payload + 3, 4);

				container.extended_payload_len = extended_payload_len;
				container.extended_payload = new int8_t[extended_payload_len];
				mavlinkLog.read(reinterpret_cast<char*>(container.extended_payload),
								extended_payload_len);

				// Publish the message on the LCM bus
				if (publishExtended)
				{
					mavconn_mavlink_msg_container_t_publish(lcmMavlink, MAVLINK_MAIN, &container);
				}

				delete [] container.extended_payload;
			}
			else
			{
				sendMAVLinkMessage(lcmMavlink, &msg);
			}

			last_time = time;
			last_current_time = current_time;
		}
	}

	if (!found_correct_timestamp)
	{
		printf("Sorry, it seems that your given start timestamp was not found.\n");
	}

	lcm_destroy(lcmMavlink);

	if (do_images)
	{
		lcm_destroy(lcmImage);
	}

	return EXIT_SUCCESS;
}
