#ifndef _MAX30102_H_
#define _MAX30102_H_

#include "typedef.h"
#include "os/os_api.h"

#include "system/includes.h"
#include "media/includes.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "asm/timer.h"


#define TCFG_MAX30102_DEV_ENABLE   1
#define TCFG_MAX30102_USE_IIC_TYPE 0

#if TCFG_MAX30102_DEV_ENABLE


#define MAX30102_RADDR        0xAF
#define MAX30102_WADDR        0xAE
//register addresses
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
// #define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
// #define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

struct _max30102_dev_platform_data {
    u8 iic_hdl;
    u8 iic_delay;          //这个延时并非影响iic的时钟频率，而是2Byte数据之间的延时
};

bool max30102_init(void *priv);
bool max30102_read_fifo(u32 *read_red_led, u32 *read_ir_led);
bool max30102_power_control(u8 shutdown_en);//1:power save; 0:normal

// void max_init_read_test();
// void cal_heart_sp02_data_test1();
// void cal_heart_sp02_data_test2();


#endif

#endif
