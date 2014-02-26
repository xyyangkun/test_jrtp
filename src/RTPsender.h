/*
 * RTPsender.h
 *
 *  Created on: 2014-2-19
 *      Author: xy
 */

#ifndef RTPSENDER_H_
#define RTPSENDER_H_
#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtptimeutilities.h"
#include "rtppacket.h"
#include <stdlib.h>
#include <iostream>
using namespace std;
using namespace jrtplib;


#define H264               96

bool CheckError(int rtperr);


class CRTPSender:public RTPSession
{
public:
    CRTPSender(void);
    ~CRTPSender(void);

protected:
    void OnAPPPacket(RTCPAPPPacket *apppacket,const RTPTime &receivetime,const RTPAddress *senderaddress);
    void OnBYEPacket(RTPSourceData *srcdat);
    void OnBYETimeout(RTPSourceData *srcdat);
public:
    void SendH264Nalu(unsigned char* m_h264Buf,int buflen);
    void SetParamsForSendingH264();
};



#endif /* RTPSENDER_H_ */
