#include "app_config.h"
#include "asm/clock.h"
#include "system/timer.h"
#include "bmp280.h"
#include "asm/cpu.h"
#include "generic/typedef.h"
#include "generic/gpio.h"

#if defined(TCFG_BMP280_DEV_ENABLE) && TCFG_BMP280_DEV_ENABLE

#undef LOG_TAG_CONST
#define LOG_TAG     "[bmp280]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

struct _bmp280_dev_platform_data *bmp280_iic_info;

#if TCFG_BMP280_USER_IIC_TYPE
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

static bool bmp280_read_buf(u8 reg, u8 *buf, u8 len)
{
    iic_start(bmp280_iic_info->iic_hdl);
    if (0 == iic_tx_byte(bmp280_iic_info->iic_hdl, BMP280_I2C_ADDR << 1 | 0)) {
        iic_stop(bmp280_iic_info->iic_hdl);
        log_error("bmp280 read fail1!\n");
        return false;
    }
    delay_2ms(bmp280_iic_info->iic_delay);
    if (0 == iic_tx_byte(bmp280_iic_info->iic_hdl, reg)) {
        iic_stop(bmp280_iic_info->iic_hdl);
        log_error("bmp280 read fail2!\n");
        return false;
    }
    delay_2ms(bmp280_iic_info->iic_delay);
    iic_start(bmp280_iic_info->iic_hdl);
    if (0 == iic_tx_byte(bmp280_iic_info->iic_hdl, BMP280_I2C_ADDR << 1 | 1)) {
        iic_stop(bmp280_iic_info->iic_hdl);
        log_error("bmp280 read fail3!\n");
        return false;
    }

    for (u8 i = 0; i < len - 1; i++) {
        buf[i] = iic_rx_byte(bmp280_iic_info->iic_hdl, 1);
        delay_2ms(bmp280_iic_info->iic_delay);
    }
    buf[len - 1] = iic_rx_byte(bmp280_iic_info->iic_hdl, 0);
    iic_stop(bmp280_iic_info->iic_hdl);
    delay_2ms(bmp280_iic_info->iic_delay);
    return true;
}

static bool bmp280_write_byte(u8 reg, u8 data)
{
    iic_start(bmp280_iic_info->iic_hdl);
    if (0 == iic_tx_byte(bmp280_iic_info->iic_hdl, BMP280_I2C_ADDR << 1)) {
        iic_stop(bmp280_iic_info->iic_hdl);
        log_error("bmp280 write fail1!\n");
        return false;
    }
    delay_2ms(bmp280_iic_info->iic_delay);
    if (0 == iic_tx_byte(bmp280_iic_info->iic_hdl, reg)) {
        iic_stop(bmp280_iic_info->iic_hdl);
        log_error("bmp280 write fail2!\n");
        return false;
    }
    delay_2ms(bmp280_iic_info->iic_delay);
    if (0 == iic_tx_byte(bmp280_iic_info->iic_hdl, data)) {
        iic_stop(bmp280_iic_info->iic_hdl);
        log_error("bmp280 write fail3!\n");
        return false;
    }
    iic_stop(bmp280_iic_info->iic_hdl);
    delay_2ms(bmp280_iic_info->iic_delay);
    return true;
}

//设置BMP过采样因子 MODE
//BMP280_SLEEP_MODE||BMP280_FORCED_MODE||BMP280_NORMAL_MODE
bool bmp280_set_temoversamp(bmp_oversample_mode *oversample_mode)
{
    u8 regtmp;
    regtmp = ((oversample_mode->t_osample) << 5) |
             ((oversample_mode->p_osample) << 2) |
             ((oversample_mode)->workmode);
    /* log_info("init reg:%x",regtmp); */

    return bmp280_write_byte(BMP280_CTRLMEAS_REG, regtmp);
}
//设置保持时间和滤波器分频因子
bool bmp280_set_standby_filter(bmp_config *bmp_config)
{
    u8 regtmp;
    regtmp = ((bmp_config->t_sb) << 5) |
             ((bmp_config->filter_coefficient) << 2) |
             ((bmp_config->spi_en));
    /* log_info("init config reg:%x",regtmp); */

    return bmp280_write_byte(BMP280_CONFIG_REG, regtmp);
}
//设置bmp280工作模式
void bmp280_set_work_mode(BMP280_WORK_MODE workmode)
{
    u8 ctrl_maes_data = 0;
    bmp280_read_buf(BMP280_CTRLMEAS_REG, &ctrl_maes_data, 1);
    /* log_info("old ctrl_maes_data:%x ",ctrl_maes_data); */
    ctrl_maes_data &= ~(BIT(1) | BIT(0));
    ctrl_maes_data += (workmode << 0);
    /* log_info("nem ctrl_maes_data:%x ",ctrl_maes_data); */
    bmp280_write_byte(BMP280_CTRLMEAS_REG, ctrl_maes_data);
}
//bmp280 复位
bool bmp280_reset()  //return 1:ok;   0:fail
{
    return bmp280_write_byte(BMP280_RESET_REG, BMP280_RESET_VALUE);
}

bmp280_params bmp280_par;
bool bmp280_init(void *priv)
{
    u8 bmp280_id;
    if (priv == NULL) {
        log_info("bmp280 init fail(no priv)\n");
        return false;
    }
    bmp280_iic_info = (struct _bmp280_dev_platform_data *)priv;

    if (bmp280_read_buf(BMP280_CHIPID_REG, &bmp280_id, 1)) {
        log_info("bmp280 id:%x", bmp280_id);
    } else {
        log_error("read bmp280 id fail!");
        return false;
    }
    /********************读矫正参数*********************/
    u8 temp[24] = {0};
    bmp280_read_buf(BMP280_DIG_T1_LSB_REG, temp, 24);
    //温度传感器的矫正值
    bmp280_par.t1 = (((u16)temp[1]) << 8) + temp[0];
    bmp280_par.t2 = (temp[3] << 8) + temp[2];
    bmp280_par.t3 = (temp[5] << 8) + temp[4];
    //大气压传感器的矫正值
    bmp280_par.p1 = (((u16)temp[7]) << 8) + temp[6];
    bmp280_par.p2 = (temp[9] << 8) + temp[8];
    bmp280_par.p3 = (temp[11] << 8) + temp[10];
    bmp280_par.p4 = (temp[13] << 8) + temp[12];
    bmp280_par.p5 = (temp[15] << 8) + temp[14];
    bmp280_par.p6 = (temp[17] << 8) + temp[16];
    bmp280_par.p7 = (temp[19] << 8) + temp[18];
    bmp280_par.p8 = (temp[21] << 8) + temp[20];
    bmp280_par.p9 = (temp[23] << 8) + temp[22];
    /* log_info("%d,%d,%d;%d,%d,%d,%d,%d,%d,%d,%d,%d",bmp280_par.t1,bmp280_par.t2,bmp280_par.t3,bmp280_par.p1,bmp280_par.p2,bmp280_par.p3,bmp280_par.p4,bmp280_par.p5,bmp280_par.p6,bmp280_par.p7,bmp280_par.p8,bmp280_par.p9); */

    /******************************************************/
    if (!(bmp280_write_byte(BMP280_RESET_REG, BMP280_RESET_VALUE))) {	//往复位寄存器写入给定值
        log_error("bmp280 reset error!\n");
        return false;
    }

    bmp_config					bmp_configstr;
    bmp_configstr.t_sb = BMP280_T_SB1;
    bmp_configstr.filter_coefficient = BMP280_FILTER_MODE_4;
    bmp_configstr.spi_en = DISABLE;
    if (!(bmp280_set_standby_filter(&bmp_configstr))) {
        log_error("bmp280 init mode error!\n");
        return false;
    }

    bmp_oversample_mode			bmp_oversample_modestr;
    bmp_oversample_modestr.t_osample = BMP280_T_MODE_1;
    bmp_oversample_modestr.p_osample = BMP280_P_MODE_3;
    bmp_oversample_modestr.workmode  = BMP280_NORMAL_MODE;
    if (!(bmp280_set_temoversamp(&bmp_oversample_modestr))) {
        log_error("bmp280 init mode error!\n");
        return false;
    }

    return true;
}

/*****************算法*******************/
/************计算补偿值***************/
s32 t_fine;			//用于计算补偿
#if USE_FIXED_POINT_COMPENSATE  //使用定点补偿
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
static s32 bmp280_compensate_t_int32(s32 adc_T)
{
    s32 var1, var2, T;
    var1 = (((adc_T >> 3) - ((s32)bmp280_par.t1 << 1)) * ((s32)bmp280_par.t2)) >> 11;
    var2 = (((((adc_T >> 4) - ((s32)bmp280_par.t1)) * ((adc_T >> 4) - ((s32)bmp280_par.t1))) >> 12) * ((s32)bmp280_par.t3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
static u32 bmp280_compensate_p_int64(s32 adc_P)
{
    s64 var1, var2, p;
    var1 = ((s64)t_fine) - 128000;
    var2 = var1 * var1 * (s64)bmp280_par.p6;
    var2 = var2 + ((var1 * (s64)bmp280_par.p5) << 17);
    var2 = var2 + (((s64)bmp280_par.p4) << 35);
    var1 = ((var1 * var1 * (s64)bmp280_par.p3) >> 8) + ((var1 * (s64)bmp280_par.p2) << 12);
    var1 = (((((s64)1) << 47) + var1)) * ((s64)bmp280_par.p1) >> 33;
    if (var1 == 0) {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((s64)bmp280_par.p9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((s64)bmp280_par.p8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((s64)bmp280_par.p7) << 4);
    return (u32)p;
}

#else  //使用浮点补偿
// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value
static double bmp280_compensate_t_double(s32 adc_T)
{
    double var1, var2, T;
    var1 = (((double)adc_T) / 16384.0 - ((double)bmp280_par.t1) / 1024.0) * ((double)bmp280_par.t2);
    var2 = ((((double)adc_T) / 131072.0 - ((double)bmp280_par.t1) / 8192.0) * (((double)adc_T) / 131072.0 - ((double) bmp280_par.t1) / 8192.0)) * ((double)bmp280_par.t3);
    t_fine = (s32)(var1 + var2);
    T = (var1 + var2) / 5120.0;
    return T;
}

// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
static double bmp280_compensate_p_double(s32 adc_P)
{
    double var1, var2, p;
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)bmp280_par.p6) / 32768.0;
    var2 = var2 + var1 * ((double)bmp280_par.p5) * 2.0;
    var2 = (var2 / 4.0) + (((double)bmp280_par.p4) * 65536.0);
    var1 = (((double)bmp280_par.p3) * var1 * var1 / 524288.0 + ((double)bmp280_par.p2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)bmp280_par.p1);
    if (var1 == 0.0) {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)bmp280_par.p9) * p * p / 2147483648.0;
    var2 = p * ((double)bmp280_par.p8) / 32768.0;
    p = p + (var1 + var2 + ((double)bmp280_par.p7)) / 16.0;
    return p;
}
#endif

//获取BMP当前状态
//status_flag = BMP280_MEASURING
//			 	BMP280_IM_UPDATE
static u8  bmp280_getstatus(u8 status_flag)
{
    u8 flag;
    bmp280_read_buf(BMP280_STATUS_REG, &flag, 1);
    if (flag & status_flag)	{
        return 1;
    } else {
        return 0;
    }
}
//温度值-℃(实际值的100倍(定点法)或实际值(浮点法))
static bmp_temperature_data bmp280_get_temperature(void)
{
    u8 tem_data[3] = {0};
    long signed bit32;
    bmp280_read_buf(BMP280_TEMPERATURE_MSB_REG, tem_data, 3);
    bit32 = ((long)(tem_data[0] << 12)) | ((long)(tem_data[1] << 4)) | (tem_data[2] >> 4);
    /* log_info("tembit32:%ld",bit32); */
#if USE_FIXED_POINT_COMPENSATE  //使用定点补偿
    return bmp280_compensate_t_int32(bit32);
#else      //使用浮点补偿
    return bmp280_compensate_t_double(bit32);
#endif
}
//获取大气压值-Pa(实际值的100倍(定点法)或实际值(浮点法))
static bmp_pressure_data bmp280_get_pressure(void)
{
    u8 tem_data[3] = {0};
    long signed bit32;
    bmp280_read_buf(BMP280_PRESSURE_MSB_REG, tem_data, 3);
    bit32 = ((long)(tem_data[0] << 12)) | ((long)(tem_data[1] << 4)) | (tem_data[2] >> 4);
    /* log_info("prebit32:%ld,t_fine:%d",bit32,t_fine); */
#if USE_FIXED_POINT_COMPENSATE  //使用定点补偿
    u32 pressure = bmp280_compensate_p_int64(bit32);
    pressure = pressure * 100 / 256;
#else      //使用浮点补偿
    double pressure = bmp280_compensate_p_double(bit32);
#endif
    return pressure;
}
/*
 * 仅在BMP280被设置为normal mode时，
 * 可使用该接口直接读取温度和气压。
 * (实际值的100倍(定点法)或实际值(浮点法))
 */
void bmp280_get_temperature_and_pressure(bmp_temperature_data *temperature, bmp_pressure_data *pressure)
{
    while (bmp280_getstatus(BMP280_MEASURING));
    while (bmp280_getstatus(BMP280_IM_UPDATE));
    *temperature = bmp280_get_temperature();  //必须先读温度再度气压
    *pressure = bmp280_get_pressure();
}

/**
 * 当BMP280被设置为forced mode时，
 * 可使用该接口直接读取温度和气压。
 * (实际值的100倍(定点法)或实际值(浮点法))
 */
void bmp280_forced_mode_get_temperature_and_pressure(bmp_temperature_data *temperature, bmp_pressure_data *pressure)
{
    bmp280_set_work_mode(BMP280_FORCED_MODE);
    bmp280_get_temperature_and_pressure(temperature, pressure);
}

//计算高度, 返回海拔高度: (m)
#include "math.h"
#define SEA_LEVEL_PRESSURE  1013.23f
float bmp280_get_altitude(bmp_temperature_data temperature, bmp_pressure_data pressure)
{
    /* log_info("temperature:%d,pressure:%d",(s32)temperature,(u32)pressure); */
#if USE_FIXED_POINT_COMPENSATE  //使用定点补偿
    /* return 44330 * (1.0 - pow(pressure/10000 / SEA_LEVEL_PRESSURE, 0.1903)); */
    //计算公式温度用实际值，压强用实际值的百分之一
    return ((float)powf(SEA_LEVEL_PRESSURE / ((float)pressure / 10000.0), 0.190223f) - 1.0f) * ((float)temperature / 100.0 + 273.15f) / 0.0065f; // Calculate the altitude in metres
#else      //使用浮点补偿
    /* return 44330 * (1.0 - pow(pressure/100 / SEA_LEVEL_PRESSURE, 0.1903)); */
    //计算公式温度用实际值，压强用实际值的百分之一
    return ((float)powf(SEA_LEVEL_PRESSURE / ((float)pressure / 100.0), 0.190223f) - 1.0f) * ((float)temperature + 273.15f) / 0.0065f; // Calculate the altitude in metres
#endif
}

/*************************test*************************/
#if 0
static struct _bmp280_dev_platform_data bmp280_iic_info_test = {
    .iic_hdl = 0,
    .iic_delay = 0
};
#define _portio 0
void bmp280_init_read_test()
{
    u16 i = 0, iii = 40;
    bmp_pressure_data bmp_pressure;
    bmp_temperature_data bmp_temperature;
    float bmp280_altitude = 0;
    /* JL_PORTA->DIR &= ~(BIT(0)|BIT(9)|BIT(10)); */
    /* JL_PORTA->OUT &= ~(BIT(0)|BIT(9)|BIT(10)); */
    iic_init(0);

    log_info("bmp280 init***************************\n");
    if (bmp280_init(&bmp280_iic_info_test)) {
        log_info("bmp280 init ok!***************************\n");
        while (iii--) {
            bmp280_get_temperature_and_pressure(&bmp_temperature, &bmp_pressure);
            /* bmp280_forced_mode_get_temperature_and_pressure(&bmp_temperature , &bmp_pressure); */
            bmp280_altitude = bmp280_get_altitude(bmp_temperature, bmp_pressure);
#if USE_FIXED_POINT_COMPENSATE  //使用定点补偿

            log_info("bmp280 fixed Pressure %ld.%02dpa,temperature:%d.%02dc,alti:%d.%02dm\n", bmp_pressure / 100, bmp_pressure % 100, bmp_temperature / 100, bmp_temperature % 100, (s32)bmp280_altitude, (u32)((s32)(bmp280_altitude * 100) % 100));
#else          //使用浮点补偿

            log_info("bmp280 float Pressure %d.%02dpa,temperature:%d.%02dc,alti:%d.%02dm\n", (s32)bmp_pressure, (s32)(bmp_pressure * 100) % 100, (s32)bmp_temperature, (s32)(bmp_temperature * 100) % 100, (s32)bmp280_altitude, (u32)((s32)(bmp280_altitude * 100) % 100));
#endif
            os_time_dly(50);
        }
    } else {
        log_error("bmp280 init fail!***************************\n");
    }
    while (1);
}

#endif



#endif
