#ifndef _PROTOCAL2929_H
#define _PROTOCAL2929_H

//数据类型
#define BYTE   (unsigned char)  //无符号单字节整数(字节 8bit)
#define WORD   (unsigned short) //无符号双字节整数(字 16bit)
#define DWORD  (unsigned int)   //无符号四字节整数(双字 32bit)
#define BCD    (char *)          //8421 BCD 码
#define STRING (char *)          //字符串,如无说明 GBK 编码


#define PROTOCAL_HEAD 0x2929
#define PROTOCAL_TAIL 0x0D



//protocal

enum protocal_cmd_up
{
	CMD_TERMINAL_GPS_UP = 0xB2,        //终端位置信息上行
	CMD_TERMINAL_INFO_UP = 0xB0,       //终端请求平台信息上行(0xB0)
	CMD_VERSION_INFO_UP = 0x84,       //终端版本信息上行(0x84)
	CMD_VOICE_UP = 0x9C       //终端主发语音上行(0x9C)
};

enum protocal_cmd_down
{
	CMD_RESPONSE = 0x21,        //平台通用应答下行(B2 外其他需要应答指令的应答)
	CMD_RESPONSE_GPS = 0x22,       //平台针对 B2 位置信息应答下行(0x22)
	CMD_RESPONSE_DATA = 0xE8,        //平台回复终端请求数据下行(0xE8)
	CMD_RESPONSE_POINT_DATA = 0x30,       //平台点名数据下行(0x30)
	CMD_VERSION_QUERY = 0x3D,        //平台版本信息查询下行(0x3D)
	CMD_SET_TEXT = 0x3A,        //平台设置指令及文本下行(0x3A)
	CMD_PHONE_LISTEN = 0x3E,       //电话监听下行(0x3E)
	CMD_CANCEL_ALERT = 0x37,       //远程取消报警下行(主要是取消 SOS 报警)(0x37)
	CMD_SET_SLEEP = 0x38,       //通知设备进入休眠模式(0x38)
	CMD_SET_ALERT = 0x39,       //通知设备振动及蜂鸣(0x39)
	CMD_WALL_CIRCLE_UPDATE = 0x46,       //圆形电子围栏更新下行(0x46)
	CMD_WALL_CIRCLE_CLEAR = 0x47,       //圆形电子围栏清除下行(0x47)
	CMD_WALL_CIRCLE_QUERY = 0x48,       //圆形电子围栏查询下行(0x48)
	CMD_WALL_CIRCLE_INFO_UP = 0x9A,       //圆形电子围栏查询信息上行(0x9A)
	CMD_WALL_GRID_UPDATE = 0x49,       //多边形电子围栏更新下行(0x49)
	CMD_WALL_GRID_CLEAR = 0x4A,       //多边形电子围栏清除下行(0x4a)
	CMD_WALL_GRID_QUERY = 0x4B,       //多边形电子围栏查询下行(0x4b)
	CMD_WALL_GRID_INFO_UP = 0x9D,       //多边形电子围栏查询信息上行(0x9D)
	CMD_VOICE_DOWN = 0x9B	       //手机主发语音下行(0x9B)

};


#pragma pack(1)
typedef struct {
  unsigned char data_head[2];
  unsigned char cmd;
  unsigned short length;
} data_frame_head;

typedef struct {
  unsigned char data_head[2];
  unsigned char cmd;
  unsigned short length;
  unsigned char terminal_id[4];

} head;


typedef struct 
{ 
  unsigned char xor_check;
  unsigned char tail;

} data_frame_tail;


#pragma pack()

unsigned int get_product_id(unsigned char * terminal_id );
unsigned char * get_terminal_id( unsigned int product_id );

//终端位置信息上行
enum data_type
{
	data_type_80 = 0x80,    //:实时位置数据上传 
	data_type_81 = 0x81,    // 点名应答 
	data_type_82 = 0x82,    //:事件位置数据上传 
	data_type_85 = 0x85,    // 参数设置等应答 
	data_type_8E = 0x8E    //:补传位置信息上传
};
//位置信息 44byte
#pragma pack(1)
typedef struct {
  unsigned char date_time[6]; //bcd code
  unsigned char latitude[4];
  unsigned char longitude[4];
  unsigned short speed;
  unsigned short direction;
  unsigned short high;
  unsigned char pos_station;
  unsigned char wire_station;
  unsigned char gps_satellite;
  unsigned char power_station;
  unsigned int power_v;
  unsigned char battery_percent;
  unsigned char mileage[4];
  unsigned char step[4];
  unsigned char st[4];
  unsigned char gsm;
  unsigned char need_response;
  unsigned char response_answer_cmd;
} gps_info;

typedef struct {
  unsigned int country_code;
  unsigned int mobile_code;
  unsigned char base_count;
} AGPS_info;

typedef struct {
  unsigned short lacN;
  unsigned short cellN;
  unsigned char signalN;
} cell_info;



typedef struct {
  unsigned char terminal_id[4];
  unsigned short flow_id;
  unsigned char data_type;
  gps_info base_info;
  //unsigned char * attach_data;
} content_gps_up;


typedef struct {
  unsigned char terminal_id[4];
  unsigned char parameter;

} content_info_up;



//平台通用应答下行(B2 外其他需要应答指令的应答)(0x21)

typedef struct {
  unsigned char data_head[2];
  unsigned char cmd;
  unsigned short length;
  unsigned short flow_id;
  unsigned char check;
  unsigned char cmd_b2;
  unsigned char sub_cmd;
  data_frame_tail data_tail;

} data_general_response;




// 平台针对B2位置信息应答下行（0x22）

typedef struct {
  unsigned char data_head[2];
  unsigned char cmd;
  unsigned short length;
  unsigned short flow_id;
  unsigned char check;
  unsigned char cmd_b2;
  unsigned char sub_cmd;
  data_frame_tail data_tail;
} data_response_gps;



 //终端请求平台信息上行(0xB0)

typedef struct {
  data_frame_head head;
  unsigned char content_parameter;
  data_frame_tail tail;
} data_info_up;


//平台回复终端请求数据下行(0xE8)
typedef struct {
  unsigned char data_head[2];
  unsigned char cmd;
  unsigned short length;
  unsigned char terminal_id[4];
  unsigned char parameter;
  unsigned char timestamp[6];
  data_frame_tail data_tail;
} data_response_time;




//平台点名数据下行(0x30)

//终端版本信息上行(0x84)

typedef struct {
  data_frame_head head;
  unsigned char *content_parameter;
  data_frame_tail tail;
} data_version_up;

//平台版本信息查询下行(0x3D)

//平台设置指令及文本下行(0x3A)


//通知设备振动及蜂鸣(0x39)
typedef struct {
  unsigned char data_head[2];
  unsigned char cmd;
  unsigned short length;
  unsigned char terminal_id[4];
  data_frame_tail data_tail;
} data_response_alert;


// (0x3e)
typedef struct {
  unsigned char data_head[2];
  unsigned char cmd;
  unsigned short length;
  unsigned char terminal_id[4];
  unsigned char tel[7];
  data_frame_tail data_tail;
} data_cmd_monitor;


#pragma pack()

unsigned int get_product_id(unsigned char * terminal_id );
unsigned char * get_terminal_id( unsigned int product_id );
unsigned char check_xor(unsigned char * message,int length);
double bcd2longitude(unsigned char* bcd, unsigned char* longitude_latitude);

double bcd2longitude1(unsigned char* bcd);
unsigned char * get_gps_time(unsigned char* bcd, unsigned char* timegps);
void telephone2bcd( char* bcd, unsigned char *tel);


#endif