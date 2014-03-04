/*
 * test_jrtp.cpp
 *
 *  Created on: 2014-2-19
 *      Author: xy
 */

#include "rtpsession.h"
#include "rtpsessionparams.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtptimeutilities.h"
#include "rtppacket.h"
#include <stdlib.h>
#include <iostream>
#include "h264.h"
#define SSRC           100

#define DEST_IP_STR   "127.0.0.1"
#define DEST_PORT     9000
#define BASE_PORT     2222

using namespace jrtplib;

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
	sessionparams.SetOwnTimestampUnit(1.0/90000.0);

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
	OpenBitstreamFile("1.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	//OpenBitstreamFile("test.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	//OpenBitstreamFile("slamtv60.264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	//OpenBitstreamFile("avc.h264");//打开264文件，并将文件指针赋给bits,在此修改文件名实现打开别的264文件。
	NALU_t *n;
	n = AllocNALU(8000000);//为结构体nalu_t及其成员buf分配空间。返回值为指向nalu_t存储空间的指针
	bool start=false;
	while(!feof(bits))
	{
		int size=GetAnnexbNALU(n);//每执行一次，文件的指针指向本次找到的NALU的末尾，下一个位置即为下个NALU的起始码0x000001
		if(size<4)
		{
			printf("get nul error!\n");
			continue;
		}
		dump(n);//输出NALU长度和TYPE
		if(!start)
		{
			if(n->nal_unit_type==5||n->nal_unit_type==6||
					n->nal_unit_type==7||n->nal_unit_type==7)
			{
				printf("begin\n");
				start=true;
			}
		}
		//将编码数据写入文件t
		//fwrite(pNals[i].p_payload, 1, pNals[i].i_payload, pFile);
		//发送编码文件
#if 1
		//	当一个NALU小于MAX_RTP_PKT_LENGTH字节的时候，采用一个单RTP包发送
			if(n->len<=MAX_RTP_PKT_LENGTH)
			{
				//printf("ddd0\n");
				//session.SetDefaultMark(false);
				//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
				nalu_hdr =(NALU_HEADER*)&sendbuf[0]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
				nalu_hdr->F=n->forbidden_bit;
				nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
				nalu_hdr->TYPE=n->nal_unit_type;

				nalu_payload=&sendbuf[1];//同理将sendbuf[13]赋给nalu_payload
				memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
				ts_current=ts_current+timestamp_increse;

				//status = session.SendPacket((void *)sendbuf,n->len);
				if(n->nal_unit_type==1 || n->nal_unit_type==5)
				{
					status = session.SendPacket((void *)sendbuf,n->len,96,true,3600);
				}
				else
				{
						status = session.SendPacket((void *)sendbuf,n->len,96,true,0);\
						//如果是6,7类型的包，不应该延时；之前有停顿，原因这在这
						continue;
				}
				//发送RTP格式数据包并指定负载类型为96
				if (status < 0)
				{
					std::cerr << RTPGetErrorString(status) << std::endl;
					exit(-1);
				}

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
						//printf("dddd1");
						memset(sendbuf,0,1500);
						//session.SetDefaultMark(false);
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

						//status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2);
						status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2,96,false,0);
						if (status < 0)
						{
							std::cerr << RTPGetErrorString(status) << std::endl;
							exit(-1);
						}
						t++;
					}
					//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
					else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过MAX_RTP_PKT_LENGTH字节（当l>1386时）。
					{
						//printf("dddd3\n");
						memset(sendbuf,0,1500);
						//session.SetDefaultMark(true);
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
						nalu_payload=&sendbuf[2];//同理将sendbuf[14]赋给nalu_payload
						memcpy(nalu_payload,n->buf+t*MAX_RTP_PKT_LENGTH+1,l-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。

						//status = session.SendPacket((void *)sendbuf,l+1);
						status = session.SendPacket((void *)sendbuf,l+1,96,true,3600);
						if (status < 0)
						{
							std::cerr << RTPGetErrorString(status) << std::endl;
							exit(-1);
						}
						t++;
					//	Sleep(100);
					}
					else if(t<k&&0!=t)
					{
						//printf("dddd2");
						memset(sendbuf,0,1500);
						//session.SetDefaultMark(false);
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

						//status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2);
						status = session.SendPacket((void *)sendbuf,MAX_RTP_PKT_LENGTH+2,96,false,0);
						if (status < 0)
						{
							std::cerr << RTPGetErrorString(status) << std::endl;
							exit(-1);
						}
						t++;
					}
				}
			}

#endif

#if 0
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
	printf("over\n");
	delay = RTPTime(10.0);
	session.BYEDestroy(delay,"Time's up",9);


	//一些清理工作…
}







