//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// 
// server.c
// This is a udp server of gateway. Receive GPS information and Send Command to 
// handring.
// Copyright by shiziliang
// 
// Compile: g++ server.c -o server -levent  -L/usr/local/lib -Wl,-rpath=/usr/local/lib $(pkg-config --cflags --libs libmongoc-1.0)
// Execture: ./server 
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
#define MAX_STANDBY 6 

int g_count = 0;
struct timeval stTv;


//
// mongodb 
//
mongoc_client_t *client;
mongoc_collection_t *collection;
mongoc_collection_t *collection_cmd;
mongoc_collection_t *collection_state;

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

void sent_time_response( int x, unsigned char * aRespBuffer , unsigned char *  terminal_id,
             struct sockaddr_in stFromAddr, unsigned int unFromAddrLen )
{
    time_t nowtime;
    struct tm *timeinfo;
    int nSendByte = 0;

    time( &nowtime );
    timeinfo = gmtime( &nowtime );

    data_response_time * response = (data_response_time *)aRespBuffer;
    response->data_head[0] = 0x29;
    response->data_head[1] = 0x29;
    response->cmd = 0xE8;
    response->length = htons(16);
    memcpy(response->terminal_id, terminal_id, sizeof(response->terminal_id) );
    response->parameter = 0x01;

    response->timestamp[0] = DEC_TO_BCD(timeinfo->tm_year-100);  
    response->timestamp[1] = DEC_TO_BCD(1+timeinfo->tm_mon);  
    response->timestamp[2] = DEC_TO_BCD(timeinfo->tm_mday);  
    response->timestamp[3] = DEC_TO_BCD(timeinfo->tm_hour);  
    response->timestamp[4] = DEC_TO_BCD(timeinfo->tm_min);  
    response->timestamp[5] = DEC_TO_BCD(timeinfo->tm_sec);  


    response->data_tail.xor_check = check_xor(aRespBuffer,16);
    response->data_tail.tail = 0x0d;
    nSendByte = 18;
    if (( sendto(x, aRespBuffer, nSendByte, 0,
        (struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
    {
        printf("error occured while receivingn");
    }

}

void send_sleep_cmd(int x, unsigned char * aRespBuffer , unsigned char * terminal_id,
             struct sockaddr_in stFromAddr, unsigned int unFromAddrLen )
{
    int nSendByte = 0;

    data_response_alert * response = (data_response_alert *)aRespBuffer;
    response->data_head[0] = 0x29;
    response->data_head[1] = 0x29;
    response->cmd = 0x38;
    response->length = htons(9);
    memcpy(response->terminal_id, terminal_id, sizeof(response->terminal_id) );
    response->data_tail.xor_check = check_xor((unsigned char *)response,9);
    response->data_tail.tail = 0x0d;
    nSendByte = 11;


    if (( sendto(x, aRespBuffer, nSendByte, 0,
        (struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
    {
        printf("error occured while receivingn");
    }

}

void send_query_cmd(int x, unsigned char * aRespBuffer , unsigned char * terminal_id,
             struct sockaddr_in stFromAddr, unsigned int unFromAddrLen )
{
    int nSendByte = 0;

    data_response_alert * response = (data_response_alert *)aRespBuffer;
    response->data_head[0] = 0x29;
    response->data_head[1] = 0x29;
    response->cmd = 0x3d;
    response->length = htons(9);
    memcpy(response->terminal_id, terminal_id, sizeof(response->terminal_id) );
    response->data_tail.xor_check = check_xor((unsigned char *)response,9);
    response->data_tail.tail = 0x0d;
    nSendByte = 11;


    if (( sendto(x, aRespBuffer, nSendByte, 0,
        (struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
    {
        printf("error occured while receivingn");
    }

}


void send_monitor_cmd(int x, unsigned char * aRespBuffer , unsigned char * terminal_id,
             struct sockaddr_in stFromAddr, unsigned int unFromAddrLen ,
             int monitor_type, char *telephone)
{
    int nSendByte = 0;
    unsigned char  tel[7] ;

    memset(tel, 0xff, sizeof(tel));

    if( monitor_type == 1) {
        tel[0] = 0x01;
    }else {
        tel[0] = 0x00;
    }

    telephone2bcd( telephone ,  tel + 1);

    data_cmd_monitor * response = (data_cmd_monitor *)aRespBuffer;
    response->data_head[0] = 0x29;
    response->data_head[1] = 0x29;
    response->cmd = 0x3E;
    response->length = htons(18);
    memcpy(response->terminal_id, terminal_id, sizeof(response->terminal_id) );
    memcpy(response->tel , tel, 7 );
    response->data_tail.xor_check = check_xor((unsigned char *)response,16);
    response->data_tail.tail = 0x0d;
    nSendByte = 18;


    if (( sendto(x, aRespBuffer, nSendByte, 0,
        (struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
    {
        printf("error occured while receivingn");
    }

}

void send_set_cmd(int x, unsigned char * aRespBuffer , unsigned char * terminal_id,
            struct sockaddr_in stFromAddr, unsigned int unFromAddrLen ,char * cmd)
{
    int nSendByte = 0;
    
    int buffer_length;
    head * response = (head *)aRespBuffer;
    response->data_head[0] = 0x29;
    response->data_head[1] = 0x29;
    response->cmd = 0x3A;
    buffer_length = sizeof(head) + strlen(cmd);
    response->length = htons(buffer_length);

    memcpy(response->terminal_id, terminal_id, sizeof(response->terminal_id) );

    strcpy( aRespBuffer + sizeof(head), cmd);

    data_frame_tail * tail = (data_frame_tail *) (aRespBuffer + sizeof(head) + strlen(cmd));

    tail->xor_check = check_xor((unsigned char *)response,buffer_length);
    tail->tail = 0x0d;


    nSendByte = buffer_length + 2;

    if (( sendto(x, aRespBuffer, nSendByte, 0,
        (struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
    {
        printf("error occured while receivingn");
    }
}

void send_22_response(int x, unsigned char * aRespBuffer , unsigned short flow_id,
            unsigned char xor , unsigned char data_type,
            struct sockaddr_in stFromAddr, unsigned int unFromAddrLen )
{
    int nSendByte = 0;

    data_response_gps * response = (data_response_gps *)aRespBuffer;
    response->data_head[0] = 0x29;
    response->data_head[1] = 0x29;
    response->cmd = 0x22;
    response->length = htons(7);
    response->flow_id = flow_id;
    response->check = xor;
    response->cmd_b2 = 0xb2;
    response->sub_cmd = data_type;
    response->data_tail.xor_check = check_xor((unsigned char *)response,7);
    response->data_tail.tail = 0x0d;
    nSendByte = sizeof(data_response_gps);

    if (( sendto(x, aRespBuffer, nSendByte, 0,
        (struct sockaddr *)&stFromAddr, unFromAddrLen)) == -1)
    {
        printf("error occured while receivingn");
    }
}
int check_state(unsigned char * terminal_id)
{

    bson_oid_t oid;
    bson_t *doc = NULL;
    bson_t *update = NULL;
    bson_t *query = NULL;
    int count;
    int found = 0;

 
    found = mongoc_collection_count (collection_state, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

    if (found == 0) {
        doc = bson_new ();
        bson_oid_init (&oid, NULL);
        BSON_APPEND_INT32 (doc, "id", get_product_id(terminal_id));
        BSON_APPEND_INT32 (doc, "count", 0);

        if (!mongoc_collection_insert (collection_state, MONGOC_INSERT_NONE, doc, NULL, &error)) {
            printf ("%s\n", error.message);
        }

    }

    query = bson_new ();
    BSON_APPEND_INT32 (query, "id", get_product_id(terminal_id));

    cursor = mongoc_collection_find (collection_state, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    //while (mongoc_cursor_next (cursor, &doc)) {
    //   mongoc_cursor_next (cursor, &result);
    while (mongoc_cursor_next (cursor, (const  bson_t **) &doc)) {
        bson_iter_t iter;
        bson_iter_t sub_iter;
        // str = bson_as_json (doc, NULL);
        // fprintf (stderr, "%s\n", str);
        // bson_free (str);

        if (bson_iter_init (&iter, doc) && bson_iter_find_descendant (&iter, "count", &sub_iter)) {
            // fprintf (stderr,"Found key \"%s\" in sub document.\n", bson_iter_key (&sub_iter));
            // printf ("The type of a.b.c.d is: %d\n", (int)bson_iter_type (&sub_iter));
            count = (int)bson_iter_int32 (&sub_iter);
        }
    }
    fprintf (stderr,"Found count %d .\n", count);

    count++;
    
    if (count > MAX_STANDBY) 
    {
        count = 1;
        update = bson_new ();
        BSON_APPEND_INT32 (update, "id",get_product_id(terminal_id));
        BSON_APPEND_INT32 (update, "count", count);


        if (!mongoc_collection_update (collection_state, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
            printf ("%s\n", error.message);
        }
    }
    else
    {
        update = bson_new ();
        BSON_APPEND_INT32 (update, "id",get_product_id(terminal_id));
        BSON_APPEND_INT32 (update, "count", count);

        if (!mongoc_collection_update (collection_state, MONGOC_UPDATE_NONE, query, update, NULL, &error)) {
            printf ("%s\n", error.message);
        }
    }

    return count;


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
    const bson_t *result;

    bson_t *query;
    int i = 0;
    int command = 0;
    int monitor_type = 0;
    char telephone[10];
    char text_message[240];
    int length;

    char strBaseinfo[10];
    AGPS_info * agps;
    cell_info * cell;

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
    unsigned char xor = *(aReqBuffer + nByte - 2);

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
            fprintf(stderr,"length %d \n",   ntohs(msg->length) );
            //fprintf(stderr,"product_id %u\n",get_product_id(msg->terminal_id));
            fprintf(stderr,"product_id %u\n",get_product_id(g->terminal_id));
            fprintf(stderr,"flow_id %x \n",   ntohs(g->flow_id) );
            fprintf(stderr,"data_type %x \n",   g->data_type );
            fprintf(stderr," time " );

            for (  i = 0 ; i< 6 ;i++)
                fprintf(stderr," %x \n",   g->base_info.date_time[i] );
            /////////
            //经纬度
            fprintf(stderr," latitude " );
            for (  i = 0 ; i< 4 ;i++)
                fprintf(stderr,"%x \n",   g->base_info.latitude[i] );
            fprintf(stderr," longitude " );
            for (  i = 0 ; i< 4 ;i++)
                fprintf(stderr," %x \n",   g->base_info.longitude[i] );
            /////////
            fprintf(stderr,"speed %u \n", ntohs(g->base_info.speed));
            fprintf(stderr,"fangxiang %u \n", ntohs(g->base_info.direction));
            fprintf(stderr,"gaodu %u\n", ntohs(g->base_info.high));
            fprintf(stderr,"定位状态 %c\n", g->base_info.pos_station);
            fprintf(stderr,"是否应答 %x\n", g->base_info.need_response);

            if (ntohs(msg->length) > sizeof (content_gps_up)) {
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
                        agps = (AGPS_info * ) ( attach_info + 2);
                        cell = (cell_info * ) ( attach_info + 2 + sizeof(AGPS_info));
                        fprintf(stderr,"\n国家编码");

                        fprintf(stderr," %x ", ntohl(agps->country_code) );
                        fprintf(stderr,"\n运营编码");
                        
                        fprintf(stderr," %x ", ntohl(agps->mobile_code) );
                        fprintf(stderr,"\n运基站定位\n");

                        for (  i = 0 ; i< agps->base_count ;i++){
                            fprintf(stderr," 基站 %d 位置区编码 %d\n", i+1, ntohs(cell->lacN) );
                            fprintf(stderr," 基站 %d 小区编码 %d \n", i+1, ntohs(cell->cellN) );
                            fprintf(stderr," 基站 %d 信号强度 %x \n", i+1, cell->signalN );
                            cell++;
                        }
                        break;


                    case 0x0C:
                    case 0x1E:
                    case 0x1F:
                    case 0x28:
                    case 0x40:
                    case 0x41:
                    case 0x42:
                    case 0x43:
                    case 0x44:
                    case 0x45:

                        break;

                }

            }

            if( g->data_type == 0x80 || g->data_type == 0x82 || g->data_type == 0x8E )
            {

                //保存mongodb
                unsigned char timegps[15];
                unsigned char latitude[10];
                unsigned char longitude[10];
                double lat;
                double lng;
                doc = bson_new ();
                bson_oid_init (&oid, NULL);
                BSON_APPEND_OID (doc, "_id", &oid);
                BSON_APPEND_INT32 (doc, "id",get_product_id(g->terminal_id));

                get_gps_time(g->base_info.date_time, timegps );
                
                BSON_APPEND_UTF8 (doc, "timestamp", timegps);
                
                lat = bcd2longitude(g->base_info.latitude, latitude );
                lng = bcd2longitude(g->base_info.longitude, longitude );
               
                BSON_APPEND_UTF8 (doc, "latitude", latitude);
                BSON_APPEND_UTF8 (doc, "longitude", longitude);
                BSON_APPEND_DOUBLE (doc, "lat", lat);
                BSON_APPEND_DOUBLE (doc, "lng", lng);
                BSON_APPEND_INT32 (doc, "speed", ntohs(g->base_info.speed));
                BSON_APPEND_INT32 (doc, "direction", ntohs(g->base_info.direction));
                BSON_APPEND_INT32 (doc, "high", ntohs(g->base_info.high));
                BSON_APPEND_INT32 (doc, "pos_station", g->base_info.pos_station);

               if (ntohs(msg->length) > sizeof (content_gps_up)) {
                    unsigned char *attach_info = aReqBuffer + sizeof(data_frame_head) + sizeof(content_gps_up);
                    
                    switch (*(attach_info))
                    {
                        case 0x01:
                            break;
                        case 0x02:
                            agps = (AGPS_info * ) ( attach_info + 2);
                            cell = (cell_info * ) ( attach_info + 2 + sizeof(AGPS_info));
                            

                            fprintf(stderr,"\n国家编码");

                            fprintf(stderr," %x ", ntohl(agps->country_code) );
                            BSON_APPEND_INT32 (doc, "country_code", ntohl(agps->country_code));

                            fprintf(stderr,"\n运营编码");
                            
                            fprintf(stderr," %x ", ntohl(agps->mobile_code) );
                            BSON_APPEND_INT32 (doc, "mobile_code", ntohl(agps->mobile_code));

                            fprintf(stderr,"\n运基站定位\n");

                            for (  i = 0 ; i< agps->base_count ;i++){
                                fprintf(stderr," 基站 %d 位置区编码 %d\n", i+1, ntohs(cell->lacN) );
                                fprintf(stderr," 基站 %d 小区编码 %d \n", i+1, ntohs(cell->cellN) );
                                fprintf(stderr," 基站 %d 信号强度 %x \n", i+1, cell->signalN );
                                sprintf(strBaseinfo,"lac%d",i);
                                BSON_APPEND_INT32 (doc, strBaseinfo, ntohs(cell->lacN) );
                                sprintf(strBaseinfo,"cell%d",i);
                                BSON_APPEND_INT32 (doc, strBaseinfo, ntohs(cell->cellN) );
                                sprintf(strBaseinfo,"signal%d",i);
                                BSON_APPEND_INT32 (doc, strBaseinfo, cell->signalN );

                                cell++;
                            }
                            break;


                        case 0x0C:
                        case 0x1E:
                        case 0x1F:
                        case 0x28:
                        case 0x40:
                        case 0x41:
                        case 0x42:
                        case 0x43:
                        case 0x44:
                        case 0x45:

                            break;

                    }

                }

                if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
                    printf ("Insert failed: %s\n", error.message);
                }

                bson_destroy (doc);
            }
            else if( g->data_type == 0x85 )
            {

                //delete from mongo
                query = bson_new ();
                BSON_APPEND_INT32 (query, "id",get_product_id(g->terminal_id));

                if( ! mongoc_collection_remove(collection_cmd, MONGOC_DELETE_NONE, query, NULL, &error))
                {
                    fprintf(stderr,"remove command failed \n"   );
                }
                bson_destroy (query);

            }
            
            if (g->base_info.need_response == 0x01)
            {
                send_22_response( x,  aRespBuffer ,  g->flow_id, xor ,  g->data_type,
                          stFromAddr,  unFromAddrLen );
            }
            

            
            break;

        case CMD_TERMINAL_INFO_UP :
            fprintf(stderr,"CMD_TERMINAL_INFO_UP\n");
            content_info_up * t = (content_info_up *)(aReqBuffer + sizeof(data_frame_head));
            fprintf(stderr,"parameter %x \n",   t->parameter );

            if(t->parameter == 0x01)
            {

                sent_time_response( x, aRespBuffer , g->terminal_id, stFromAddr, unFromAddrLen );
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


    //get command from mongo
    char *str;
    query = bson_new ();
    BSON_APPEND_INT32 (query, "id",get_product_id(g->terminal_id));



    cursor = mongoc_collection_find (collection_cmd, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

    //while (mongoc_cursor_next (cursor, &doc)) {
    //   mongoc_cursor_next (cursor, &result);
    while (mongoc_cursor_next (cursor, (const  bson_t **) &doc)) {
        bson_iter_t iter;
        bson_iter_t sub_iter;
        str = bson_as_json (doc, NULL);
        fprintf (stderr, "%s\n", str);
        bson_free (str);

        if (bson_iter_init (&iter, doc) && bson_iter_find_descendant (&iter, "cmd", &sub_iter)) {
            fprintf (stderr,"Found key \"%s\" in sub document.\n", bson_iter_key (&sub_iter));
            printf ("The type of a.b.c.d is: %d\n", (int)bson_iter_type (&sub_iter));
            command = (int)bson_iter_int32 (&sub_iter);
        }
        if (bson_iter_init (&iter, doc) && bson_iter_find_descendant (&iter, "parameter", &sub_iter)) {
            fprintf (stderr,"Found key \"%s\" in sub document.\n", bson_iter_key (&sub_iter));
            printf ("The type of a.b.c.d is: %d\n", (int)bson_iter_type (&sub_iter));
            strcpy (telephone , bson_iter_utf8 (&sub_iter,&length));
        }
        if (bson_iter_init (&iter, doc) && bson_iter_find_descendant (&iter, "monitor_type", &sub_iter)) {
            fprintf (stderr,"Found key \"%s\" in sub document.\n", bson_iter_key (&sub_iter));
            printf ("The type of a.b.c.d is: %d\n", (int)bson_iter_type (&sub_iter));
            monitor_type = (int)bson_iter_int32 (&sub_iter);
        }

        if (bson_iter_init (&iter, doc) && bson_iter_find_descendant (&iter, "text_message", &sub_iter)) {
            fprintf (stderr,"Found key \"%s\" in sub document.\n", bson_iter_key (&sub_iter));
            printf ("The type of a.b.c.d is: %d\n", (int)bson_iter_type (&sub_iter));
            strcpy (text_message , bson_iter_utf8 (&sub_iter,&length));
        }
    }
    mongoc_cursor_destroy (cursor);

    bson_destroy (query);

    //send command to handring

    if (command == 1)
    {
        send_monitor_cmd( x,  aRespBuffer , g->terminal_id, stFromAddr,  unFromAddrLen ,monitor_type, telephone);
    }
    else if (command == 9)
    {
        send_sleep_cmd( x,  aRespBuffer , g->terminal_id, stFromAddr,  unFromAddrLen );
    }
    else if (command == 2)
    {
        send_set_cmd( x,  aRespBuffer , g->terminal_id, stFromAddr,  unFromAddrLen ,text_message);
    }
    else{
        //为了避免长时间使用网络。在通信6次发送一个休眠指令
        if( (msg->cmd == CMD_TERMINAL_GPS_UP) && (check_state(g->terminal_id) == MAX_STANDBY) )
        {
            send_sleep_cmd( x,  aRespBuffer , g->terminal_id, stFromAddr,  unFromAddrLen );
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
    if (!client) {
        fprintf (stderr, "Failed to parse URI.\n");
        return EXIT_FAILURE;
    }

    collection = mongoc_client_get_collection (client, "test", "gps");
    collection_cmd = mongoc_client_get_collection (client, "test", "cmd");
    collection_state = mongoc_client_get_collection (client, "test", "state");


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
    mongoc_collection_destroy (collection_cmd);
    mongoc_collection_destroy (collection_state);
   
    mongoc_client_destroy (client);

    return 0;
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
