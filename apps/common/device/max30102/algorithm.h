#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include "typedef.h"
#include "os/os_api.h"

#include "system/includes.h"
#include "media/includes.h"
#include "asm/iic_hw.h"
#include "asm/iic_soft.h"
#include "asm/timer.h"

#define TCFG_MAX30102_DEV_ENABLE  1

#if TCFG_MAX30102_DEV_ENABLE

/*************algorithm 1****************/
typedef struct {
    int64_t RealNum;
    int64_t ImagNum;
} ComplexNum;
extern u32  RedBuffer[256];
extern u32  IrBuffer[256];
u16  MAX30102_GetHeartRate();
float MAX30102_GetSPO2(void);

/*************algorithm 2****************/
#define Calculate_type 0
#if Calculate_type
#define FS 100
#define BUFFER_SIZE  (FS* 5)
#define HR_FIFO_SIZE 7
#define MA4_SIZE  4 // DO NOT CHANGE
#define HAMMING_SIZE  5// DO NOT CHANGE

#else
#define FreqS 25    //sampling frequency   //50
#define BUFFER_SIZE (FreqS * 4)            //50 * 3
#define MA4_SIZE 4 // DONOT CHANGE
#endif
void max301x_heart_rate_and_oxygen_saturation(u32 *pun_ir_buffer,  int n_ir_buffer_length, u32 *pun_red_buffer, int *pn_spo2, char *pch_spo2_valid, int *pn_heart_rate, char  *pch_hr_valid);
int32_t calculate_average(int pn_data, char *pch_data_valid, u8 data_type);

#endif
#endif
