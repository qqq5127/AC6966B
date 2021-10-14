#ifndef _BMP280_H_
#define _BMP280_H_

#include "typedef.h"
#include "os/os_api.h"

#include "system/includes.h"
#include "media/includes.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "asm/timer.h"

#define TCFG_BMP280_DEV_ENABLE    1
#define TCFG_BMP280_USER_IIC_TYPE 0

#if TCFG_BMP280_DEV_ENABLE

struct _bmp280_dev_platform_data {
    u8 comms;        //0:IIC;  1:SPI
    u8 iic_hdl;
    u8 iic_delay;          //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
};



#define BMP280_I2C_ADDR		 	0x76	// The BMP280 I2C address
#define BMP280_I2C_ALT_ADDR 	0x77	// The BMP280 I2C alternate address
#define BMP280_DEVICE_ID 				0x58	// The BMP280 device ID
#define BMP280_RESET_VALUE				0xB6	// The BMP280 reset code

#define BMP280_CHIPID_REG                    0xD0  /*Chip ID Register */
#define BMP280_RESET_REG                     0xE0  /*Softreset Register */
#define BMP280_STATUS_REG                    0xF3  /*Status Register */
#define BMP280_CTRLMEAS_REG                  0xF4  /*Ctrl Measure Register */
#define BMP280_CONFIG_REG                    0xF5  /*Configuration Register */
#define BMP280_PRESSURE_MSB_REG              0xF7  /*Pressure MSB Register */
#define BMP280_PRESSURE_LSB_REG              0xF8  /*Pressure LSB Register */
#define BMP280_PRESSURE_XLSB_REG             0xF9  /*Pressure XLSB Register */
#define BMP280_TEMPERATURE_MSB_REG           0xFA  /*Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG           0xFB  /*Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG          0xFC  /*Temperature XLSB Reg */
//状态寄存器转换标志
#define	BMP280_MEASURING					0x01
#define	BMP280_IM_UPDATE					0x08

/*calibration parameters */
#define BMP280_DIG_T1_LSB_REG                0x88
#define BMP280_DIG_T1_MSB_REG                0x89
#define BMP280_DIG_T2_LSB_REG                0x8A
#define BMP280_DIG_T2_MSB_REG                0x8B
#define BMP280_DIG_T3_LSB_REG                0x8C
#define BMP280_DIG_T3_MSB_REG                0x8D
#define BMP280_DIG_P1_LSB_REG                0x8E
#define BMP280_DIG_P1_MSB_REG                0x8F
#define BMP280_DIG_P2_LSB_REG                0x90
#define BMP280_DIG_P2_MSB_REG                0x91
#define BMP280_DIG_P3_LSB_REG                0x92
#define BMP280_DIG_P3_MSB_REG                0x93
#define BMP280_DIG_P4_LSB_REG                0x94
#define BMP280_DIG_P4_MSB_REG                0x95
#define BMP280_DIG_P5_LSB_REG                0x96
#define BMP280_DIG_P5_MSB_REG                0x97
#define BMP280_DIG_P6_LSB_REG                0x98
#define BMP280_DIG_P6_MSB_REG                0x99
#define BMP280_DIG_P7_LSB_REG                0x9A
#define BMP280_DIG_P7_MSB_REG                0x9B
#define BMP280_DIG_P8_LSB_REG                0x9C
#define BMP280_DIG_P8_MSB_REG                0x9D
#define BMP280_DIG_P9_LSB_REG                0x9E
#define BMP280_DIG_P9_MSB_REG                0x9F
/******************ctrl_meas reg************************/
//BMP工作模式  bit:1,0
typedef enum {
    BMP280_SLEEP_MODE = 0x0,
    BMP280_FORCED_MODE = 0x1,	//可以说0x2
    BMP280_NORMAL_MODE = 0x3
} BMP280_WORK_MODE;
//BMP压力过采样因子  bit:4,3,2
typedef enum {
    BMP280_P_MODE_SKIP = 0x0,	/*skipped*/
    BMP280_P_MODE_1,			/*x1*/
    BMP280_P_MODE_2,			/*x2*/
    BMP280_P_MODE_3,			/*x4*/
    BMP280_P_MODE_4,			/*x8*/
    BMP280_P_MODE_5			    /*x16*/
} BMP280_P_OVERSAMPLING;
//BMP温度过采样因子  bit:7,6,5
typedef enum {
    BMP280_T_MODE_SKIP = 0x0,	/*skipped*/
    BMP280_T_MODE_1,			/*x1*/
    BMP280_T_MODE_2,			/*x2*/
    BMP280_T_MODE_3,			/*x4*/
    BMP280_T_MODE_4,			/*x8*/
    BMP280_T_MODE_5			    /*x16*/
} BMP280_T_OVERSAMPLING;

/******************config reg************************/
//bit0:spi3w_en: 1:enable spi 3-wire
//IIR滤波器时间常数  bit:4,3,2
typedef enum {
    BMP280_FILTER_OFF = 0x0,	/*filter off*/
    BMP280_FILTER_MODE_1,		/*0.223*ODR*/	/*x2*/
    BMP280_FILTER_MODE_2,		/*0.092*ODR*/	/*x4*/
    BMP280_FILTER_MODE_3,		/*0.042*ODR*/	/*x8*/
    BMP280_FILTER_MODE_4		/*0.021*ODR*/	/*x16*/
} BMP280_FILTER_COEFFICIENT;
//保持时间  bit:7,6,5
typedef enum {
    BMP280_T_SB1 = 0x0,	    /*0.5ms*/
    BMP280_T_SB2,			/*62.5ms*/
    BMP280_T_SB3,			/*125ms*/
    BMP280_T_SB4,			/*250ms*/
    BMP280_T_SB5,			/*500ms*/
    BMP280_T_SB6,			/*1000ms*/
    BMP280_T_SB7,			/*2000ms*/
    BMP280_T_SB8,			/*4000ms*/
} BMP280_T_SB;


typedef struct {
    /* T1~P9 为补偿系数 */
    u16 t1;
    s16	t2;
    s16	t3;
    u16 p1;
    s16	p2;
    s16	p3;
    s16	p4;
    s16	p5;
    s16	p6;
    s16	p7;
    s16	p8;
    s16	p9;
} bmp280_params;


typedef struct {
    BMP280_T_OVERSAMPLING t_osample;
    BMP280_P_OVERSAMPLING p_osample;
    BMP280_WORK_MODE		workmode;
} bmp_oversample_mode;

typedef struct {
    BMP280_T_SB 				t_sb;
    BMP280_FILTER_COEFFICIENT 	filter_coefficient;
    u8				spi_en;
} bmp_config;


#define USE_FIXED_POINT_COMPENSATE  1//数据补偿算法选择:1:使用定点补偿 0:使用浮点补偿
#if USE_FIXED_POINT_COMPENSATE  //使用定点补偿
typedef u32 bmp_pressure_data;
typedef s32 bmp_temperature_data;
#else  //使用浮点补偿
typedef double bmp_pressure_data;
typedef double bmp_temperature_data;
#endif



bool bmp280_init(void *priv);
bool bmp280_set_temoversamp(bmp_oversample_mode *oversample_mode);
bool bmp280_set_standby_filter(bmp_config *bmp_config);
void bmp280_set_work_mode(BMP280_WORK_MODE workmode);
bool bmp280_reset();  //return 1:ok;   0:fail
void bmp280_get_temperature_and_pressure(bmp_temperature_data *temperature, bmp_pressure_data *pressure);
void bmp280_forced_mode_get_temperature_and_pressure(bmp_temperature_data *temperature, bmp_pressure_data *pressure);
float bmp280_get_altitude(bmp_temperature_data temperature, bmp_pressure_data pressure);

// void bmp280_init_read_test();

#endif
#endif
