#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtptimeutilities.h"
#include "rtppacket.h"
#include <stdlib.h>
#include <iostream>

#include "h264.h"
#include "RTPsender.h"
CRTPSender::CRTPSender(void)
{
}


CRTPSender::~CRTPSender(void)
{
}

void CRTPSender::OnAPPPacket(RTCPAPPPacket *apppacket,const RTPTime &receivetime,const RTPAddress *senderaddress)
{//收到RTCP APP数据
    std::cout<<"Got RTCP packet from: "<<senderaddress<<std::endl;
   // std::cout<<"Got RTCP subtype: "<<apppacket->GetSubType()<<std::endl;
    //std::cout<<"Got RTCP data: "<<(char *)apppacket->GetAPPData()<<std::endl;
    return ;
}

void CRTPSender::SendH264Nalu(unsigned char* m_h264Buf,int buflen)
{
    unsigned char *pSendbuf; //发送数据指针
    pSendbuf = m_h264Buf;

    //去除前导码0x000001 或者0x00000001
//    if( 0x01 == m_h264Buf[2] )
//    {
//      pSendbuf = &m_h264Buf[3];
//      buflen -= 3;
//    }
//    else
//    {
//      pSendbuf = &m_h264Buf[4];
//      buflen -= 4;
//    }


    char sendbuf[1430];   //发送的数据缓冲
    memset(sendbuf,0,1430);

    int status;

    //printf("send packet length %d \n",buflen);

    if ( buflen <= MAX_RTP_PKT_LENGTH )
    {
        memcpy(sendbuf,pSendbuf,buflen);
        status = this->SendPacket((void *)sendbuf,buflen);

        CheckError(status);

    }
    else if(buflen > MAX_RTP_PKT_LENGTH)
    {
        //设置标志位Mark为0
        this->SetDefaultMark(false);
        //printf("buflen = %d\n",buflen);
        //得到该需要用多少长度为MAX_RTP_PKT_LENGTH字节的RTP包来发送
        int k=0,l=0;
        k = buflen / MAX_RTP_PKT_LENGTH;
        l = buflen % MAX_RTP_PKT_LENGTH;
        int t=0;//用指示当前发送的是第几个分片RTP包

        char nalHeader = pSendbuf[0]; // NALU 头ª¡¤
        while( t < k || ( t==k && l>0 ) )
        {
            if( (0 == t ) || ( t<k && 0!=t ) )//第一包到最后包的前一包
            {
                /*sendbuf[0] = (nalHeader & 0x60)|28;
                sendbuf[1] = (nalHeader & 0x1f);
                if ( 0 == t )
                {
                    sendbuf[1] |= 0x80;
                }
                memcpy(sendbuf+2,&pSendbuf[t*MAX_RTP_PKT_LENGTH],MAX_RTP_PKT_LENGTH);
                status = this->SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2);*/
                memcpy(sendbuf,&pSendbuf[t*MAX_RTP_PKT_LENGTH],MAX_RTP_PKT_LENGTH);
                status = this->SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH);
                CheckError(status);
                t++;
            }
            //最后一包
            else if( ( k==t && l>0 ) || ( t== (k-1) && l==0 ))
            {
                //设置标志位Mark为1
                this->SetDefaultMark(true);

                int iSendLen;
                if ( l > 0)
                {
                    iSendLen = buflen - t*MAX_RTP_PKT_LENGTH;
                }
                else
                    iSendLen = MAX_RTP_PKT_LENGTH;

                //sendbuf[0] = (nalHeader & 0x60)|28;
                //sendbuf[1] = (nalHeader & 0x1f);
                //sendbuf[1] |= 0x40;

                //memcpy(sendbuf+2,&pSendbuf[t*MAX_RTP_PKT_LENGTH],iSendLen);
                //status = this->SendPacket((void *)sendbuf,iSendLen+2);

                memcpy(sendbuf,&pSendbuf[t*MAX_RTP_PKT_LENGTH],iSendLen);
                status = this->SendPacket((void *)sendbuf,iSendLen);

                CheckError(status);
                t++;
            }
        }
    }
}


void CRTPSender::SetParamsForSendingH264()
{
    this->SetDefaultPayloadType(H264);//设置传输类型
    this->SetDefaultMark(true);      //设置位
    this->SetTimestampUnit(1.0/9000.0); //设置采样间隔
    this->SetDefaultTimestampIncrement(3600);//设置时间戳增加间隔
}

void CRTPSender::OnBYEPacket(RTPSourceData *srcdat)
{

}

void CRTPSender::OnBYETimeout(RTPSourceData *srcdat)
{

}

bool CheckError(int rtperr)
{
    if (rtperr < 0)
    {
        std::cout<<"ERROR: "<<RTPGetErrorString(rtperr)<<std::endl;
        return false;
    }
    return true;
}
