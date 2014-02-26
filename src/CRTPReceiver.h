/*
 * CRTPReceiver.h
 *
 *  Created on: 2014-2-22
 *      Author: xy
 */

#ifndef CRTPRECEIVER_H_
#define CRTPRECEIVER_H_

#include <rtpsession.h>

using namespace jrtplib;
class CRTPReceiver : public RTPSession
{
protected:
    void OnPollThreadStep();
    void ProcessRTPPacket(const RTPSourceData &srcdat,const RTPPacket &rtppack);
    void OnRTCPCompoundPacket(RTCPCompoundPacket *pack,const RTPTime &receivetime,const RTPAddress *senderaddress);

public:
    CThreadSafeArray m_ReceiveArray;
    void InitBufferSize();

private:
    CVideoData* m_pVideoData;
    unsigned char m_buffer[BUFFER_SIZE];
    int m_current_size;
};

#endif /* CRTPRECEIVER_H_ */
