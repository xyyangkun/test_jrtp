/*
 * test_jrtp.cpp
 *
 *  Created on: 2014-2-19
 *      Author: xy
 */
#if 0
/**
 * @note x264的编码示例.
 * 使用x264的版本为libx264-115
 * 1. 示例是个死循环,会源源不断的编码,然后将数据写文件.
 * 2. 示例的行为是:编码1000帧后,取空编码缓冲区,然后循环执行这两步.
 * @author

 *gcc  -lx264

 **/
#include <cassert>
#include <iostream>
#include <string>
#include "stdint.h"
#include <string.h>
#include <stdio.h>
extern "C" {
#include "x264.h"
}
;
unsigned int g_uiPTSFactor = 0;
int iNal = 0;
x264_nal_t* pNals = NULL;
int encode(x264_t* p264, x264_picture_t* pIn, x264_picture_t* pOut);
int main(int argc, char** argv) {
	int iResult = 0;
	x264_t* pX264Handle = NULL;

	x264_param_t* pX264Param = new x264_param_t;
	assert(pX264Param);
//* 配置参数
//* 使用默认参数
	x264_param_default(pX264Param);
//* cpuFlags
	pX264Param->i_threads = X264_SYNC_LOOKAHEAD_AUTO; //* 取空缓冲区继续使用不死锁的保证.
//* video Properties
	pX264Param->i_width = 320; //* 宽度.
	pX264Param->i_height = 240; //* 高度
	pX264Param->i_frame_total = 0; //* 编码总帧数.不知道用0.
	pX264Param->i_keyint_max = 10;
//* bitstream parameters
	pX264Param->i_bframe = 5;
	pX264Param->b_open_gop = 0;
	pX264Param->i_bframe_pyramid = 0;
	pX264Param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

//* 宽高比,有效果,但不是想要的.
//pX264Param->vui.i_sar_width = 1080;
//pX264Param->vui.i_sar_height = 720;

//* Log
	pX264Param->i_log_level = X264_LOG_DEBUG;
//* Rate control Parameters
	pX264Param->rc.i_bitrate = 1024 * 10; //* 码率(比特率,单位Kbps)
//* muxing parameters
	pX264Param->i_fps_den = 1; //* 帧率分母
	pX264Param->i_fps_num = 25; //* 帧率分子
	pX264Param->i_timebase_den = pX264Param->i_fps_num;
	pX264Param->i_timebase_num = pX264Param->i_fps_den;

//* 设置Profile.使用MainProfile
	x264_param_apply_profile(pX264Param, x264_profile_names[1]);

//* 打开编码器句柄,通过x264_encoder_parameters得到设置给X264
//* 的参数.通过x264_encoder_reconfig更新X264的参数
	pX264Handle = x264_encoder_open(pX264Param);
	assert(pX264Handle);

//* 获取整个流的PPS和SPS,不需要可以不调用.
	iResult = x264_encoder_headers(pX264Handle, &pNals, &iNal);
	assert(iResult >= 0);
//* PPS SPS 总共只有36B,如何解析出来呢?
	for (int i = 0; i < iNal; ++i) {
		switch (pNals[i].i_type) {
		case NAL_SPS:
			break;
		case NAL_PPS:
			break;
		default:
			break;
		}
	}

//* 获取允许缓存的最大帧数.
	int iMaxFrames = x264_encoder_maximum_delayed_frames(pX264Handle);

//* 编码需要的参数.
	iNal = 0;
	pNals = NULL;
	x264_picture_t* pPicIn = new x264_picture_t;
	x264_picture_t* pPicOut = new x264_picture_t;

	x264_picture_init(pPicOut);
	x264_picture_alloc(pPicIn, X264_CSP_I420, pX264Param->i_width,
			pX264Param->i_height);
	pPicIn->img.i_csp = X264_CSP_I420;
	pPicIn->img.i_plane = 3;

//* 创建文件,用于存储编码数据
	FILE* pFile = fopen("agnt.264", "wb");
	assert(pFile);

//* 示例用编码数据.
	int iDataLen = pX264Param->i_width * pX264Param->i_height;
	uint8_t* data = new uint8_t[iDataLen];

	unsigned int uiComponent = 0;
	while (++uiComponent) {
//* 构建需要编码的源数据(YUV420色彩格式)
		memset(data, uiComponent, iDataLen);
		memcpy(pPicIn->img.plane[0], data, iDataLen);
		memcpy(pPicIn->img.plane[1], data, iDataLen / 4);
		memcpy(pPicIn->img.plane[2], data, iDataLen / 4);

		if (uiComponent <= 1000) {
			pPicIn->i_pts = uiComponent + g_uiPTSFactor * 1000;
			encode(pX264Handle, pPicIn, pPicOut);
		} else {
//* 将缓存的数据取出
			int iResult = encode(pX264Handle, NULL, pPicOut);
			if (0 == iResult) {
//break; //* 取空,跳出
				uiComponent = 0;
				++g_uiPTSFactor;

				/* {{ 这个解决不了取空缓冲区,再压缩无B帧的问题
				 x264_encoder_reconfig(pX264Handle, pX264Param);
				 x264_encoder_intra_refresh(pX264Handle);
				 //* }} */
			}
		}

//* 将编码数据写入文件.
		for (int i = 0; i < iNal; ++i) {
			fwrite(pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
		}
	}
//* 清除图像区域
	x264_picture_clean(pPicIn);
	x264_picture_clean(pPicOut);
//* 关闭编码器句柄
	x264_encoder_close(pX264Handle);
	pX264Handle = NULL;

	delete pPicIn;
	pPicIn = NULL;

	delete pPicOut;
	pPicOut = NULL;

	delete pX264Param;
	pX264Param = NULL;

	delete[] data;
	data = NULL;
	return 0;
}

int encode(x264_t* pX264Handle, x264_picture_t* pPicIn,
		x264_picture_t* pPicOut) {

	int iResult = 0;
	iResult = x264_encoder_encode(pX264Handle, &pNals, &iNal, pPicIn, pPicOut);
	if (0 == iResult) {
		std::cout << "编码成功,但被缓存了." << std::endl;
	} else if (iResult < 0) {
		std::cout << "编码出错" << std::endl;
	} else if (iResult > 0) {
		std::cout << "得到编码数据" << std::endl;
	}

	/* {{ 作用不明
	 unsigned char* pNal = NULL;
	 for (int i = 0;i < iNal; ++i)
	 {
	 int iData = 1024 * 32;
	 x264_nal_encode(pX264Handle, pNal,&pNals[i]);
	 }
	 //* }} */

//* 获取X264中缓冲帧数.
	int iFrames = x264_encoder_delayed_frames(pX264Handle);
	std::cout << "当前编码器中缓存数据:" << iFrames << "帧\n";
	return iFrames;
}

#endif




#if 1




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
#define SSRC           100

#define DEST_IP_STR   "127.0.0.1"
#define DEST_PORT     9000
#define BASE_PORT     2222


void SetRTPParams(CRTPSender& sess,uint32_t destip,uint16_t destport,uint16_t baseport);
int main(int argc, char** argv)
{
#if 0
	CRTPSender sender;
	string destip_str = "127.0.0.1";
	uint32_t dest_ip = inet_addr(destip_str.c_str());

	SetRTPParams(sender,dest_ip,DEST_PORT,BASE_PORT);
	sender.SetParamsForSendingH264();
#else
	RTPSession session;

	RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0/9000);
	sessionparams.SetUsePredefinedSSRC(true);  //设置使用预先定义的SSRC
	sessionparams.SetPredefinedSSRC(SSRC);     //定义SSRC

	RTPUDPv4TransmissionParams transparams;
	transparams.SetPortbase(8000);

	int status = session.Create(sessionparams,&transparams);
	if (status < 0)
	{
		std::cerr << RTPGetErrorString(status) << std::endl;
		exit(-1);
	}

	uint8_t localip[]={127,0,0,1};
	RTPIPv4Address addr(localip,9000);

	status = session.AddDestination(addr);
	if (status < 0)
	{
		std::cerr << RTPGetErrorString(status) << std::endl;
		exit(-1);
	}

	session.SetDefaultPayloadType(96);
	session.SetDefaultMark(false);
	session.SetDefaultTimestampIncrement(90000.0 /25.0);

	uint8_t silencebuffer[160];


	RTPTime delay(0.040);
	RTPTime starttime = RTPTime::CurrentTime();

#endif

	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	char sendbuf[1500];
	char* nalu_payload;
	unsigned int timestamp_increse=0,ts_current=0;

#define ddd
	//OpenBitstreamFile("agnt.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	OpenBitstreamFile("/home/xy/11.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	//OpenBitstreamFile("test.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	//OpenBitstreamFile("test22.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	//OpenBitstreamFile("avc.h264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	NALU_t *n;
	n = AllocNALU(8000000);//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针
	while(!feof(bits))
	{
		GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		dump(n);//输出NALU长度和TYPE

		//将编码数据写入文件t
		//fwrite(pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
		//发送编码文件
#if 0
		sender.SendH264Nalu(n->buf,n->len);
#else


		//	当一个NALU小于MAX_RTP_PKT_LENGTH字节的时候，采用一个单RTP包发送
			if(n->len<=MAX_RTP_PKT_LENGTH)
			{
				printf("ddd0\n");
				session.SetDefaultMark(true);
				//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
				nalu_hdr =(NALU_HEADER*)&sendbuf[0]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
				nalu_hdr->F=n->forbidden_bit;
				nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
				nalu_hdr->TYPE=n->nal_unit_type;

				nalu_payload=&sendbuf[1];//同理将sendbuf[13]赋给nalu_payload
				memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
				ts_current=ts_current+timestamp_increse;

				status = session.SendPacket((void *)sendbuf,n->len);
				CheckError(status);

			}
			else if(n->len>MAX_RTP_PKT_LENGTH)
			{
				//得到该nalu需要用多少长度为MAX_RTP_PKT_LENGTH字节的RTP包来发送
				int k=0,l=0;
				k=n->len/MAX_RTP_PKT_LENGTH;//需要k个MAX_RTP_PKT_LENGTH字节的RTP包
				l=n->len%MAX_RTP_PKT_LENGTH;//最后一个RTP包的需要装载的字节数
				int t=0;//用于指示当前发送的是第几个分片RTP包
				ts_current=ts_current+timestamp_increse;
				while(t<=k)
				{

					if(!t)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
					{
						printf("dddd1");
						memset(sendbuf,0,1500);
						session.SetDefaultMark(false);
						//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
						fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendbuf[1];
						fu_hdr->E=0;
						fu_hdr->R=0;
						fu_hdr->S=1;
						fu_hdr->TYPE=n->nal_unit_type;


						nalu_payload=&sendbuf[2];//同理将sendbuf[14]赋给nalu_payload
						memcpy(nalu_payload,n->buf+1,MAX_RTP_PKT_LENGTH);//去掉NALU头

						status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2);
						CheckError(status);

						t++;
					}
					//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
					else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过MAX_RTP_PKT_LENGTH字节（当l>1386时）。
					{
						printf("dddd3\n");
						memset(sendbuf,0,1500);
						session.SetDefaultMark(true);
						//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
						fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendbuf[1];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->TYPE=n->nal_unit_type;
						fu_hdr->E=1;

						memcpy(nalu_payload,n->buf+t*MAX_RTP_PKT_LENGTH+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。

						status = session.SendPacket((void *)sendbuf,l+1);
						CheckError(status);

						t++;
					//	Sleep(100);
					}
					else if(t<k&&0!=t)
					{
						printf("dddd2");
						memset(sendbuf,0,1500);
						session.SetDefaultMark(false);
						//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
						fu_ind =(FU_INDICATOR*)&sendbuf[0]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendbuf[1];
						//fu_hdr->E=0;
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->E=0;
						fu_hdr->TYPE=n->nal_unit_type;

						nalu_payload=&sendbuf[2];//同理将sendbuf[14]的地址赋给nalu_payload
						memcpy(nalu_payload,n->buf+t*MAX_RTP_PKT_LENGTH+1,MAX_RTP_PKT_LENGTH);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。

						status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2);
						CheckError(status);

						t++;
					}
				}
			}

#endif

#if 1
		session.BeginDataAccess();
		if (session.GotoFirstSource())
		{
			do
			{
				RTPPacket *packet;

				while ((packet = session.GetNextPacket()) != 0)
				{
					std::cout << "Got packet with "
					          << "extended sequence number "
					          << packet->GetExtendedSequenceNumber()
					          << " from SSRC " << packet->GetSSRC()
					          << std::endl;
					session.DeletePacket(packet);
				}
			} while (session.GotoNextSource());
		}
		session.EndDataAccess();
#endif
		RTPTime::Wait(delay);

		RTPTime t = RTPTime::CurrentTime();
		t -= starttime;
		if (t > RTPTime(60.0))
			break;
	}
	delay = RTPTime(10.0);
	session.BYEDestroy(delay,"Time's up",9);


	//一些清理工作…
}
void SetRTPParams(CRTPSender& sess,uint32_t destip,uint16_t destport,uint16_t baseport)
{
    int status;
    //RTP+RTCP库初始化SOCKET环境
    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;
    sessparams.SetOwnTimestampUnit(1.0/9000.0); //时间戳单位
    sessparams.SetAcceptOwnPackets(true);   //接收自己发送的数据包
    sessparams.SetUsePredefinedSSRC(true);  //设置使用预先定义的SSRC
    sessparams.SetPredefinedSSRC(SSRC);     //定义SSRC

    transparams.SetPortbase(baseport);

    status = sess.Create(sessparams,&transparams);
    CheckError(status);

    destip = ntohl(destip);
    RTPIPv4Address addr(destip,destport);
    status = sess.AddDestination(addr);
    CheckError(status);

    //为发送H264包设置参数
    //sess.SetParamsForSendingH264();

}

#endif








#if 0
#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtptimeutilities.h"
#include "rtppacket.h"
#include <stdlib.h>
#include <iostream>

using namespace jrtplib;

int main(void)
{
	RTPSession session;

	RTPSessionParams sessionparams;
	sessionparams.SetOwnTimestampUnit(1.0/25.0);

	RTPUDPv4TransmissionParams transparams;
	transparams.SetPortbase(8000);

	int status = session.Create(sessionparams,&transparams);
	if (status < 0)
	{
		std::cerr << RTPGetErrorString(status) << std::endl;
		exit(-1);
	}

	uint8_t localip[]={127,0,0,1};
	RTPIPv4Address addr(localip,9000);

	status = session.AddDestination(addr);
	if (status < 0)
	{
		std::cerr << RTPGetErrorString(status) << std::endl;
		exit(-1);
	}

	session.SetDefaultPayloadType(96);
	session.SetDefaultMark(false);
	session.SetDefaultTimestampIncrement(160);

	uint8_t silencebuffer[160];
	for (int i = 0 ; i < 160 ; i++)
		silencebuffer[i] = 128;

	RTPTime delay(0.040);
	RTPTime starttime = RTPTime::CurrentTime();

	bool done = false;
	while (!done)
	{
		status = session.SendPacket(silencebuffer,160);
		if (status < 0)
		{
			std::cerr << RTPGetErrorString(status) << std::endl;
			exit(-1);
		}

		session.BeginDataAccess();
		if (session.GotoFirstSource())
		{
			do
			{
				RTPPacket *packet;

				while ((packet = session.GetNextPacket()) != 0)
				{
					std::cout << "Got packet with "
					          << "extended sequence number "
					          << packet->GetExtendedSequenceNumber()
					          << " from SSRC " << packet->GetSSRC()
					          << std::endl;
					session.DeletePacket(packet);
				}
			} while (session.GotoNextSource());
		}
		session.EndDataAccess();

		RTPTime::Wait(delay);

		RTPTime t = RTPTime::CurrentTime();
		t -= starttime;
		if (t > RTPTime(60.0))
			done = true;
	}

	delay = RTPTime(10.0);
	session.BYEDestroy(delay,"Time's up",9);

#ifdef WIN32
	WSACleanup();
#endif // WIN32
	return 0;
}



#endif
