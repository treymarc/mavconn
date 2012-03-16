/*=====================================================================

MAVCONN Micro Air Vehicle Flying Robotics Toolkit
Please see our website at <http://MAVCONN.ethz.ch>

(c) 2009 MAVCONN PROJECT  <http://MAVCONN.ethz.ch>

This file is part of the MAVCONN project

    MAVCONN is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MAVCONN is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MAVCONN. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief MAVCONN Lightweight Robotics Middleware main header
 *          (C++ implementation)
 *
 *   @author Lionel Heng <hengli@inf.ethz.ch>
 *
 */

/** @addtogroup comm */
/*@{*/


#ifndef _MAVCONN_HPP_
#define _MAVCONN_HPP_

// MAVLINK message format includes
#include <mavlink.h>

#include "mavconn.h"

// LCM transport includes
#include <lcm/lcm-cpp.hpp>
#include "comm/lcm/mavlink_message_t.hpp"
#include "comm/lcm/mavlink_msg_container_t.hpp"

static inline void
copyMAVLinkMessage(mavlink_message_t& dst, const mavconn::mavlink_message_t& src);

static inline void
copyMAVLinkMessage(mavlink_message_t& dst, const mavconn::mavlink_message_t& src)
{
	dst.checksum = src.checksum;
        dst.magic = src.magic;
        dst.len = src.len;
        dst.seq = src.seq;
        dst.sysid = src.sysid;
        dst.compid = src.compid;
        dst.msgid = src.msgid;
	memcpy(dst.payload64, src.payload64, sizeof(int64_t) * 33);
}

static inline void
copyMAVLinkMessage(mavconn::mavlink_message_t& dst, const mavlink_message_t& src);

static inline void
copyMAVLinkMessage(mavconn::mavlink_message_t& dst, const mavlink_message_t& src)
{
	dst.checksum = src.checksum;
        dst.magic = src.magic;
        dst.len = src.len;
        dst.seq = src.seq;
        dst.sysid = src.sysid;
        dst.compid = src.compid;
        dst.msgid = src.msgid;
	memcpy(dst.payload64, src.payload64, sizeof(int64_t) * 33);
}

static inline void
sendMAVLinkMessage(lcm::LCM& lcm, const mavlink_message_t* msg, MAVCONN_LINK_TYPE link_type=MAVCONN_LINK_TYPE_LCM);

static inline void
sendMAVLinkMessage(lcm::LCM& lcm, const mavlink_message_t* msg, MAVCONN_LINK_TYPE link_type)
{
	// Pack a new container
	static mavconn::mavlink_msg_container_t container;
	container.link_component_id = 0;
	container.link_network_source = link_type;
	container.extended_payload_len = 0;
	copyMAVLinkMessage(container.msg, *msg);

	// Publish the message on the LCM bus
	lcm.publish(MAVLINK_MAIN, &container);
}

#ifdef PROTOBUF_FOUND
static inline void
sendMAVLinkExtendedMessage(lcm::LCM& lcm, const mavlink_extended_message_t* msg, MAVCONN_LINK_TYPE link_type=MAVCONN_LINK_TYPE_LCM);

static inline void
sendMAVLinkExtendedMessage(lcm::LCM& lcm, const mavlink_extended_message_t* msg, MAVCONN_LINK_TYPE link_type)
{
	// Pack a new container
	static mavconn::mavlink_msg_container_t container;
	container.link_component_id = 0;
	container.link_network_source = link_type;
	copyMAVLinkMessage(container.msg, msg->base_msg);
	container.extended_payload_len = msg->extended_payload_len;
	container.extended_payload.assign(msg->extended_payload, msg->extended_payload + msg->extended_payload_len);

	// Publish the message on the LCM bus
	lcm.publish(MAVLINK_MAIN, &container);
}

static inline void
sendMAVLinkExtendedMessage(lcm::LCM& lcm, const std::vector<mavlink_extended_message_t>& msg, MAVCONN_LINK_TYPE link_type=MAVCONN_LINK_TYPE_LCM);

static inline void
sendMAVLinkExtendedMessage(lcm::LCM& lcm, const std::vector<mavlink_extended_message_t>& msg, MAVCONN_LINK_TYPE link_type)
{
	for (size_t i = 0; i < msg.size(); ++i)
	{
		const mavlink_extended_message_t& fragment = msg.at(i);

		// Pack a new container
		static mavconn::mavlink_msg_container_t container;
		container.link_component_id = 0;
		container.link_network_source = link_type;
		copyMAVLinkMessage(container.msg, fragment.base_msg);
		container.extended_payload_len = fragment.extended_payload_len;
		container.extended_payload.assign(fragment.extended_payload, fragment.extended_payload + fragment.extended_payload_len);

		// Publish the message on the LCM bus
		lcm.publish(MAVLINK_MAIN, &container);
	}
}
#endif

static inline mavlink_message_t
getMAVLinkMessage(const mavconn::mavlink_msg_container_t* container)
{
	mavlink_message_t msg;

	copyMAVLinkMessage(msg, container->msg);

	return msg;
}

#ifdef PROTOBUF_FOUND
static inline mavlink_extended_message_t
getMAVLinkExtendedMsg(const mavconn::mavlink_msg_container_t* container)
{
	mavlink_extended_message_t msg;
	copyMAVLinkMessage(msg.base_msg, container->msg);
	msg.extended_payload_len = container->extended_payload_len;
	memcpy(msg.extended_payload, &(container->extended_payload[0]), container->extended_payload_len);

	return msg;
}
#endif

#endif

