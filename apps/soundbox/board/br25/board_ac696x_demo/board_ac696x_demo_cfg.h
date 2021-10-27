#ifndef CONFIG_BOARD_AC696X_DEMO_CFG_H
#define CONFIG_BOARD_AC696X_DEMO_CFG_H

#ifdef CONFIG_BOARD_AC696X_DEMO

#define CONFIG_SDFILE_ENABLE
#define CONFIG_FLASH_SIZE       (1024 * 1024)

//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE					1
#define DISABLE_THIS_MOUDLE					0

#define ENABLE								1
#define DISABLE								0

#define LINEIN_INPUT_WAY_ANALOG      0
#define LINEIN_INPUT_WAY_ADC         1
#define LINEIN_INPUT_WAY_DAC         2

#define NO_CONFIG_PORT						(-1)

//*********************************************************************************//
//                                  app 配置                                       //
//*********************************************************************************//
#define TCFG_APP_BT_EN			            1
#define TCFG_APP_MUSIC_EN			        0
#define TCFG_APP_LINEIN_EN					0
#define TCFG_APP_FM_EN					    1
#define TCFG_APP_PC_EN					    0
#define TCFG_APP_RTC_EN					    1
#define TCFG_APP_RECORD_EN				  0
#define TCFG_APP_SPDIF_EN           0
//*********************************************************************************//
//                               PCM_DEBUG调试配置                                 //
//*********************************************************************************//

//#define AUDIO_PCM_DEBUG					  	//PCM串口调试，写卡通话数据

//*********************************************************************************//
//                                 UART配置                                        //
//*********************************************************************************//
#define TCFG_UART0_ENABLE					ENABLE_THIS_MOUDLE                     //串口打印模块使能
#define TCFG_UART0_RX_PORT					NO_CONFIG_PORT                         //串口接收脚配置（用于打印可以选择NO_CONFIG_PORT）
#define TCFG_UART0_TX_PORT  				IO_PORTC_03  //IO_PORTC_03 IO_PORTB_00                          //串口发送脚配置
#define TCFG_UART0_BAUDRATE  				1000000                                //串口波特率配置

//*********************************************************************************//
//                                 IIC配置                                        //
//*********************************************************************************//
/*软件IIC设置*/
#define TCFG_SW_I2C0_CLK_PORT               IO_PORTA_09                             //软件IIC  CLK脚选择
#define TCFG_SW_I2C0_DAT_PORT               IO_PORTA_10                             //软件IIC  DAT脚选择
#define TCFG_SW_I2C0_DELAY_CNT              50                                      //IIC延时参数，影响通讯时钟频率

/*硬件IIC端口选择
  SCL         SDA
  'A': IO_PORT_DP   IO_PORT_DM
  'B': IO_PORTC_04  IO_PORTC_05
  'C': IO_PORTB_06  IO_PORTB_07
  'D': IO_PORTA_05  IO_PORTA_06
 */
#define TCFG_HW_I2C0_PORTS                  'B'
#define TCFG_HW_I2C0_CLK                    100000                                  //硬件IIC波特率

//*********************************************************************************//
//                                 硬件SPI 配置                                        //
//*********************************************************************************//
#define	TCFG_HW_SPI1_ENABLE		DISABLE_THIS_MOUDLE
//A组IO:    DI: PB2     DO: PB1     CLK: PB0
//B组IO:    DI: PC3     DO: PC5     CLK: PC4
#define TCFG_HW_SPI1_PORT		'A'
#define TCFG_HW_SPI1_BAUD		4000000L
#define TCFG_HW_SPI1_MODE		SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI1_ROLE		SPI_ROLE_MASTER

#define	TCFG_HW_SPI2_ENABLE		DISABLE_THIS_MOUDLE
//A组IO:    DI: PB8     DO: PB10    CLK: PB9
//B组IO:    DI: PA13    DO: DM      CLK: DP
#define TCFG_HW_SPI2_PORT		'A'
#define TCFG_HW_SPI2_BAUD		2000000L
#define TCFG_HW_SPI2_MODE		SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI2_ROLE		SPI_ROLE_MASTER

//*********************************************************************************//
//                                 FLASH 配置                                      //
//*********************************************************************************//
#define TCFG_NORFLASH_DEV_ENABLE		    DISABLE_THIS_MOUDLE //需要关闭SD0
#define TCFG_FLASH_DEV_SPI_HW_NUM			1// 1: SPI1    2: SPI2
#define TCFG_FLASH_DEV_SPI_CS_PORT	    	IO_PORTA_03


//*********************************************************************************//
//                                  充电参数配置                                   //
//*********************************************************************************//
//是否支持芯片内置充电
#define TCFG_CHARGE_ENABLE					DISABLE_THIS_MOUDLE
//是否支持开机充电
#define TCFG_CHARGE_POWERON_ENABLE			ENABLE
//是否支持拔出充电自动开机功能
#define TCFG_CHARGE_OFF_POWERON_NE			ENABLE

#define TCFG_CHARGE_FULL_V					CHARGE_FULL_V_4202

#define TCFG_CHARGE_FULL_MA					CHARGE_FULL_mA_10

#define TCFG_CHARGE_MA						CHARGE_mA_60



//*********************************************************************************//
//                                  SD 配置                                        //
//*********************************************************************************//
#define     SD_CMD_DECT 	0
#define     SD_CLK_DECT  	1
#define     SD_IO_DECT 		2

//A组IO: CMD:PC4    CLK:PC5    DAT0:PC3             //D组IO: CMD:PB2    CLK:PB0    DAT0:PB3
//B组IO: CMD:PB6    CLK:PB7    DAT0:PB5             //E组IO: CMD:PA4    CLK:PC5    DAT0:DM
//C组IO: CMD:PA4    CLK:PA2    DAT0:PA3             //F组IO: CMD:PB6    CLK:PB7    DAT0:PB4
#define TCFG_SD0_ENABLE						DISABLE_THIS_MOUDLE
#define TCFG_SD0_PORTS						'D'
#define TCFG_SD0_DAT_MODE					1//AC696x不支持4线模式
#define TCFG_SD0_DET_MODE                   SD_CLK_DECT
#define TCFG_SD0_DET_IO 					IO_PORT_DM//当SD_DET_MODE为2时有效
#define TCFG_SD0_DET_IO_LEVEL				0//IO检查，0：低电平检测到卡。 1：高电平(外部电源)检测到卡。 2：高电平(SD卡电源)检测到卡。
#define TCFG_SD0_CLK						(3000000*2L)

#define TCFG_SD0_SD1_USE_THE_SAME_HW	    DISABLE_THIS_MOUDLE
#if TCFG_SD0_SD1_USE_THE_SAME_HW
#define TCFG_SD1_ENABLE						0
#else
#define TCFG_SD1_ENABLE						0
#endif
#define TCFG_SD1_PORTS						'F'
#define TCFG_SD1_DAT_MODE					1//AC696x不支持4线模式
#define TCFG_SD1_DET_MODE					SD_CLK_DECT
#define TCFG_SD1_DET_IO 					IO_PORT_DM//当SD_DET_MODE为2时有效
#define TCFG_SD1_DET_IO_LEVEL				0//IO检查，0：低电平检测到卡。 1：高电平(外部电源)检测到卡。 2：高电平(SD卡电源)检测到卡。
#define TCFG_SD1_CLK						(3000000*2L)




#define TCFG_KEEP_CARD_AT_ACTIVE_STATUS      0 //保持卡活跃状态



//*********************************************************************************//
//                                 USB 配置                                        //
//*********************************************************************************//
#define TCFG_PC_ENABLE						TCFG_APP_PC_EN//PC模块使能
#define TCFG_UDISK_ENABLE					DISABLE_THIS_MOUDLE//U盘模块使能
#define TCFG_HOST_AUDIO_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_HID_HOST_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_OTG_USB_DEV_EN                 BIT(0)//USB0 = BIT(0)  USB1 = BIT(1)

#include "usb_std_class_def.h"


#define TCFG_USB_PORT_CHARGE            DISABLE

#define TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0       DISABLE


#if TCFG_USB_DM_MULTIPLEX_WITH_SD_DAT0
//复用情况下，如果使用此USB口作为充电（即LDO5V_IN连接到此USB口），
//TCFG_OTG_MODE需要或上TCFG_OTG_MODE_CHARGE，用来把charge从host区
//分开；否则不需要，如果LDO5V_IN与其他IO绑定，则不能或上
#define TCFG_DM_MULTIPLEX_WITH_SD_PORT      0//0:sd0  1:sd1 //dm 参与复用的sd配置
#undef TCFG_OTG_MODE
#define TCFG_OTG_MODE                       (TCFG_OTG_MODE_HOST|TCFG_OTG_MODE_SLAVE|TCFG_OTG_MODE_CHARGE|OTG_DET_DP_ONLY)

#undef USB_DEVICE_CLASS_CONFIG
#if TCFG_SD0_SD1_USE_THE_SAME_HW //开启了双卡的可以使能读卡器存续设备
#define     USB_DEVICE_CLASS_CONFIG (MASSSTORAGE_CLASS|SPEAKER_CLASS|MIC_CLASS|HID_CLASS)
#else
#define     USB_DEVICE_CLASS_CONFIG (SPEAKER_CLASS|MIC_CLASS|HID_CLASS)
#endif

#undef TCFG_SD0_DET_MODE
#define TCFG_SD0_DET_MODE					SD_CLK_DECT
#define TCFG_USB_SD_MULTIPLEX_IO            IO_PORTB_03

#endif

//*********************************************************************************//
//                                 fat_FLASH 配置                                      //
//*********************************************************************************//
#define TCFG_CODE_FLASH_ENABLE				DISABLE_THIS_MOUDLE

#define FLASH_INSIDE_REC_ENABLE             0

#if  TCFG_NORFLASH_DEV_ENABLE
#define TCFG_NOR_FAT                    0//ENABLE
#define TCFG_NOR_FS                     1//ENABLE
#define TCFG_NOR_REC                    0//ENABLE
#else
#define TCFG_NOR_FAT                    0//ENABLE
#define TCFG_NOR_FS                     0//ENABLE
#define TCFG_NOR_REC                    0//ENABLE
#endif

#define TCFG_VIRFAT_FLASH_ENABLE  0
#if TCFG_VIRFAT_FLASH_ENABLE
#undef TCFG_NOR_FS
#define TCFG_NOR_FS               1
#endif
// #define TCFG_VIRFAT_FLASH_SIMPLE


//*********************************************************************************//
//                                 key 配置                                        //
//*********************************************************************************//
//#define KEY_NUM_MAX                        	10
//#define KEY_NUM                            	3
#define KEY_IO_NUM_MAX						6
#define KEY_AD_NUM_MAX						10
#define KEY_IR_NUM_MAX						21
#define KEY_TOUCH_NUM_MAX					6
#define KEY_RDEC_NUM_MAX                    6
#define KEY_CTMU_TOUCH_NUM_MAX				6

#define MULT_KEY_ENABLE						DISABLE 		//是否使能组合按键消息, 使能后需要配置组合按键映射表

#define TCFG_KEY_TONE_EN					0//ENABLE		// 按键提示音。建议音频输出使用固定采样率

//*********************************************************************************//
//                                 iokey 配置                                      //
//*********************************************************************************//
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE //是否使能IO按键

#define TCFG_IOKEY_POWER_CONNECT_WAY		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO

#define TCFG_IOKEY_POWER_ONE_PORT			IO_PORTB_01        //IO按键端口

#define TCFG_IOKEY_PREV_CONNECT_WAY			ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY_PREV_ONE_PORT			IO_PORTB_00

#define TCFG_IOKEY_NEXT_CONNECT_WAY 		ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY_NEXT_ONE_PORT			IO_PORTB_02

//*********************************************************************************//
//                                 adkey 配置                                      //
//*********************************************************************************//
#define TCFG_ADKEY_ENABLE                   ENABLE_THIS_MOUDLE//是否使能AD按键
#define TCFG_ADKEY_LED_IO_REUSE				DISABLE_THIS_MOUDLE	//ADKEY 和 LED IO复用，led只能设置蓝灯显示
#define TCFG_ADKEY_IR_IO_REUSE				DISABLE_THIS_MOUDLE	//ADKEY 和 红外IO复用
#define TCFG_ADKEY_LED_SPI_IO_REUSE			DISABLE_THIS_MOUDLE	//ADKEY 和 LED SPI IO复用
#define TCFG_ADKEY_PORT                     IO_PORTB_01         //AD按键端口(需要注意选择的IO口是否支持AD功能)
#define TCFG_ADKEY_AD_CHANNEL               AD_CH_PB1
#define TCFG_ADKEY_EXTERN_UP_ENABLE         ENABLE_THIS_MOUDLE //是否使用外部上拉

#if TCFG_ADKEY_EXTERN_UP_ENABLE
#define R_UP    100                 //22K，外部上拉阻值在此自行设置
#else
#define R_UP    100                 //10K，内部上拉默认10K
#endif

//必须从小到大填电阻，没有则同VDDIO,填0x3ffL
#define TCFG_ADKEY_AD0      (0)                                 //0R
#define TCFG_ADKEY_AD1      (0x3ffL * 10   / (10   + R_UP))     //3k
#define TCFG_ADKEY_AD2      (0x3ffL * 22   / (22   + R_UP))     //6.2k
#define TCFG_ADKEY_AD3      (0x3ffL * 47   / (47   + R_UP))     //9.1k
#define TCFG_ADKEY_AD4      (0x3ffL * 68  / (68  + R_UP))     //15k
#define TCFG_ADKEY_AD5      (0x3ffL * 100  / (100  + R_UP))     //24k
#define TCFG_ADKEY_AD6      (0x3ffL)     //33k
#define TCFG_ADKEY_AD7      (0x3ffL)     //51k
#define TCFG_ADKEY_AD8      (0x3ffL)     //100k
#define TCFG_ADKEY_AD9      (0x3ffL)     //220k
#define TCFG_ADKEY_VDDIO    (0x3ffL)

#define TCFG_ADKEY_VOLTAGE0 ((TCFG_ADKEY_AD0 + TCFG_ADKEY_AD1) / 2)
#define TCFG_ADKEY_VOLTAGE1 ((TCFG_ADKEY_AD1 + TCFG_ADKEY_AD2) / 2)
#define TCFG_ADKEY_VOLTAGE2 ((TCFG_ADKEY_AD2 + TCFG_ADKEY_AD3) / 2)
#define TCFG_ADKEY_VOLTAGE3 ((TCFG_ADKEY_AD3 + TCFG_ADKEY_AD4) / 2)
#define TCFG_ADKEY_VOLTAGE4 ((TCFG_ADKEY_AD4 + TCFG_ADKEY_AD5) / 2)
#define TCFG_ADKEY_VOLTAGE5 ((TCFG_ADKEY_AD5 + TCFG_ADKEY_AD6) / 2)
#define TCFG_ADKEY_VOLTAGE6 ((TCFG_ADKEY_AD6 + TCFG_ADKEY_AD7) / 2)
#define TCFG_ADKEY_VOLTAGE7 ((TCFG_ADKEY_AD7 + TCFG_ADKEY_AD8) / 2)
#define TCFG_ADKEY_VOLTAGE8 ((TCFG_ADKEY_AD8 + TCFG_ADKEY_AD9) / 2)
#define TCFG_ADKEY_VOLTAGE9 ((TCFG_ADKEY_AD9 + TCFG_ADKEY_VDDIO) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   4
#define TCFG_ADKEY_VALUE5                   5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   9

//*********************************************************************************//
//                                 irkey 配置                                      //
//*********************************************************************************//
#define TCFG_IRKEY_ENABLE                   DISABLE_THIS_MOUDLE//是否使能ir按键
#define TCFG_IRKEY_PORT                     IO_PORTA_02        //IR按键端口

//*********************************************************************************//
//                             tocuh key 配置 (不支持)                                      //
//*********************************************************************************//
//#define TCFG_TOUCH_KEY_ENABLE 				ENABLE_THIS_MOUDLE 		//是否使能触摸按键
#define TCFG_TOUCH_KEY_ENABLE 				DISABLE_THIS_MOUDLE 		//是否使能触摸按键

/* 触摸按键计数参考时钟选择, 频率越高, 精度越高
** 可选参数:
	1.TOUCH_KEY_OSC_CLK,
    2.TOUCH_KEY_MUX_IN_CLK,  //外部输入, ,一般不用, 保留
    3.TOUCH_KEY_PLL_192M_CLK,
    4.TOUCH_KEY_PLL_240M_CLK,
*/
#define TCFG_TOUCH_KEY_CLK 					TOUCH_KEY_PLL_192M_CLK 	//触摸按键时钟配置
#define TCFG_TOUCH_KEY_CHANGE_GAIN 			4 	//变化放大倍数, 一般固定
#define TCFG_TOUCH_KEY_PRESS_CFG 			-100//触摸按下灵敏度, 类型:s16, 数值越大, 灵敏度越高
#define TCFG_TOUCH_KEY_RELEASE_CFG0 		-50 //触摸释放灵敏度0, 类型:s16, 数值越大, 灵敏度越高
#define TCFG_TOUCH_KEY_RELEASE_CFG1 		-80 //触摸释放灵敏度1, 类型:s16, 数值越大, 灵敏度越高

//key0配置
#define TCFG_TOUCH_KEY0_PORT 				IO_PORTB_06  //触摸按键IO配置
#define TCFG_TOUCH_KEY0_VALUE 				1 		 	 //触摸按键key0 按键值

//key1配置
#define TCFG_TOUCH_KEY1_PORT 				IO_PORTB_07  //触摸按键key1 IO配置
#define TCFG_TOUCH_KEY1_VALUE 				2 		 	 //触摸按键key1按键值

//*********************************************************************************//
//                            ctmu tocuh key 配置 (不支持)                                     //
//*********************************************************************************//
#define TCFG_CTMU_TOUCH_KEY_ENABLE              DISABLE_THIS_MOUDLE             //是否使能CTMU触摸按键
//key0配置
#define TCFG_CTMU_TOUCH_KEY0_PORT 				IO_PORTB_06  //触摸按键key0 IO配置
#define TCFG_CTMU_TOUCH_KEY0_VALUE 				0 		 	 //触摸按键key0 按键值

//key1配置
#define TCFG_CTMU_TOUCH_KEY1_PORT 				IO_PORTB_07  //触摸按键key1 IO配置
#define TCFG_CTMU_TOUCH_KEY1_VALUE 				1 		 	 //触摸按键key1 按键值

//*********************************************************************************//
//                                 rdec_key 配置                                      //
//*********************************************************************************//
#define TCFG_RDEC_KEY_ENABLE					DISABLE_THIS_MOUDLE //是否使能RDEC按键
//RDEC0配置
#define TCFG_RDEC0_ECODE1_PORT					IO_PORTA_03
#define TCFG_RDEC0_ECODE2_PORT					IO_PORTA_04
#define TCFG_RDEC0_KEY0_VALUE 				 	0
#define TCFG_RDEC0_KEY1_VALUE 				 	1

//RDEC1配置
#define TCFG_RDEC1_ECODE1_PORT					IO_PORTB_02
#define TCFG_RDEC1_ECODE2_PORT					IO_PORTB_03
#define TCFG_RDEC1_KEY0_VALUE 				 	2
#define TCFG_RDEC1_KEY1_VALUE 				 	3

//RDEC2配置
#define TCFG_RDEC2_ECODE1_PORT					IO_PORTB_04
#define TCFG_RDEC2_ECODE2_PORT					IO_PORTB_05
#define TCFG_RDEC2_KEY0_VALUE 				 	4
#define TCFG_RDEC2_KEY1_VALUE 				 	5

//*********************************************************************************//
//                                 Audio配置                                       //
//*********************************************************************************//
#define TCFG_AUDIO_ADC_ENABLE				ENABLE_THIS_MOUDLE
//MIC只有一个声道，固定选择右声道
#define TCFG_AUDIO_ADC_MIC_CHA				LADC_CH_MIC_R
/*MIC LDO电流档位设置：
    0:0.625ua    1:1.25ua    2:1.875ua    3:2.5ua*/
#define TCFG_AUDIO_ADC_LDO_SEL				2

// LADC通道
#define TCFG_AUDIO_ADC_LINE_CHA0			LADC_LINE1_MASK
#define TCFG_AUDIO_ADC_LINE_CHA1			LADC_CH_LINE0_L

#define TCFG_AUDIO_DAC_ENABLE				ENABLE_THIS_MOUDLE
#define TCFG_AUDIO_DAC_LDO_SEL				1
/*
DACVDD电压设置(要根据具体的硬件接法来确定):
    DACVDD_LDO_1_20V        DACVDD_LDO_1_30V        DACVDD_LDO_2_35V        DACVDD_LDO_2_50V
    DACVDD_LDO_2_65V        DACVDD_LDO_2_80V        DACVDD_LDO_2_95V        DACVDD_LDO_3_10V*/
#define TCFG_AUDIO_DAC_LDO_VOLT				DACVDD_LDO_2_90V
/*预留接口，未使用*/
#define TCFG_AUDIO_DAC_PA_PORT				NO_CONFIG_PORT
/*
DAC硬件上的连接方式,可选的配置：
    DAC_OUTPUT_MONO_L               左声道
    DAC_OUTPUT_MONO_R               右声道
    DAC_OUTPUT_LR                   立体声
    DAC_OUTPUT_MONO_LR_DIFF         单声道差分输出
*/
#define TCFG_AUDIO_DAC_CONNECT_MODE    DAC_OUTPUT_MONO_L

/*
解码后音频的输出方式:
    AUDIO_OUTPUT_ORIG_CH            按原始声道输出
    AUDIO_OUTPUT_STEREO             按立体声
    AUDIO_OUTPUT_L_CH               只输出原始声道的左声道
    AUDIO_OUTPUT_R_CH               只输出原始声道的右声道
    AUDIO_OUTPUT_MONO_LR_CH         输出左右合成的单声道
 */

#define AUDIO_OUTPUT_MODE          AUDIO_OUTPUT_MONO_LR_CH

#define AUDIO_OUTPUT_WAY_DAC        0
#define AUDIO_OUTPUT_WAY_IIS        1
#define AUDIO_OUTPUT_WAY_FM         2
#define AUDIO_OUTPUT_WAY_HDMI       3
#define AUDIO_OUTPUT_WAY_SPDIF      4
#define AUDIO_OUTPUT_WAY_BT      	5	// bt emitter
#define AUDIO_OUTPUT_WAY_DAC_IIS    6
#define AUDIO_OUTPUT_WAY_DONGLE		7
#define AUDIO_OUTPUT_WAY            AUDIO_OUTPUT_WAY_DAC
#define LINEIN_INPUT_WAY            LINEIN_INPUT_WAY_ANALOG

#define AUDIO_OUTPUT_AUTOMUTE       0//ENABLE


/*
 *系统音量类型选择
 *软件数字音量是指纯软件对声音进行运算后得到的
 *硬件数字音量是指dac内部数字模块对声音进行运算后输出
 */
#define VOL_TYPE_DIGITAL		0	//软件数字音量
#define VOL_TYPE_ANALOG			1	//硬件模拟音量
#define VOL_TYPE_AD				2	//联合音量(模拟数字混合调节)
#define VOL_TYPE_DIGITAL_HW		3  	//硬件数字音量
#define VOL_TYPE_DIGGROUP       4   //独立通道数字音量

//每个解码通道都开启数字音量管理,音量类型为VOL_TYPE_DIGGROUP时要使能
#define SYS_DIGVOL_GROUP_EN     DISABLE

#define SYS_VOL_TYPE            VOL_TYPE_AD

#if  (SYS_VOL_TYPE == VOL_TYPE_DIGGROUP)
#undef SYS_DIGVOL_GROUP_EN
#define SYS_DIGVOL_GROUP_EN     ENABLE
#endif



/*
 *通话的时候使用数字音量
 *0：通话使用和SYS_VOL_TYPE一样的音量调节类型
 *1：通话使用数字音量调节，更加平滑
 */
#define TCFG_CALL_USE_DIGITAL_VOLUME		0

// 使能改宏，提示音音量使用music音量
#define APP_AUDIO_STATE_WTONE_BY_MUSIC      (1)
// 0:提示音不使用默认音量； 1:默认提示音音量值
#define TONE_MODE_DEFAULE_VOLUME            (0)

/*
 *支持省电容MIC模块
 *(1)要使能省电容mic,首先要支持该模块:TCFG_SUPPORT_MIC_CAPLESS
 *(2)只有支持该模块，才能使能该模块:TCFG_MIC_CAPLESS_ENABLE
 */
#define TCFG_SUPPORT_MIC_CAPLESS			ENABLE_THIS_MOUDLE
//省电容MIC使能
#define TCFG_MIC_CAPLESS_ENABLE				DISABLE_THIS_MOUDLE
//*********************************************************************************//
//                                  充电仓配置  (不支持)                                   //
//*********************************************************************************//
#define TCFG_CHARGESTORE_ENABLE				DISABLE_THIS_MOUDLE       //是否支持智能充点仓
#define TCFG_TEST_BOX_ENABLE			    0
#define TCFG_CHARGESTORE_PORT				IO_PORTA_02               //耳机和充点仓通讯的IO口
#define TCFG_CHARGESTORE_UART_ID			IRQ_UART1_IDX             //通讯使用的串口号

#ifdef AUDIO_PCM_DEBUG
#ifdef	TCFG_TEST_BOX_ENABLE
#undef 	TCFG_TEST_BOX_ENABLE
#define TCFG_TEST_BOX_ENABLE				0		//因为使用PCM使用到了串口1
#endif
#endif/*AUDIO_PCM_DEBUG*/

//*********************************************************************************//
//                                  LED 配置                                       //
//******************************************************************************
#if TCFG_ADKEY_LED_IO_REUSE
//打开ADKEY和LED IO复用功能，LED使用ADKEY_IO
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						TCFG_ADKEY_PORT						//LED使用的IO口

#else

#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTB_06					//LED使用的IO口 注意和led7是否有io冲突

#endif
//*********************************************************************************//
//                                  UI 配置                                        //
//*********************************************************************************//
#define TCFG_UI_ENABLE 						ENABLE_THIS_MOUDLE 	//UI总开关
#define CONFIG_UI_STYLE                     STYLE_JL_LED7
#define TCFG_UI_LED7_ENABLE 			 	ENABLE_THIS_MOUDLE 	//UI使用LED7显示
// #define TCFG_UI_LCD_SEG3X9_ENABLE 		ENABLE_THIS_MOUDLE 	//UI使用LCD段码屏显示
// #define TCFG_LCD_ST7735S_ENABLE	        ENABLE_THIS_MOUDLE
// #define TCFG_LCD_ST7789VW_ENABLE	        ENABLE_THIS_MOUDLE
#define TCFG_SPI_LCD_ENABLE                 DISABLE_THIS_MOUDLE //spi lcd开关
#define TCFG_TFT_LCD_DEV_SPI_HW_NUM			 1// 1: SPI1    2: SPI2 配置lcd选择的spi口
#define TCFG_LED7_RUN_RAM 					DISABLE_THIS_MOUDLE 	//led7跑ram 不屏蔽中断(需要占据2k附近ram)
//*********************************************************************************//
//                                  时钟配置                                       //
//*********************************************************************************//
#define TCFG_CLOCK_SYS_SRC					SYS_CLOCK_INPUT_PLL_BT_OSC   //系统时钟源选择
#define TCFG_CLOCK_SYS_HZ					24000000                     //系统时钟设置
#define TCFG_CLOCK_OSC_HZ					24000000                     //外界晶振频率设置
#define TCFG_CLOCK_MODE                     CLOCK_MODE_ADAPTIVE

//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15                    //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                            //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			0//SLEEP_EN                     //SNIFF状态下芯片是否进入powerdown
/*强VDDIO等级配置,可选：
    VDDIOM_VOL_20V    VDDIOM_VOL_22V    VDDIOM_VOL_24V    VDDIOM_VOL_26V
    VDDIOM_VOL_30V    VDDIOM_VOL_30V    VDDIOM_VOL_32V    VDDIOM_VOL_34V*/
#if ((TCFG_SD0_ENABLE) ||(TCFG_SD1_ENABLE))
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_34V    //VDDIO 设置的值要和vbat的压差要大于300mv左右，否则会出现DAC杂音
#else
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V    //VDDIO 设置的值要和vbat的压差要大于300mv左右，否则会出现DAC杂音
#endif
/*弱VDDIO等级配置，可选：
    VDDIOW_VOL_21V    VDDIOW_VOL_24V    VDDIOW_VOL_28V    VDDIOW_VOL_32V*/
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_28V               //弱VDDIO等级配置

//*********************************************************************************//
//                                  EQ配置                                         //
//*********************************************************************************//
//EQ配置，使用在线EQ时，EQ文件和EQ模式无效。有EQ文件时，使能TCFG_USE_EQ_FILE,默认不用EQ模式切换功能
#define TCFG_EQ_ENABLE                      0     //支持EQ功能,EQ总使能
#if TCFG_EQ_ENABLE
#define TCFG_EQ_ONLINE_ENABLE               0     //支持在线EQ调试,sdk默认使用uart串口调试，如需使用spp调试，需使能 APP_ONLINE_DEBUG(二选一)
#define TCFG_BT_MUSIC_EQ_ENABLE             1      //支持蓝牙音乐EQ
#define TCFG_PHONE_EQ_ENABLE                1      //支持通话近端EQ
#define TCFG_MUSIC_MODE_EQ_ENABLE           1     //支持音乐模式EQ
#define TCFG_LINEIN_MODE_EQ_ENABLE          1     //支持linein近端EQ
#define TCFG_FM_MODE_EQ_ENABLE              0     //支持fm模式EQ
#define TCFG_SPDIF_MODE_EQ_ENABLE           1     //支持SPDIF模式EQ
#define TCFG_PC_MODE_EQ_ENABLE              1     //支持pc模式EQ
#define TCFG_AUDIO_OUT_EQ_ENABLE			0 	  //mix_out后高低音EQ

#define TCFG_USE_EQ_FILE                    0    //离线eq使用配置文件还是默认系数表 1：使用文件  0 使用默认系数表
#if !TCFG_USE_EQ_FILE
#define TCFG_USER_EQ_MODE_NUM               7    //eq默认系数表的模式个数，默认是7个
#endif

#define EQ_SECTION_MAX                      10    //eq段数
#endif//TCFG_EQ_ENABLE

#define TCFG_DRC_ENABLE						0 	  //DRC 总使能
#if TCFG_DRC_ENABLE
#define TCFG_BT_MUSIC_DRC_ENABLE            1     //支持蓝牙音乐DRC
#define TCFG_MUSIC_MODE_DRC_ENABLE          1     //支持音乐模式DRC
#define TCFG_LINEIN_MODE_DRC_ENABLE         1     //支持LINEIN模式DRC
#define TCFG_FM_MODE_DRC_ENABLE             0     //支持FM模式DRC
#define TCFG_SPDIF_MODE_DRC_ENABLE          1     //支持SPDIF模式DRC
#define TCFG_PC_MODE_DRC_ENABLE             1     //支持PC模式DRC
#define TCFG_AUDIO_OUT_DRC_ENABLE			0 	  //mix_out后drc
#endif//TCFG_DRC_ENABLE

// ONLINE CCONFIG
// 如果调试串口是DP DM,使用eq调试串口时，需关闭usb宏
#define TCFG_ONLINE_ENABLE                  (TCFG_EQ_ONLINE_ENABLE)    //是否支持EQ在线调试功能
#define TCFG_ONLINE_TX_PORT					IO_PORT_DP                 //EQ调试TX口选择
#define TCFG_ONLINE_RX_PORT					IO_PORT_DM                 //EQ调试RX口选择

/*在线调试，需要打开SPP和在线调试功能*/
#define APP_ONLINE_DEBUG                    0

/***********************************非用户配置区***********************************/
#if TCFG_EQ_ONLINE_ENABLE
#if APP_ONLINE_DEBUG
#undef TCFG_ONLINE_ENABLE
#define TCFG_ONLINE_ENABLE  0
#endif

#if (TCFG_USE_EQ_FILE == 0)
#undef TCFG_USE_EQ_FILE
#define TCFG_USE_EQ_FILE                    1    //开在线调试时，打开使用离线配置文件宏定义
#endif
#if TCFG_AUDIO_OUT_EQ_ENABLE
#undef TCFG_AUDIO_OUT_EQ_ENABLE
#define TCFG_AUDIO_OUT_EQ_ENABLE            0    //开在线调试时，关闭高低音
#endif
#endif
/**********************************************************************************/


//*********************************************************************************//
//                                  mic effect 配置                                //
//*********************************************************************************//
#define TCFG_MIC_EFFECT_ENABLE       DISABLE
#define TCFG_MIC_EFFECT_DEBUG        0//调试打印
#define TCFG_MIC_EFFECT_ONLINE_ENABLE  0//混响音效在线调试使能
#if ((TCFG_ONLINE_ENABLE == 0) && TCFG_MIC_EFFECT_ONLINE_ENABLE)
#undef TCFG_ONLINE_ENABLE
#define TCFG_ONLINE_ENABLE 1
#endif

#define MIC_EFFECT_REVERB             0
#define MIC_EFFECT_ECHO               1
// #define TCFG_MIC_EFFECT_SEL           MIC_EFFECT_REVERB
#define TCFG_MIC_EFFECT_SEL           MIC_EFFECT_ECHO

#if TCFG_EQ_ENABLE
#define  MIC_EFFECT_EQ_EN             0//混响音效的EQ
#endif

#if TCFG_MIC_EFFECT_ENABLE
#ifdef TCFG_AEC_ENABLE
#undef TCFG_AEC_ENABLE
#define TCFG_AEC_ENABLE 0
#endif//TCFG_AEC_ENABLE
#endif

#define TCFG_REVERB_SAMPLERATE_DEFUAL (44100)
#define MIC_EFFECT_SAMPLERATE			(44100L)

#if TCFG_MIC_EFFECT_ENABLE
#undef MIC_SamplingFrequency
#define     MIC_SamplingFrequency         1
#undef MIC_AUDIO_RATE
#define     MIC_AUDIO_RATE              MIC_EFFECT_SAMPLERATE
#endif




/*********扩音器功能使用mic_effect.c混响流程，功能选配在effect_reg.c中 ***********/
/*********配置MIC_EFFECT_CONFIG宏定义即可********************************/
#define TCFG_LOUDSPEAKER_ENABLE            DISABLE //扩音器功能使能
#define TCFG_USB_MIC_ECHO_ENABLE           DISABLE //不能与TCFG_MIC_EFFECT_ENABLE同时打开
#define TCFG_USB_MIC_DATA_FROM_MICEFFECT   DISABLE //要确保开usbmic前已经开启混响

//*********************************************************************************//
//                                  g-sensor配置                                   //
//*********************************************************************************//
#define TCFG_GSENSOR_ENABLE                       0     //gSensor使能
#define TCFG_DA230_EN                             0
#define TCFG_SC7A20_EN                            0
#define TCFG_STK8321_EN                           0
#define TCFG_GSENOR_USER_IIC_TYPE                 0     //0:软件IIC  1:硬件IIC

//*********************************************************************************//
//                                  系统配置                                         //
//*********************************************************************************//
#define TCFG_AUTO_SHUT_DOWN_TIME		    0   //没有蓝牙连接自动关机时间
#define TCFG_SYS_LVD_EN						1   //电量检测使能
#define TCFG_POWER_ON_NEED_KEY				0	  //是否需要按按键开机配置
#define TWFG_APP_POWERON_IGNORE_DEV         4000//上电忽略挂载设备，0时不忽略，非0则n毫秒忽略


//*********************************************************************************//
//                                  蓝牙配置                                       //
//*********************************************************************************//
#define TCFG_USER_TWS_ENABLE                0   //tws功能使能
#define TCFG_USER_BLE_ENABLE                0   //BLE功能使能
#define TCFG_USER_BT_CLASSIC_ENABLE         1   //经典蓝牙功能使能
#define TCFG_BT_SUPPORT_AAC                 0   //AAC格式支持
#define TCFG_USER_EMITTER_ENABLE            0   //(暂不支持)emitter功能使能
#define TCFG_BT_SNIFF_ENABLE                0   //bt sniff 功能使能

#define USER_SUPPORT_PROFILE_SPP    0
#define USER_SUPPORT_PROFILE_HFP    0
#define USER_SUPPORT_PROFILE_A2DP   1
#define USER_SUPPORT_PROFILE_AVCTP  1
#define USER_SUPPORT_PROFILE_HID    0
#define USER_SUPPORT_PROFILE_PNP    1
#define USER_SUPPORT_PROFILE_PBAP   0


#define TCFG_VIRTUAL_FAST_CONNECT_FOR_EMITTER      0

#if TCFG_USER_TWS_ENABLE
#define TCFG_BD_NUM						    1   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME       0   //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#define TCFG_USER_ESCO_SLAVE_MUTE           1   //对箱通话slave出声音
#else
#define TCFG_BD_NUM						    1   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME       0 //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#define TCFG_USER_ESCO_SLAVE_MUTE           0   //对箱通话slave出声音
#endif

#define BT_INBAND_RINGTONE                  0   //是否播放手机自带来电铃声
#define BT_PHONE_NUMBER                     0   //是否播放来电报号
#define BT_SYNC_PHONE_RING                  1   //是否TWS同步播放来电铃声
#define BT_SUPPORT_DISPLAY_BAT              1   //是否使能电量检测
#define BT_SUPPORT_MUSIC_VOL_SYNC           1   //是否使能音量同步

#define TCFG_BLUETOOTH_BACK_MODE			0	//不支持后台模式

#if(TCFG_BLUETOOTH_BACK_MODE)
#error "ont support background mode!!!!"
#endif

#if (TCFG_BLUETOOTH_BACK_MODE == 0)
//696非后台解码库空间使用overlay
#ifdef TCFG_MEDIA_LIB_USE_MALLOC
#undef TCFG_MEDIA_LIB_USE_MALLOC
#define TCFG_MEDIA_LIB_USE_MALLOC			0
#endif//TCFG_MEDIA_LIB_USE_MALLOC
#endif//(TCFG_BLUETOOTH_BACK_MODE == 0)

#if (TCFG_USER_TWS_ENABLE && TCFG_BLUETOOTH_BACK_MODE) && (TCFG_BT_SNIFF_ENABLE==0) && defined(CONFIG_LOCAL_TWS_ENABLE)
#define TCFG_DEC2TWS_ENABLE					0
#define TCFG_PCM_ENC2TWS_ENABLE				0
#define TCFG_TONE2TWS_ENABLE				0
#else
#define TCFG_DEC2TWS_ENABLE					0
#define TCFG_PCM_ENC2TWS_ENABLE				0
#define TCFG_TONE2TWS_ENABLE				0
#endif

//#define TWS_PHONE_LONG_TIME_DISCONNECTED

#if (APP_ONLINE_DEBUG && !USER_SUPPORT_PROFILE_SPP)
#error "NEED ENABLE USER_SUPPORT_PROFILE_SPP!!!"
#endif


//*********************************************************************************//
//                                  REC 配置                                       //
//*********************************************************************************//
#define RECORDER_MIX_EN						DISABLE//混合录音使能, 需要录制例如蓝牙、FM、 LINEIN才开
#define TCFG_RECORD_FOLDER_DEV_ENABLE       DISABLE//ENABLE//音乐播放录音区分使能
#define RECORDER_MIX_BT_PHONE_EN			ENABLE//电话录音使能



//*********************************************************************************//
//                                  linein配置                                     //
//*********************************************************************************//
#define TCFG_LINEIN_ENABLE					TCFG_APP_LINEIN_EN	// linein使能
// #define TCFG_LINEIN_LADC_IDX				0					// linein使用的ladc通道，对应ladc_list
#if (RECORDER_MIX_EN)
#define TCFG_LINEIN_LR_CH					AUDIO_LIN0L_CH//AUDIO_LIN0_LR
#else
#define TCFG_LINEIN_LR_CH					AUDIO_LIN0_LR
#endif/*RECORDER_MIX_EN*/
#define TCFG_LINEIN_CHECK_PORT				IO_PORTB_01			// linein检测IO
#define TCFG_LINEIN_PORT_UP_ENABLE        	1					// 检测IO上拉使能
#define TCFG_LINEIN_PORT_DOWN_ENABLE       	0					// 检测IO下拉使能
#define TCFG_LINEIN_AD_CHANNEL             	NO_CONFIG_PORT		// 检测IO是否使用AD检测
#define TCFG_LINEIN_VOLTAGE                	0					// AD检测时的阀值
#if(TCFG_MIC_EFFECT_ENABLE)
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ANALOG
#else
#if (RECORDER_MIX_EN)
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ADC//LINEIN_INPUT_WAY_ANALOG
#else
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ANALOG
#endif/*RECORDER_MIX_EN*/
#endif
#define TCFG_LINEIN_MULTIPLEX_WITH_FM		DISABLE 				// linein 脚与 FM 脚复用
#define TCFG_LINEIN_MULTIPLEX_WITH_SD		DISABLE 				// linein 检测与 SD cmd 复用
#define TCFG_LINEIN_SD_PORT		            0// 0:sd0 1:sd1     //选择复用的sd

//*********************************************************************************//
//                                  music 配置                                     //
//*********************************************************************************//
#define TCFG_DEC_G729_ENABLE                ENABLE
#define TCFG_DEC_MP3_ENABLE					ENABLE
#define TCFG_DEC_WMA_ENABLE					ENABLE
#define TCFG_DEC_WAV_ENABLE					ENABLE
#define TCFG_DEC_FLAC_ENABLE				DISABLE
#define TCFG_DEC_APE_ENABLE					DISABLE
#define TCFG_DEC_M4A_ENABLE					DISABLE
#define TCFG_DEC_ALAC_ENABLE				DISABLE
#define TCFG_DEC_AMR_ENABLE					DISABLE
#define TCFG_DEC_DTS_ENABLE					DISABLE
#define TCFG_DEC_MIDI_ENABLE                DISABLE
#define TCFG_DEC_G726_ENABLE                DISABLE
#define TCFG_DEC_MTY_ENABLE					DISABLE
#define TCFG_DEC_WTGV2_ENABLE				DISABLE


#define TCFG_DEC_ID3_V1_ENABLE				DISABLE
#define TCFG_DEC_ID3_V2_ENABLE				DISABLE
#define TCFG_DEC_DECRYPT_ENABLE				DISABLE
#define TCFG_DEC_DECRYPT_KEY				(0x12345678)

////<变速变调
#define TCFG_SPEED_PITCH_ENABLE             DISABLE//
//*********************************************************************************//
//                                  fm 配置                                     //
//*********************************************************************************//
#define TCFG_FM_ENABLE							TCFG_APP_FM_EN // fm 使能
#define TCFG_FM_INSIDE_ENABLE					ENABLE
#define TCFG_FM_RDA5807_ENABLE					DISABLE
#define TCFG_FM_BK1080_ENABLE					DISABLE
#define TCFG_FM_QN8035_ENABLE					DISABLE

#define TCFG_FMIN_LADC_IDX				1				// linein使用的ladc通道，对应ladc_list
#define TCFG_FMIN_LR_CH					AUDIO_LIN1_LR
#define TCFG_FM_INPUT_WAY               LINEIN_INPUT_WAY_ANALOG

#if (TCFG_FM_INSIDE_ENABLE && TCFG_FM_ENABLE)
#if (((TCFG_USER_TWS_ENABLE) || (RECORDER_MIX_EN) || (TCFG_MIC_EFFECT_ENABLE)))
#define TCFG_CODE_RUN_RAM_FM_MODE 					DISABLE_THIS_MOUDLE  	//FM模式 代码跑ram
#else
#define TCFG_CODE_RUN_RAM_FM_MODE 					ENABLE_THIS_MOUDLE  	//FM模式 代码跑ram
#endif
#else
#define TCFG_CODE_RUN_RAM_FM_MODE 					DISABLE_THIS_MOUDLE 	//FM模式 代码跑ram
#endif /*(TCFG_FM_INSIDE_ENABLE && TCFG_FM_ENABLE)*/

#if (TCFG_CODE_RUN_RAM_FM_MODE && TCFG_UI_ENABLE)
#undef TCFG_LED7_RUN_RAM
#define TCFG_LED7_RUN_RAM 					ENABLE_THIS_MOUDLE 	//led7跑ram 不屏蔽中断(需要占据2k附近ram)
#endif /*(TCFG_CODE_RUN_RAM_FM_MODE && TCFG_UI_ENABLE)*/

//*********************************************************************************//
//                                  fm emitter 配置 (不支持)                                    //
//*********************************************************************************//
#define TCFG_APP_FM_EMITTER_EN                  DISABLE_THIS_MOUDLE
#define TCFG_FM_EMITTER_INSIDE_ENABLE			DISABLE
#define TCFG_FM_EMITTER_AC3433_ENABLE			DISABLE
#define TCFG_FM_EMITTER_QN8007_ENABLE			DISABLE
#define TCFG_FM_EMITTER_QN8027_ENABLE			DISABLE

//*********************************************************************************//
//                                  rtc 配置(不支持)                               //
//*********************************************************************************//
#define TCFG_RTC_ENABLE						TCFG_APP_RTC_EN

#if TCFG_RTC_ENABLE
#define TCFG_USE_VIRTUAL_RTC                   ENABLE
#else
#define TCFG_USE_VIRTUAL_RTC                   DISABLE
#endif

//*********************************************************************************//
//                                  SPDIF & ARC 配置(不支持)                                     //
//*********************************************************************************//
#define TCFG_SPDIF_ENABLE                       TCFG_APP_SPDIF_EN
#define TCFG_SPDIF_OUTPUT_ENABLE                ENABLE
#define TCFG_HDMI_ARC_ENABLE                    ENABLE
#define TCFG_HDMI_CEC_PORT                      IO_PORTA_02

//*********************************************************************************//
//                                  IIS 配置                                       //
//  注意, iis 与其他模块的 io 互斥，                                               //
//  例如与 mic bias， aux0_L， aux0_R 等                                           //
//  另外因为某些bug， PA1 io 不要用作iis功能。                                     //
//  ALINK0_PORTA //MCLK:PA2 SCLK:PA3  LRCK:PA4 CH0:PA0 CH1:PA1 CH2:PA5 CH3:PA6     //
//  ALINK0_PORTB //MCLK:PC2 SCLK:PA5  LRCK:PA6 CH0:PA0 CH1:PA2 CH2:PA3 CH3:PA4     //
//*********************************************************************************//
#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_IIS) || (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DAC_IIS)
#define TCFG_IIS_ENABLE                       ENABLE_THIS_MOUDLE
#else
#define TCFG_IIS_ENABLE                       DISABLE_THIS_MOUDLE
#endif

#define TCFG_IIS_MODE                         (0)   //  0:master  1:slave

#define TCFG_IIS_OUTPUT_EN                    (ENABLE && TCFG_IIS_ENABLE)   // 输出使能
#define TCFG_IIS_OUTPUT_PORT                  ALINK0_PORTA                  // 端口选择
#define TCFG_IIS_OUTPUT_CH_NUM                1 //0:mono,1:stereo           // 只支持立体声
#define TCFG_IIS_OUTPUT_SR                    44100                         // 采样率(如果输出输入是同一个端口需要相同采样率)
#define TCFG_IIS_OUTPUT_DATAPORT_SEL          0                             // 选择通道 0

#define TCFG_IIS_INPUT_EN                    (DISABLE && TCFG_IIS_ENABLE)   // 输入使能
#define TCFG_IIS_INPUT_PORT                  ALINK0_PORTA                   // 端口选择
#define TCFG_IIS_INPUT_CH_NUM                1 //0:mono,1:stereo            // 只支持立体声
#define TCFG_IIS_INPUT_SR                    44100                          // 采样率(如果输出输入是同一个端口需要相同采样率)
#define TCFG_IIS_INPUT_DATAPORT_SEL          2                              // 选择通道 2

//*********************************************************************************//
//                                  fat 文件系统配置                                       //
//*********************************************************************************//
#define CONFIG_FATFS_ENABLE					ENABLE




//*********************************************************************************//
//                                  encoder 配置                                   //
//*********************************************************************************//
#define TCFG_ENC_CVSD_ENABLE                ENABLE
#define TCFG_ENC_MSBC_ENABLE                ENABLE
#define TCFG_ENC_MP3_ENABLE                 ENABLE
#define TCFG_ENC_ADPCM_ENABLE               ENABLE
#define TCFG_ENC_SBC_ENABLE                 ENABLE
#define TCFG_ENC_OPUS_ENABLE                DISABLE
#define TCFG_ENC_SPEEX_ENABLE               DISABLE

//*********************************************************************************//
//ali ai profile
#define DUEROS_DMA_EN              0  //not surport
#define TRANS_DATA_EN              0  //not surport
#define	ANCS_CLIENT_EN			   0

#if (DUEROS_DMA_EN || TRANS_DATA_EN || ANCS_CLIENT_EN)
#define BT_FOR_APP_EN			   1
#else
#define BT_FOR_APP_EN			   0
#endif

//*********************************************************************************//
//                                 电源切换配置                                    //
//*********************************************************************************//

#define CONFIG_PHONE_CALL_USE_LDO15	    1

//*********************************************************************************//
//                                人声消除使能
//*********************************************************************************//
#define AUDIO_VOCAL_REMOVE_EN       0

///*********************************************************************************//
//          等响度 开启后，需要固定模拟音量,调节软件数字音量
//          等响度使用eq实现，同个数据流中，若打开等响度，请开eq总使能，关闭其他eq,例如蓝牙模式eq
//*********************************************************************************//
#define AUDIO_EQUALLOUDNESS_CONFIG  0  //等响度
#define AUDIO_VBASS_CONFIG  0  //虚拟低音
#define AUDIO_SURROUND_CONFIG  0  //环绕音效
#define AUDIO_SPECTRUM_CONFIG  0  //频响能量值获取接口
#define AUDIO_MIDI_CTRL_CONFIG 0  //midi电子琴接口使能


//*********************************************************************************//
//                               解码独立任务设置，需要消耗额外的ram，慎用！
//*********************************************************************************//
#if (TCFG_MIC_EFFECT_ENABLE && (TCFG_DEC_APE_ENABLE || TCFG_DEC_FLAC_ENABLE || TCFG_DEC_DTS_ENABLE))
#define TCFG_AUDIO_DEC_OUT_TASK				1	// 解码使用单独任务做输出
#else
#define TCFG_AUDIO_DEC_OUT_TASK				0	// 解码使用单独任务做输出
#endif

#define	PA_CONTROL_PIN	IO_PORTA_00
#define	PA_AB_D_CONTROL_PIN	IO_PORTB_04
#define	CHARGE_DETECT_PIN	IO_PORTB_06

//*********************************************************************************//
//                                 编译警告                                         //
//*********************************************************************************//
#if ((ANCS_CLIENT_EN || TRANS_DATA_EN || ((TCFG_ONLINE_TX_PORT == IO_PORT_DP) && TCFG_ONLINE_ENABLE)) && (TCFG_PC_ENABLE || TCFG_UDISK_ENABLE || TCFG_SD0_PORTS == 'E'))
#error "eq online adjust enable, plaease close usb marco  and sdcard port not e!!!"
#endif// ((TRANS_DATA_EN || TCFG_ONLINE_ENABLE) && (TCFG_PC_ENABLE || TCFG_UDISK_ENABLE))

#if TCFG_UI_ENABLE
#if ((TCFG_SPI_LCD_ENABLE &&  TCFG_CODE_FLASH_ENABLE) && (TCFG_FLASH_DEV_SPI_HW_NUM == TCFG_TFT_LCD_DEV_SPI_HW_NUM))
#error "flash spi port == lcd spi port, please close one !!!"
#endif//((TCFG_SPI_LCD_ENABLE &&  TCFG_CODE_FLASH_ENABLE) && (TCFG_FLASH_DEV_SPI_HW_NUM == TCFG_TFT_LCD_DEV_SPI_HW_NUM))
#endif//TCFG_UI_ENABLE

#if((TRANS_DATA_EN + DUEROS_DMA_EN + ANCS_CLIENT_EN) > 1)
#error "they can not enable at the same time,just select one!!!"
#endif//(TRANS_DATA_EN && DUEROS_DMA_EN)

#if (TCFG_DEC2TWS_ENABLE && (TCFG_APP_RECORD_EN || TCFG_APP_RTC_EN ||TCFG_DRC_ENABLE))
#error "对箱支持音源转发，请关闭录音等功能 !!!"
#endif// (TCFG_DEC2TWS_ENABLE && (TCFG_APP_RECORD_EN || TCFG_APP_RTC_EN ||TCFG_DRC_ENABLE))

#if (TCFG_DRC_ENABLE && TCFG_MIC_EFFECT_ENABLE && (TCFG_DEC_APE_ENABLE || TCFG_DEC_FLAC_ENABLE || TCFG_DEC_DTS_ENABLE))
#error "无损格式+混响不支持DRC !!!"
#endif//(TCFG_MIC_EFFECT_ENABLE && (TCFG_DEC_APE_ENABLE || TCFG_DEC_FLAC_ENABLE || TCFG_DEC_DTS_ENABLE))


#if ((TCFG_NORFLASH_DEV_ENABLE || TCFG_NOR_FS_ENABLE) &&  TCFG_UI_ENABLE)
#error "引脚复用问题，使用norflash需要关闭UI ！！！"
#endif


#if ((TCFG_APP_RECORD_EN) && (TCFG_USER_TWS_ENABLE))
#error "TWS 暂不支持录音功能"
#endif

#include "app_config.h"
#if ((TCFG_SD0_ENABLE) && (TCFG_SD0_PORTS == 'D') && ((RECORDER_MIX_EN) || (TCFG_SD0_DET_MODE == SD_CMD_DECT)))
/*
 1.如果有FM模式下录音的功能，即FM和SD同时工作的情况，那么SD IO就不能用PB0,PB2,PB3这组口。
 2.如果FM模式没有录音的功能，即FM和SD不会有同时工作的情况，那么 SD IO可以使用PB0,PB2,PB3这组口，
   但SD就不能使用CMD检测。要用CLK或IO检测，这样就要有硬件上的支持，如3.3K电阻或者牺牲另一个引脚做IO检测*
 */
#error "SD IO使用D组口 引脚干扰FM的问题 ！！！"
#endif


#if (RECORDER_MIX_BT_PHONE_EN && TCFG_RECORD_FOLDER_DEV_ENABLE)
#error "AC696由于资源不足， 不能同时打开此两个功能！！！"
#endif


#if (((TCFG_USER_TWS_ENABLE) || (RECORDER_MIX_EN) || (TCFG_MIC_EFFECT_ENABLE)) && (TCFG_CODE_RUN_RAM_FM_MODE))
#error "复杂功能不支持 FM 代码放ram跑！！！"
#endif

#if ((TCFG_BD_NUM == 2)&&((TCFG_DEC_M4A_ENABLE)||(TCFG_DEC_ALAC_ENABLE)))
#error "1to2 不支持ma4、alac格式！！！"
#endif

///<<<<所有宏定义不要在编译警告后面定义！！！！！！！！！！！！！！！！！！！！！！！！！！
///<<<<所有宏定义不要在编译警告后面定义！！！！！！！！！！！！！！！！！！！！！！！！！！


//*********************************************************************************//
//                                 配置结束                                         //
//*********************************************************************************//


#endif //CONFIG_BOARD_AC693X_DEMO
#endif //CONFIG_BOARD_AC693X_DEMO_CFG_H
