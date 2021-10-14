#ifndef _ATK1218_BD_H_
#define _ATK1218_BD_H_

#include "typedef.h"
#include "os/os_api.h"

#define TCFG_GPS_DEV_ENABLE  1
#if defined(TCFG_GPS_DEV_ENABLE) && TCFG_GPS_DEV_ENABLE

#define GPS_SET_BAUD         6     //设置GPS模块波特率0~8 (目前0,1,5无法初始化)
#define GPS_SET_FREP         5     //设置GPS模块更新速率（1,2,4,5,8,10,20hz）
#define GPS_SET_PPS_WIDTH    100000   //设置GPS模块PPS脉冲宽度1~100000(us)

#define _PACKED __attribute__((packed))
//GPS 北斗 NMEA-0183协议
//卫星信息
typedef struct {
    u8 num;		//卫星编号
    u8 eledeg;	//卫星仰角
    u16 azideg;	//卫星方位角
    u8 sn;		//信噪比
} _PACKED nmea_slmsg_t;
//UTC时间信息
typedef struct {
    u16 year;	//年份
    u8 month;	//月份
    u8 date;	//日期
    u8 hour; 	//小时
    u8 min; 	//分钟
    u8 sec; 	//秒钟
} _PACKED nmea_utc_time_t;
//NMEA 0183 协议解析后数据存放结构体
typedef struct {
    u8 svnum;					//可见GPS卫星数
    u8 beidou_svnum;					//可见GPS卫星数
    nmea_slmsg_t slmsg[12];		//最多12颗GPS卫星
    nmea_slmsg_t beidou_slmsg[12];		//最多12颗北斗卫星
    nmea_utc_time_t utc;			//UTC时间
    u32 latitude;				//纬度 分扩大100000倍,实际要除以100000
    u8 nshemi;					//北纬/南纬,N:北纬;S:南纬
    u32 longitude;			    //经度 分扩大100000倍,实际要除以100000
    u8 ewhemi;					//东经/西经,E:东经;W:西经
    u8 gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.
    u8 posslnum;				//用于定位的GPS卫星数,0~12.
    u8 possl[12];				//用于定位的卫星编号
    u8 fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
    u16 pdop;					//位置精度因子 0~500,对应实际值0~50.0
    u16 hdop;					//水平精度因子 0~500,对应实际值0~50.0
    u16 vdop;					//垂直精度因子 0~500,对应实际值0~50.0

    int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m
    u32 speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
} _PACKED nmea_msg_t;

//SkyTra S1216F8 配置波特率结构体
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0004；
    u8 id;             //ID，固定为0X05
    u8 com_port;       //COM口，固定为0X00，即COM1
    u8 baud_id;       //波特率（0~8,4800,9600,19200,38400,57600,115200,230400,460800,921600）
    u8 attributes;     //配置数据保存位置 ,0保存到SRAM，1保存到SRAM&FLASH
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_baudrate_t;

//SkyTra S1216F8 配置输出信息结构体
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0009；
    u8 id;             //ID，固定为0X08
    u8 gga;            //1~255（s）,0:disable
    u8 gsa;            //1~255（s）,0:disable
    u8 gsv;            //1~255（s）,0:disable
    u8 gll;            //1~255（s）,0:disable
    u8 rmc;            //1~255（s）,0:disable
    u8 vtg;            //1~255（s）,0:disable
    u8 zda;            //1~255（s）,0:disable
    u8 attributes;     //配置数据保存位置 ,0保存到SRAM，1保存到SRAM&FLASH
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_outmsg_t;

//SkyTra S1216F8 配置位置更新率结构体
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0003；
    u8 id;             //ID，固定为0X0E
    u8 rate;           //取值范围:1, 2, 4, 5, 8, 10, 20, 25, 40, 50
    u8 attributes;     //配置数据保存位置 ,0保存到SRAM，1保存到SRAM&FLASH
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_posrate_t;

//SkyTra S1216F8 配置输出脉冲(PPS)宽度结构体
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0007；
    u8 id;             //ID，固定为0X65
    u8 sub_id;         //0X01
    u32 width;        //1~100000(us)
    u8 attributes;     //配置数据保存位置 ,0保存到SRAM，1保存到SRAM&FLASH
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_pps_width_t;

//SkyTra S1216F8 ACK结构体
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0002；
    u8 id;             //ID，固定为0X83
    u8 ack_id;         //ACK ID may further consist of message ID and message sub-ID which will become 3 bytes of ACK message
    u8 cs;             //校验值
    u16 end;            //结束符
} _PACKED skytra_ack_t;

//SkyTra S1216F8 NACK结构体
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0002；
    u8 id;             //ID，固定为0X84
    u8 nack_id;         //ACK ID may further consist of message ID and message sub-ID which will become 3 bytes of ACK message
    u8 cs;             //校验值
    u16 end;            //结束符
} _PACKED skytra_nack_t;

//SkyTra S1216F8 restart结构体
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X000F；
    u8 id;             //ID，固定为0X01
    u8 start_mode;             //0~4.  0:no change 1:hot start 2:warm start 3:cold start 4:test mode
    u16 utc_year;             //年，需大于1980
    u8 utc_month;             //月，1~12
    u8 utc_day;             //日，1~31
    u8 utc_hour;             //时，0~23
    u8 utc_minu;             //分，0~59
    u8 utc_sec;             //秒，0~59
    u16 gps_latitude;             //维度：-9000~9000    >0: North Hemisphere; <0: South Hemisphere
    u16 gps_longitude;             //经度：-18000~18000 >0: East Hemisphere; <0: West Hemisphere
    u16 gps_altitude;             //高度：-1000~18300
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_restart_t;

//SkyTra S1216F8 配置NMEA消息输出间隔结构体 (600us ack)
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X000F；
    u8 id;              //ID，固定为0X64
    u8 sub_id;          //ID，固定为0X02
    u8 gga_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 gsa_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 gsv_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 gll_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 rmc_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 vtg_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 zda_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 gns_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 gbs_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 grs_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 dtm_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 gst_interval;    //0~255.  0:disable 1~255:输出间隔(单位：s)
    u8 attributes;     //配置数据保存位置 ,0保存到SRAM，1保存到SRAM&FLASH
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_message_interval_t;

//SkyTra S1216F8 查询NMEA消息输出间隔结构体 (400us ack)
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0002
    u8 id;              //ID，固定为0X64
    u8 sub_id;          //ID，固定为0X03
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_get_message_interval_t;

//SkyTra S1216F8 配置消息类型结构体 (65ms ack)
typedef struct {
    u16 sos;            //启动序列，固定为0XA0A1
    u16 pl;             //有效数据长度0X0003
    u8 id;              //ID，固定为0X09
    u8 type;          //00:no output; 01:NMEA message; 02:binary message
    u8 attributes;     //配置数据保存位置 ,0保存到SRAM，1保存到SRAM&FLASH
    u8 cs;             //校验值
    u16 end;            //结束符:0X0D0A
} _PACKED skytra_message_type_t;






u8 skytra_cfg_message_interval();//配置NMEA消息输出间隔
u8 skytra_cfg_message_type(u8 type);//配置输出消息类型

void uart_gps_init();
void get_gps_message();

// void uart1_readgps_test();
#endif

#endif
