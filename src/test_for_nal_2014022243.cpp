/*
 * test_for_nal.cpp
 *
 *  Created on: 2014-2-21
 *      Author: xy
 */


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
typedef enum {
 NALU_TYPE_SLICE    = 1,
 NALU_TYPE_DPA      = 2,
 NALU_TYPE_DPB      = 3,
 NALU_TYPE_DPC      = 4,
 NALU_TYPE_IDR      = 5,
 NALU_TYPE_SEI      = 6,
 NALU_TYPE_SPS      = 7,
 NALU_TYPE_PPS      = 8,
 NALU_TYPE_AUD      = 9,
 NALU_TYPE_EOSEQ    = 10,
 NALU_TYPE_EOSTREAM = 11,
 NALU_TYPE_FILL     = 12,

} NaluType;
typedef enum {
 FrameT		= 1,
 FrameTypeSPS	= 2,
 FrameTypePPS	= 3,
 FrameTypeI		= 4,
 FrameTypeP		= 5,

} FrameType;

using namespace std;
FrameType find_head(unsigned char *Buffer);
int find_nal(unsigned char *f264, unsigned int size);
int main()
{
	FILE *f=fopen("/home/xy/1.264","rb");fseek(f,0,SEEK_END);long len=ftell(f);fseek(f,0,SEEK_SET);
	unsigned char *data=(unsigned char*)malloc(len+1);fread(data,1,len,f);fclose(f);

	find_nal(data,len);
	printf("%X %X %X %x %x\n",data[0],data[1],data[2],data[3],data[4]&NALU_TYPE_SPS);
	free(data);

}

int find_nal(unsigned char *f264, unsigned int size)
{
	unsigned char *p=f264;
	unsigned int len=0;
	FrameType ft;
	while(len<=size-5)
	{
#if 0
		if( (*(p+len) == 0) && (*(p+len + 1) == 0) \
				&& (*(p+len + 2) == 3))
		{
			if(*(p+len + 4)==1)
			{
				if (*(p+len + 5)&0x1f == NALU_TYPE_SPS)
				{ //ox67为 0110 0111(nal_unit_type为低5位，u(5)= 0 0111 = 7)
					cout << " SPS " ;
				}
				else if (*(p+len + 5)&0x1f  == NALU_TYPE_PPS)
				{ //ox68为 0110 1000 （nal_unit_type为低5位，u(5)= 0 1000 = 8）
					cout << " PPS ";
				}
				else if (*(p+len + 5)&0x1f  == NALU_TYPE_IDR)
				{ //ox65为 0110 0101 （nal_unit_type为低5位，u(5)= 0 0101 = 5）
					//return FrameTypeI;
					cout << " I1 " << endl;
				}
				else
				{ //0x41为0100 00001 (nal_ref_idc是参考级别，代表被其它帧参考情况，u(2)= 10 = 2; nal_unit_type为低5位，u(5)= 0 0001 = 1)
					//return FrameTypeP;
					cout << " P1 "  ;
				}
				//len+=5;
				//continue;
			}
		}
#endif


		if ((*(p+len) == 0) && (*(p+len+1) == 0) \
				&& (*(p+len + 2) == 0)	&& (*(p+len + 3) == 1)) //NAL头的0x00 00 00 01起始码
		{
			unsigned char a=7;
			printf(" %#X ",*(p+len+4)&0x1f);
			printf(" %d ", p[len + 4]&0x1f ==a);

			if (*(p+len + 4)&0x1f == 0b111/*NALU_TYPE_SPS*/)
			{ //ox67为 0110 0111(nal_unit_type为低5位，u(5)= 0 0111 = 7)
				cout << " SPS ";
			}
			else if ((*(p+len + 4))&0x1f  == 0x8/*NALU_TYPE_PPS*/)
			{ //ox68为 0110 1000 （nal_unit_type为低5位，u(5)= 0 1000 = 8）
				cout << " PPS ";
			}
			else if ((*(p+len + 4))&0x1f  == 5/*NALU_TYPE_IDR*/)
			{ //ox65为 0110 0101 （nal_unit_type为低5位，u(5)= 0 0101 = 5）
				//return FrameTypeI;
				cout << "I " << endl;
			}
			else
			{ //0x41为0100 00001 (nal_ref_idc是参考级别，代表被其它帧参考情况，u(2)= 10 = 2; nal_unit_type为低5位，u(5)= 0 0001 = 1)
				//return FrameTypeP;
				cout << "P "  ;
			}

			if ((*(p+len + 5) & 0x80) == 0x80)
			{
				//return FrameT;
			}
			//len+=4;
			//continue;
		}
		len++;
	}





}
FrameType find_head(unsigned char *Buffer)
{
	if ((*(Buffer) == 0) && (*(Buffer + 1) == 0) && (*(Buffer + 2) == 0)
			&& (*(Buffer + 3) == 1)) //NAL头的0x00 00 00 01起始码
			{
		if (*(Buffer + 4)&0x1f == NALU_TYPE_SPS) { //ox67为 0110 0111(nal_unit_type为低5位，u(5)= 0 0111 = 7)
			return FrameTypeSPS;
		} else if (*(Buffer + 4)&0x1f  == NALU_TYPE_PPS) { //ox68为 0110 1000 （nal_unit_type为低5位，u(5)= 0 1000 = 8）
			return FrameTypePPS;
		} else if (*(Buffer + 4)&0x1f  == NALU_TYPE_IDR) { //ox65为 0110 0101 （nal_unit_type为低5位，u(5)= 0 0101 = 5）
			return FrameTypeI;
		} else { //0x41为0100 00001 (nal_ref_idc是参考级别，代表被其它帧参考情况，u(2)= 10 = 2; nal_unit_type为低5位，u(5)= 0 0001 = 1)
			return FrameTypeP;
		}
		if ((*(Buffer + 5) & 0x80) == 0x80) {
			return FrameT;
		}
	}
}
