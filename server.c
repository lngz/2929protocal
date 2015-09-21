//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// 
// server.c
// This is a udp server of gateway. Receive GPS information and Send Command to 
// handring.
// 
// Compile: g++ event_test.c -o event_test -levent  -L/usr/local/lib -Wl,-rpath=/usr/local/lib $(pkg-config --cflags --libs libmongoc-1.0)
// Execture: ./event_test 
// 
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define UDP_PORT 7212 
#include <event.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <bson.h>
#include <mongoc.h>
#include <stdio.h>
#include <time.h>  

#include "protocal2929.h"
#define DEC_TO_BCD(x)  ((((((x))/10)<<4)|((x))%10))

int g_count = 0;
struct timeval stTv;


//
// mongodb 
//
mongoc_client_t *client;
mongoc_collection_t *collection;
mongoc_cursor_t *cursor;
bson_error_t error;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// TIMER FUNCTION
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void timer_function(int x, short int y, void *pargs)
{
	printf("Get %d Message in a 5 sec \n",g_count);
	g_count = 0 ;
	event_add((struct event*)pargs,&stTv);
}



//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Event Function
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void gps_gateway_process(int x, short int y, void *pargs)
{
	unsigned int unFromAddrLen;
	int nByte = 0;
	int nSendByte = 0;
	unsigned char aReqBuffer[512];
	unsigned char aRespBuffer[512];
	struct sockaddr_in stFromAddr;
	bson_oid_t oid;
	bson_t *doc;

	time_t nowtime;
	struct tm *timeinfo;

	int setsleep = 0;


	unFromAddrLen = sizeof(stFromAddr);
	memset(aReqBuffer,0x00,512);
	memset(aRespBuffer,0x00,512);

	if ((nByte = recvfrom(x, aReqBuffer, sizeof(aReqBuffer), 0,
		(struct sockaddr *)&stFromAddr, &unFromAddrLen)) == -1)
	{
		printf("error occured while receivingn");
	}

	//proc();
	data_frame_head *msg = (data_frame_head *) aReqBuffer;

	content_gps_up *g = (content_gps_up *)(aReqBuffer + sizeof(data_frame_head));

	//异或校验

	//异或校验
	unsigned char xor = *(aReqBuffer+ nByte - 2);

    if( xor != check_xor(aReqBuffer, nByte - 2))
    {
    	return ;//drop down BAD check xor.
    }

	switch (msg->cmd)
	{
		case CMD_TERMINAL_GPS_UP  :


			//解析位置信息，得到经纬度
			fprintf(stderr,"protocal head %x \n",msg->data_head[0] );
			fprintf(stderr,"cmd %x \n",       msg->cmd );
			fprintf(stderr,"length %d \n",   htons(msg->length) );
			//fprintf(stderr,"product_id %u\n",get_product_id(msg->terminal_id));
			fprintf(stderr,"product_id %u\n",get_product_id(g->terminal_id));
			fprintf(stderr,"flow_id %x \n",   htons(g->flow_id) );
			fprintf(stderr,"data_type %x \n",   g->data_type );
			fprintf(stderr," time " );

			for (int i = 0 ; i< 6 ;i++)
				fprintf(stderr," %x \n",   g->base_info.date_time[i] );
			/////////
			//经纬度
			fprintf(stderr," latitude " );
			for (int i = 0 ; i< 4 ;i++)
				fprintf(stderr,"%x \n",   g->base_info.latitude[i] );
			fprintf(stderr," longitude " );
			for (int i = 0 ; i< 4 ;i++)
				fprintf(stderr," %x \n",   g->base_info.longitude[i] );
			/////////
		    fprintf(stderr,"speed %u \n", htons(g->base_info.speed));
		    fprintf(stderr,"fangxiang %u \n", htons(g->base_info.direction));
		    fprintf(stderr,"gaodu %u\n", htons(g->base_info.high));
		    fprintf(stderr,"定位状态 %c\n", g->base_info.pos_station);

		    if (htons(msg->length) > sizeof (content_gps_up)) {
		    	fprintf(stderr,"含有附带信息\n");
		    	unsigned char *attach_info = aReqBuffer + sizeof(data_frame_head) + sizeof(content_gps_up);
		    	fprintf(stderr,"附带信息类型：%x\n",*attach_info);
		    	fprintf(stderr,"附带信息长度：%d\n",*(attach_info+1));
		    	
		    	switch (*(attach_info))
		    	{
		    		case 0x01:
		    			break;
		    		case 0x02:
		    			fprintf(stderr,"基站信息\n");

		    			break;

		    	}

		    }

		    if( g->data_type == 0x80 )
		    {

				//保存mongodb
				unsigned char timegps[15];
				unsigned char latitude[10];
				unsigned char longitude[10];
			    doc = bson_new ();
			    bson_oid_init (&oid, NULL);
			    BSON_APPEND_OID (doc, "_id", &oid);
			    BSON_APPEND_INT32 (doc, "id",get_product_id(g->terminal_id));

			    get_gps_time(g->base_info.date_time, timegps );
			    
			    BSON_APPEND_UTF8 (doc, "timestamp", timegps);
			    
			    bcd2longitude(g->base_info.latitude, latitude );
			    bcd2longitude(g->base_info.longitude, longitude );
			    BSON_APPEND_UTF8 (doc, "latitude", latitude);
			    BSON_APPEND_UTF8 (doc, "longitude", longitude);
			    BSON_APPEND_INT32 (doc, "speed",htons(g->base_info.speed));
			    BSON_APPEND_INT32 (doc, "direction",htons(g->base_info.direction));
			    BSON_APPEND_INT32 (doc, "high",htons(g->base_info.high));
			    BSON_APPEND_INT32 (doc, "pos_station", g->base_info.pos_station);

			    if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
			        printf ("Insert failed: %s\n", error.message);
			    }

			    bson_destroy (doc);
			}
			else if( g->data_type == 0x85 )
			{

			}
			
		    if (g->base_info.pos_station == 'A')
		    {
			    data_response_gps * response = (data_response_gps *)aRespBuffer;
			    response->data_head[0] = 0x29;
			    response->data_head[1] = 0x29;
			    response->cmd = 0x22;
			    response->length = htons(7);
			    response->flow_id = g->flow_id;
			    response->check = *(aReqBuffer+ nByte - 2);
			    response->cmd_b2 = 0xb2;
			    response->sub_cmd = g->data_type;
			    response->data_tail.xor_check = check_xor(response,7);
			    response->data_tail.date_tail = 0x0d;
			    nSendByte = sizeof(data_response_gps);
				
				if (( sendto(x, aRespBuffer, nSendByte, 0,
					(struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
				{
					printf("error occured while receivingn");
				}

				setsleep = 1;

			}

			
			break;

		case CMD_TERMINAL_INFO_UP :
			fprintf(stderr,"CMD_TERMINAL_INFO_UP\n");
			content_info_up *t = (content_info_up *)(aReqBuffer + sizeof(data_frame_head));
			fprintf(stderr,"parameter %x \n",   t->parameter );

			if(t->parameter == 0x01)
			{

				time( &nowtime );
				timeinfo = gmtime( &nowtime );

			    data_response_time * response = (data_response_time *)aRespBuffer;
			    response->data_head[0] = 0x29;
			    response->data_head[1] = 0x29;
			    response->cmd = 0xE8;
			    response->length = htons(16);
			    memcpy(response->terminal_id,t->terminal_id, sizeof(response->terminal_id) );
			    response->parameter = 0x01;

			   	response->timestamp[0] = DEC_TO_BCD(timeinfo->tm_year-100);  
				response->timestamp[1] = DEC_TO_BCD(1+timeinfo->tm_mon);  
				response->timestamp[2] = DEC_TO_BCD(timeinfo->tm_mday);  
				response->timestamp[3] = DEC_TO_BCD(timeinfo->tm_hour);  
				response->timestamp[4] = DEC_TO_BCD(timeinfo->tm_min);  
				response->timestamp[5] = DEC_TO_BCD(timeinfo->tm_sec);  


			    response->data_tail.xor_check = check_xor(response,16);
		    	response->data_tail.date_tail = 0x0d;
		    	nSendByte = 18;
			}
			if (( sendto(x, aRespBuffer, nSendByte, 0,
				(struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
			{
				printf("error occured while receivingn");
			}

			break;
		case CMD_VERSION_INFO_UP  :
			fprintf(stderr,"CMD_VERSION_INFO_UP");
			break;
		case CMD_VOICE_UP         :
			fprintf(stderr,"CMD_VOICE_UP");
			break;
		default:
			break;
	}






	if (setsleep == 1)
	{
		data_response_alert * response = (data_response_alert *)aRespBuffer;
		response->data_head[0] = 0x29;
		response->data_head[1] = 0x29;
		response->cmd = 0x38;
		response->length = htons(11);
		memcpy(response->terminal_id, g->terminal_id, sizeof(response->terminal_id) );
		response->data_tail.xor_check = check_xor(response,11);
		response->data_tail.date_tail = 0x0d;
		nSendByte = 11;


		if (( sendto(x, aRespBuffer, nSendByte, 0,
			(struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
		{
			printf("error occured while receivingn");
		}

	}
	//printf("Function called buffer is %sn",aReqBuffer);
	g_count++;

}


//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// MAIN
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
int main(int argc, char **argv)
{
	struct event_base *base ;
	struct event g_Timer;
	
				
	struct event g_eve;
	int udpsock_fd;
	struct sockaddr_in stAddr;


	base = event_init();

    mongoc_init ();

    client = mongoc_client_new ("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection (client, "test", "gps");


	if ((udpsock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("ERROR - unable to create socket:n");
		exit(-1);
	}

	//Start : Set flags in non-blocking mode
	int nReqFlags = fcntl(udpsock_fd, F_GETFL, 0);
	if (nReqFlags< 0)
	{
		printf("ERROR - cannot set socket options");
	}

	if (fcntl(udpsock_fd, F_SETFL, nReqFlags | O_NONBLOCK) < 0)
	{
		printf("ERROR - cannot set socket options");
	}
	// End: Set flags in non-blocking mode
	memset(&stAddr, 0, sizeof(struct sockaddr_in));
	//stAddr.sin_addr.s_addr = inet_addr("192.168.64.1555552");
	stAddr.sin_addr.s_addr = INADDR_ANY; //listening on local ip
	stAddr.sin_port = htons(UDP_PORT);
	stAddr.sin_family = AF_INET;


	int nOptVal = 1;
	if (setsockopt(udpsock_fd, SOL_SOCKET, SO_REUSEADDR,
		(const void *)&nOptVal, sizeof(nOptVal)))
	{
		printf("ERROR - socketOptions: Error at Setsockopt");

	}

	if (bind(udpsock_fd, (struct sockaddr *)&stAddr, sizeof(stAddr)) != 0)
	{
		printf("Error: Unable to bind the default IP n");
		exit(-1);
	}

	event_set(&g_eve, udpsock_fd, EV_READ | EV_PERSIST, gps_gateway_process, &g_eve);
	event_add(&g_eve, NULL);

	/////////////TIMER START/////////////////////////////////// 
	stTv.tv_sec = 5;
	stTv.tv_usec = 0;
	event_set(&g_Timer, -1, EV_TIMEOUT , timer_function, &g_Timer);
	event_add(&g_Timer, &stTv);
	////////////TIMER END/////////////////////////////////////

	event_base_dispatch(base);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);

	return 0;
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
