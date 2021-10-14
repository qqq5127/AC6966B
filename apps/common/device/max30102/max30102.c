#include "app_config.h"
#include "asm/clock.h"
#include "system/timer.h"
/* #include "asm/uart_dev.h" */
#include "max30102.h"
#include "algorithm.h"
#include "asm/cpu.h"
#include "generic/typedef.h"
#include "generic/gpio.h"

#if defined(TCFG_MAX30102_DEV_ENABLE) && TCFG_MAX30102_DEV_ENABLE

#undef LOG_TAG_CONST
#define LOG_TAG     "[max30102]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

#if TCFG_MAX30102_USE_IIC_TYPE
#define iic_init(iic)                       hw_iic_init(iic)
#define iic_uninit(iic)                     hw_iic_uninit(iic)
#define iic_start(iic)                      hw_iic_start(iic)
#define iic_stop(iic)                       hw_iic_stop(iic)
#define iic_tx_byte(iic, byte)              hw_iic_tx_byte(iic, byte)
#define iic_rx_byte(iic, ack)               hw_iic_rx_byte(iic, ack)
#define iic_read_buf(iic, buf, len)         hw_iic_read_buf(iic, buf, len)
#define iic_write_buf(iic, buf, len)        hw_iic_write_buf(iic, buf, len)
#define iic_suspend(iic)                    hw_iic_suspend(iic)
#define iic_resume(iic)                     hw_iic_resume(iic)
#else
#define iic_init(iic)                       soft_iic_init(iic)
#define iic_uninit(iic)                     soft_iic_uninit(iic)
#define iic_start(iic)                      soft_iic_start(iic)
#define iic_stop(iic)                       soft_iic_stop(iic)
#define iic_tx_byte(iic, byte)              soft_iic_tx_byte(iic, byte)
#define iic_rx_byte(iic, ack)               soft_iic_rx_byte(iic, ack)
#define iic_read_buf(iic, buf, len)         soft_iic_read_buf(iic, buf, len)
#define iic_write_buf(iic, buf, len)        soft_iic_write_buf(iic, buf, len)
#define iic_suspend(iic)                    soft_iic_suspend(iic)
#define iic_resume(iic)                     soft_iic_resume(iic)
#endif
/* extern void delay(unsigned int cnt);//eeprom */
extern void delay_2ms(int cnt);//fm

static struct _max30102_dev_platform_data *max30102_iic_info;

static bool max30102_write_reg(u8 max_reg_addr, u8 max_data)
{
    iic_start(max30102_iic_info->iic_hdl);
    if (0 == iic_tx_byte(max30102_iic_info->iic_hdl, MAX30102_WADDR)) {
        iic_stop(max30102_iic_info->iic_hdl);
        log_error("max30102 write fail1!\n");
        return false;
    }
    delay_2ms(max30102_iic_info->iic_delay);
    if (0 == iic_tx_byte(max30102_iic_info->iic_hdl, max_reg_addr)) {
        iic_stop(max30102_iic_info->iic_hdl);
        log_error("max30102 write fail2!\n");
        return false;
    }
    delay_2ms(max30102_iic_info->iic_delay);
    if (0 == iic_tx_byte(max30102_iic_info->iic_hdl, max_data)) {
        iic_stop(max30102_iic_info->iic_hdl);
        log_error("max30102 write fail3!\n");
        return false;
    }
    iic_stop(max30102_iic_info->iic_hdl);
    delay_2ms(max30102_iic_info->iic_delay);
    return true;
}
static bool max30102_read_reg(u8 max_reg_addr, u8 *read_data, u8 len)
{
    iic_start(max30102_iic_info->iic_hdl);
    if (0 == iic_tx_byte(max30102_iic_info->iic_hdl, MAX30102_WADDR)) {
        iic_stop(max30102_iic_info->iic_hdl);
        log_error("max30102 read fail1!\n");
        return false;
    }
    delay_2ms(max30102_iic_info->iic_delay);
    if (0 == iic_tx_byte(max30102_iic_info->iic_hdl, max_reg_addr)) {
        iic_stop(max30102_iic_info->iic_hdl);
        log_error("max30102 read fail2!\n");
        return false;
    }
    delay_2ms(max30102_iic_info->iic_delay);
    iic_start(max30102_iic_info->iic_hdl);
    if (0 == iic_tx_byte(max30102_iic_info->iic_hdl, MAX30102_RADDR)) {
        iic_stop(max30102_iic_info->iic_hdl);
        log_error("max30102 read fail3!\n");
        return false;
    }

    for (u8 i = 0; i < len - 1; i++) {
        read_data[i] = iic_rx_byte(max30102_iic_info->iic_hdl, 1);
        delay_2ms(max30102_iic_info->iic_delay);
    }
    read_data[len - 1] = iic_rx_byte(max30102_iic_info->iic_hdl, 0);
    iic_stop(max30102_iic_info->iic_hdl);
    delay_2ms(max30102_iic_info->iic_delay);
    return true;
}
//Reset the MAX30102
static bool max30102_reset()
{
    if (!max30102_write_reg(REG_MODE_CONFIG, 0x40)) {
        return false;
    } else {
        return true;
    }
}

bool max30102_init(void *priv)
{
    u8 temp = 0;
    if (priv == NULL) {
        log_info("max30102 init fail(no priv)\n");
        return false;
    }
    max30102_iic_info = (struct _max30102_dev_platform_data *)priv;
    if (max30102_reset()) {
        log_info("reset ok");
    }
    os_time_dly(1);//10ms
    if (max30102_read_reg(REG_INTR_STATUS_1, &temp, 1)) { //上电清中断
        log_info("clear int ok");
    }
    //algorithm:2    1
    if (!max30102_write_reg(REG_INTR_ENABLE_1, 0x40)) { //0xc0 0x40 INTR setting
        return false;
    }
    if (!max30102_write_reg(REG_INTR_ENABLE_2, 0x00)) {
        return false;
    }
    if (!max30102_write_reg(REG_FIFO_WR_PTR, 0x00)) {   //FIFO_WR_PTR[4:0]
        return false;
    }
    if (!max30102_write_reg(REG_OVF_COUNTER, 0x00)) {   //OVF_COUNTER[4:0]
        return false;
    }
    if (!max30102_write_reg(REG_FIFO_RD_PTR, 0x00)) {   //FIFO_RD_PTR[4:0]
        return false;
    }
    if (!max30102_write_reg(REG_FIFO_CONFIG, 0x0f)) {   //0x6f 0x00 sample avg = 1, fifo rollover=false, fifo almost full = 17
        return false;
    }
    if (!max30102_write_reg(REG_MODE_CONFIG, 0x03)) {   //0x03 0x03. 0x02 for heart rate(Red only), 0x03 for SpO2 mode 0x07 multimode LED
        return false;
    }
    if (!max30102_write_reg(REG_SPO2_CONFIG, 0x67)) {   //0x27 0x67 SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (11:410uS;ADC:18bit)
        return false;
    }

    if (!max30102_write_reg(REG_LED1_PA, 0x9f)) {       //0x17 0xbf Choose value for ~ 7mA for LED1
        return false;
    }
    if (!max30102_write_reg(REG_LED2_PA, 0x9f)) {       //0x17 0xbf Choose value for ~ 7mA for LED2
        return false;
    }
    return true;
}
//This function reads a set of samples from the MAX30102 FIFO register
bool max30102_read_fifo(u32 *read_red_led, u32 *read_ir_led)
{
    u32 un_temp;
    u8 uch_temp;
    *read_red_led = 0;
    *read_ir_led = 0;
    u8 ach_i2c_data[6];

    //read and clear status register
    max30102_read_reg(REG_INTR_STATUS_1, &uch_temp, 1);
    max30102_read_reg(REG_INTR_STATUS_2, &uch_temp, 1);

    max30102_read_reg(REG_FIFO_DATA, ach_i2c_data, 6);
    *read_red_led = (u32)(ach_i2c_data[0]) << 16 | (u32)ach_i2c_data[1] << 8 | (u32)ach_i2c_data[2];
    *read_ir_led = (u32)(ach_i2c_data[3]) << 16 | (u32)ach_i2c_data[4] << 8 | (u32)ach_i2c_data[5];

    *read_red_led &= 0x03FFFF; //Mask MSB [23:18]
    *read_ir_led &= 0x03FFFF; //Mask MSB [23:18]
    /* if (!max30102_write_reg(REG_FIFO_WR_PTR, 0x00)) { //FIFO_WR_PTR[4:0] */
    /*     return false; */
    /* } */
    /* if (!max30102_write_reg(REG_OVF_COUNTER, 0x00)) { //OVF_COUNTER[4:0] */
    /*     return false; */
    /* } */
    /* if (!max30102_write_reg(REG_FIFO_RD_PTR, 0x00)) { //FIFO_RD_PTR[4:0] */
    /*     return false; */
    /* } */
    return true;
}

bool max30102_power_control(u8 shutdown_en)//1:power save(中断也被清除); 0:normal
{
    u8 mode_reg_data = 0;
    max30102_read_reg(REG_MODE_CONFIG,  &mode_reg_data, 1);
    if (shutdown_en) {
        mode_reg_data |= BIT(7);
    } else {
        mode_reg_data &= ~BIT(7);
    }
    delay_2ms(10);
    if (max30102_write_reg(REG_MODE_CONFIG, mode_reg_data)) {
        return true;
    } else {
        return false;
    }
}





/*************************test*************************/
#if 0
//test 1: max30102 read 500 data
static struct _max30102_dev_platform_data max30102_iic_info_test = {
    .iic_hdl = 0,
    .iic_delay = 0
};
#define MAX30102_BUFFER_LEN BUFFER_SIZE
#define _portio 1
static u32 max30102_red_buf[MAX30102_BUFFER_LEN], max30102_ir_buf[MAX30102_BUFFER_LEN];
void max_init_read_test()
{
    u16 i = 0, iii = 10;
    iic_init(0);
    JL_PORTA->DIR &= ~(BIT(0) | BIT(9) | BIT(10));
    JL_PORTA->OUT &= ~(BIT(0) | BIT(9) | BIT(10));
    JL_PORTA->DIR |= (BIT(_portio));
    JL_PORTA->DIE |= (BIT(_portio));
    JL_PORTA->PU |= (BIT(_portio));

    if (max30102_init(&max30102_iic_info_test)) {
        log_info("max30102 init ok!***************************\n");
        wdt_clear();
    } else {
        log_error("max30102 init fail!***************************\n");
    }
    log_info("max30102 read data test end!***************************\n");
}

//传感器受环境光影响，尽量避免环境光干扰
//算法1：不够稳定，比算法2差
void cal_heart_sp02_data_test1()
{
    u8 ii = 50;
    u16 i = 0;
    int data_spo2 = 0, data_heart_rate = 0;
    int spo2_avg = 0, heart_rate_avg = 0;
    char spo2_valid = 0, heart_rate_valid = 0;

    max_init_read_test();
    for (i = 0; i < MAX30102_BUFFER_LEN; i++) {
        while (JL_PORTA->IN & BIT(_portio)); //wait New FIFO Data Ready
        max30102_read_fifo(max30102_red_buf + i, max30102_ir_buf + i);
        printf("i:%d,%d,%d\n", i, max30102_red_buf[i], max30102_ir_buf[i]);
        wdt_clear();
    }
    log_info("max30102 calculate heartrate data test!***************************");
    while (ii--) {
        for (i = MAX30102_BUFFER_LEN / 5; i < MAX30102_BUFFER_LEN; i++) {
            max30102_red_buf[i - MAX30102_BUFFER_LEN / 5] = max30102_red_buf[i];
            max30102_ir_buf[i - MAX30102_BUFFER_LEN / 5] = max30102_ir_buf[i];
        }
        for (i = MAX30102_BUFFER_LEN / 5 * 4; i < MAX30102_BUFFER_LEN; i++) {
            while (JL_PORTA->IN & BIT(_portio));
            max30102_read_fifo(max30102_red_buf + i, max30102_ir_buf + i);
            wdt_clear();
        }
        max301x_heart_rate_and_oxygen_saturation(max30102_ir_buf, MAX30102_BUFFER_LEN, max30102_red_buf, &data_spo2, &spo2_valid, &data_heart_rate, &heart_rate_valid);
        heart_rate_avg = calculate_average(data_heart_rate, &heart_rate_valid, 1);
        spo2_avg = calculate_average(data_spo2, &spo2_valid, 0);
        log_info("data_spo2:%d:%d, spo2_valid:%d, data_heart_rate:%d:%d,heart_rate_valid:%d\n", data_spo2, spo2_avg, spo2_valid, data_heart_rate, heart_rate_avg, heart_rate_valid);
    }
}

//协议
void sanwai_show(u32 redbuf, u32 irbuf, u32 hr)
{
    u8 temp1[16];
    temp1[0] = 0x03;
    temp1[1] = 0xfc;
    temp1[15] = 0x03;
    temp1[14] = 0xfc;
    temp1[5] = redbuf >> 24;
    temp1[4] = redbuf >> 16;
    temp1[3] = redbuf >> 8;
    temp1[2] = redbuf;
    temp1[9] = irbuf >> 24;
    temp1[8] = irbuf >> 16;
    temp1[7] = irbuf >> 8;
    temp1[6] = irbuf;
    temp1[13] = 0;
    temp1[12] = 0;
    temp1[11] = hr >> 8;
    temp1[10] = hr;
    for (u8 j = 0; j < 16; j++) {
        putbyte(temp1[j]);
    }
    /* os_time_dly(1); */
    /* log_info("RedBuffer:%d,IrBuffer:%d,FFT_Power:%d",RedBuffer[i],IrBuffer[i],FFT_Power[i]); */
}
//传感器受环境光影响，尽量避免环境光干扰
//算法2：
void cal_heart_sp02_data_test2()
{
    unsigned  int i = 60, HR = 0;
    u16 j = 0;
    double spo2_data;
    int spo2_avg = 0, heart_rate_avg = 0;
    char spo2_valid = 0, heart_rate_valid = 1;

    max_init_read_test();
    while (i--) {
        HR = MAX30102_GetHeartRate();
        heart_rate_valid = 1;
        heart_rate_avg = calculate_average(HR, &heart_rate_valid, 1);
        wdt_clear();
        for (j = 0; j < 256; j++) {
            sanwai_show(RedBuffer[j], IrBuffer[j], HR);
        }
        log_info("\nHeart Rate:%d ,avg:%d  beats/min %d\n", HR, heart_rate_avg, heart_rate_valid);
        spo2_data = MAX30102_GetSPO2();
        spo2_valid = 1;
        spo2_avg = calculate_average((s32)spo2_data, &spo2_valid, 0);
        log_info("SPO2:%d  avg:%d.%02d%%", (s32)spo2_data, spo2_avg, ((s32)(spo2_data * 100)) % 100);
        wdt_clear();
    }
}

#endif



#endif
