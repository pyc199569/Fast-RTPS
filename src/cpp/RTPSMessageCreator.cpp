/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * CDRMessageCreator.cpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/RTPSMessageCreator.h"
#include "eprosimartps/CDRMessage.h"

#include "eprosimartps/dds/ParameterList.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace boost::posix_time;



using namespace eprosima::dds;

namespace eprosima {
namespace rtps{

// Auxiliary message to avoid creation of new messages each time.
ObjectPool<CDRMessage_t> g_pool_submsg(100);
const boost::posix_time::ptime t_epoch(boost::gregorian::date(1900,1,1),boost::posix_time::time_duration(0,0,0));



RTPSMessageCreator::RTPSMessageCreator() {
	// TODO Auto-generated constructor stub


}

RTPSMessageCreator::~RTPSMessageCreator() {
	// TODO Auto-generated destructor stub
}


bool RTPSMessageCreator::addHeader(CDRMessage_t*msg, GuidPrefix_t& guidPrefix,
		ProtocolVersion_t version,VendorId_t vendorId)
{

	try{
		CDRMessage::addOctet(msg,'R');
		CDRMessage::addOctet(msg,'T');
		CDRMessage::addOctet(msg,'P');
		CDRMessage::addOctet(msg,'S');

		CDRMessage::addOctet(msg,version.major);
		CDRMessage::addOctet(msg,version.minor);

		CDRMessage::addOctet(msg,vendorId[0]);
		CDRMessage::addOctet(msg,vendorId[1]);

		for (uint i = 0;i<12;i++){
			CDRMessage::addOctet(msg,guidPrefix.value[i]);
		}
		msg->length = msg->pos;
	}
	catch(int e)
	{
		pError("Header creation fails."<<endl);
		return false;
	}

	return true;
}

bool RTPSMessageCreator::addHeader(CDRMessage_t*msg, GuidPrefix_t& guidPrefix)
{
	ProtocolVersion_t prot;
	PROTOCOLVERSION(prot);
	VendorId_t vend;
	VENDORID_EPROSIMA(vend);
	return RTPSMessageCreator::addHeader(msg,guidPrefix,prot,vend);
}


bool RTPSMessageCreator::addSubmessageHeader(CDRMessage_t* msg,
		octet id,octet flags,uint16_t size) {

	try{
		CDRMessage::addOctet(msg,id);
		CDRMessage::addOctet(msg,flags);
		CDRMessage::addUInt16(msg, size);
		msg->length = msg->pos;
	}
	catch(int e){

		pError("Submessage Header creation fails."<<endl);
		return false;
	}

	return true;
}

bool RTPSMessageCreator::addSubmessageInfoTS(CDRMessage_t* msg,Time_t& time,bool invalidateFlag)
{
	octet flags = 0x0;
	uint16_t size = 8;
	if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		flags = flags | BIT(0);
		msg->msg_endian  = LITTLEEND;
	}
	else
	{
		msg->msg_endian = BIGEND;
	}
	if(invalidateFlag)
	{
		flags = flags | BIT(1);
		size = 0;
	}
	try{
		CDRMessage::addOctet(msg,INFO_TS);
		CDRMessage::addOctet(msg,flags);
		CDRMessage::addUInt16(msg, size);
		if(!invalidateFlag)
		{
			CDRMessage::addInt32(msg,time.seconds);
			CDRMessage::addUInt32(msg,time.fraction);
		}
	}
	catch(int e)
	{
		pError("Submessage Header creation fails."<<endl);
		return false;
	}
	return true;
}

bool RTPSMessageCreator::addSubmessageInfoTS_Now(CDRMessage_t* msg,bool invalidateFlag)
{
	boost::posix_time::ptime boost_time_now= microsec_clock::local_time();
	Time_t time_now;
	time_now.seconds = (int32_t)(boost_time_now-t_epoch).total_seconds();
	time_now.fraction = (boost_time_now-t_epoch).fractional_seconds()*(int32_t)(pow(2,32)*pow(10,-boost::posix_time::time_duration::num_fractional_digits()));
//	cout << t << endl;
//	cout << (t-t_epoch) << endl;
//	cout << time_now.seconds << endl;
//	cout << time_now.fraction << endl;
	return RTPSMessageCreator::addSubmessageInfoTS(msg,time_now,invalidateFlag);
}

}; /* namespace rtps */
}; /* namespace eprosima */


#include "submessages/DataMsg.hpp"
#include "submessages/HeartbeatMsg.hpp"
#include "submessages/AckNackMsg.hpp"
#include "submessages/GapMsg.hpp"
