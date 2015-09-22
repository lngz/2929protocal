#include <stdio.h>
#include "protocal2929.h"


unsigned int get_product_id(unsigned char * terminal_id )
{

	return terminal_id[0]*1000000 + (terminal_id[1]-0x80)*10000
	+ (terminal_id[2]-0x80)* 100 + terminal_id[3];

}

unsigned char * get_terminal_id( unsigned int product_id )
{
	static unsigned char s[9];
	unsigned int t1,t2,t3,t4;
	t1 = product_id / 1000000;
	t2 = (product_id /10000) % 100 + 0x80;
	t3 = (product_id /100) % 100 + 0x80;
	t4 = product_id % 100;
	sprintf(s,"%x%x%x%x",t1,t2,t3,t4);
	return s;
}


unsigned char check_xor(unsigned char * message,int length)
{
	unsigned char result;
	result = *message ^ *(message+1);
	int i;
	for ( i=2 ; i< length; i++)
	{
		result=result ^ *(message + i);
	}
	//printf("%x",result);
	return result;

}

//压缩BCD 经纬度转化


double bcd2longitude(unsigned char* bcd, unsigned char* longitude_latitude)
{
    int num = 0;
    double du;
    if (((*(bcd) & 0xF0) >>4) >= 8) 
    {
        num = (((*(bcd) & 0xF0) >>4) - 8) * 100;
        //值为负
    }
    else {
        num = (((*(bcd) & 0xF0) >>4) ) * 100;
    }
    num += (*bcd & 0x0F) * 10 + ((*(bcd+1) & 0xF0) >>4);
    int point = ((*(bcd+1) & 0x0F) *10000);
    point += ((*(bcd+2) & 0xF0) >>4) *1000;
    point += ((*(bcd+2) & 0x0F) *100);
    point += ((*(bcd+3) & 0xF0) >>4) *10;
    point += (*(bcd+3) & 0x0F);

    sprintf(longitude_latitude,"%d.%05d",num, point);

    du = (double)num + ((double)point)/1000/60;

    return du;
    
}



unsigned char * get_gps_time(unsigned char* bcd, unsigned char* timegps)
{
	


	sprintf(timegps,"20%02x%02x%02x%02x%02x%02x",
		*bcd, 
		*(bcd + 1),
		*(bcd + 2),
		*(bcd + 3),
		*(bcd + 4),
		*(bcd + 5)
		);
	return timegps;
}


#ifdef _MYDEBUG_
int main()
{
	/*
	测试数据
	*/

	unsigned char s[] = {
	0x29,0x29, 0xb2,
	0x00,
0x63,0x00, 0x80,0x80, 0x3e,0x00, 0x58, 0x80, 0x04,0x01, 0x01,0x01, 0x50, 0x32, 0x00,0x00,
0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00, 0x00, 0x00,0x00, 0x00,0x00, 0x47, 0x00, 0x05,0x00,
0x00,0x00, 0x0e,0xc2, 0x2e,0x00, 0x00, 0x00, 0x00,0xe5, 0x9f,0xf0, 0x18, 0x28, 0x00,0x00,
0x00,0x5f, 0x00,0x00, 0x02,0x2c, 0x00, 0x00, 0x01,0xcc, 0x00,0x00, 0x00, 0x00, 0x07,0x10,
0x2c,0x78, 0x86,0x91, 0x10,0x2c, 0x01, 0xca, 0x95,0x10, 0x2c,0x0e, 0xf4, 0x8d, 0x10,0x2c,
0x01,0xc9, 0x8c,0x10, 0x2c,0x0e, 0xf5, 0x8c, 0x10,0x2c, 0x01,0xc8, 0x86, 0x10, 0x2c,0x1f,
0xc5,0x81, 0xac,0x0d, 

	             	};

	//网络绑定操作



	//解析数据头
	data_frame_head *a = (data_frame_head *) s;

      

	//异或校验
    if(*(s+ sizeof(s) - 2) != check_xor(s, sizeof(s) - 2))
    {
    	return 1;
    }

	switch (a->cmd)
	{
		case CMD_TERMINAL_GPS_UP  :
			printf("CMD_TERMINAL_GPS_UP\n");

			//解析位置信息，得到经纬度
			printf("protocal head %x \n",a->data_head[0] );
			printf("cmd %x \n",       a->cmd );
			printf("length %d \n",   htons(a->length) );
			//printf("product_id %u\n",get_product_id(a->terminal_id));
			content_gps_up *g = (content_gps_up *)(s + sizeof(data_frame_head));
			printf("product_id %u\n",get_product_id(g->terminal_id));
			printf("flow_id %x \n",   htons(g->flow_id) );
			printf("data_type %x \n",   g->data_type );

			printf("time %s \n",   get_gps_time(g->base_info.date_time ));
			/////////
			//经纬度
			printf("jing wei du %s \n",   bcd2longitude(g->base_info.latitude ));
			printf("jing wei du %s \n",   bcd2longitude(g->base_info.longitude ));
			/////////
		    printf("speed %u \n", htons(g->base_info.speed));
		    printf("fangxiang %u \n", htons(g->base_info.direction));
		    printf("gaodu %u\n", htons(g->base_info.high));
		    printf("定位状态 %c\n", g->base_info.pos_station);

		    if (htons(a->length) > sizeof (content_gps_up)) {
		    	printf("含有附带信息\n");
		    	unsigned char *attach_info = s + sizeof(data_frame_head) + sizeof(content_gps_up);
		    	printf ( "附带信息类型：%x\n",*attach_info);
		    	printf ( "附带信息长度：%d\n",*(attach_info+1));
		    	
		    	switch (*(attach_info))
		    	{
		    		case 0x01:
		    			break;
		    		case 0x02:
		    			printf("基站信息\n");

		    			break;

		    	}

		    }
		    printf("%x\n", check_xor(s, sizeof(s) - 2));


		//  wire_station;
		//  gps_satellite;
		//  power_station;
		// power_v;
		//  battery_percent;
		//  mileage[4];
		//  step[4];
		//  st[4];
		//  gsm;
		//  need_response;
		//  response_answer_cmd;

		// 00 定位天线状态 正常                   
		// 06 卫星数             
		// 00 电源状态             
		// 0000 0ec2 电源电压              
		// 2e 百分比             
		// 00 0000 00 里程             
		// e5 9ff0 18 步数             
		// 28 0000 00              
		// 5b GSM信号百分比             
		// 00 是否需要平台应答             
		// 00 应答平台命令        



			//设置 id 和经纬度到mongodb



			break;

		case CMD_TERMINAL_INFO_UP :
			printf("CMD_TERMINAL_INFO_UP");
			break;
		case CMD_VERSION_INFO_UP  :
			printf("CMD_VERSION_INFO_UP");
			break;
		case CMD_VOICE_UP         :
			printf("CMD_VOICE_UP");
			break;
		default:
			break;
	}

	return 0;
}

#endif





