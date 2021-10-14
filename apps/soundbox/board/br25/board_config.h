#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 *  板级配置选择
 */


#ifdef  CONFIG_AC608N

#define CONFIG_BOARD_AC6082_DEMO
// #define CONFIG_BOARD_AC6082_IAP

#else

#define CONFIG_BOARD_AC696X_DEMO
// #define CONFIG_BOARD_AC6969D_DEMO
// #define CONFIG_BOARD_AC696X_LIGHTER
// #define CONFIG_BOARD_AC696X_TWS_BOX
// #define CONFIG_BOARD_AC696X_TWS
// #define CONFIG_BOARD_AC696X_SMARTBOX
// #define CONFIG_BOARD_AC6082_DEMO
// #define CONFIG_BOARD_AC696X_BTBOX
// #define CONFIG_BOARD_AC696X_BTEMITTER
// #define CONFIG_BOARD_AC636X_CHARGEBOX_OUTSIDE
// #define CONFIG_BOARD_AC636X_CHARGEBOX_INSIDE
// #define CONFIG_BOARD_AC6366C_CHARGEBOX_INSIDE
//#define CONFIG_BOARD_AC696X_LCD

#endif


#include "board_ac696x_demo/board_ac696x_demo_cfg.h"
//#include "board_ac696x_lcd/board_ac696x_lcd_cfg.h"
//#include "board_ac6969d_demo/board_ac6969d_demo_cfg.h"
//#include "board_ac696x_lighter/board_ac696x_lighter_cfg.h"
//#include "board_ac696x_tws_box/board_ac696x_tws_box.h"   //转发对箱
//#include "board_ac696x_tws/board_ac696x_tws.h"   //纯对箱
//#include "board_ac696x_smartbox/board_ac696x_smartbox.h"   // AI
//#include "board_ac6082_demo/board_ac6082_demo_cfg.h"
//#include "board_ac6082_iap/board_ac6082_iap_cfg.h"
//#include "board_ac696x_btbox/board_ac696x_btbox_cfg.h" //纯蓝牙单箱
//#include "board_ac696x_btemitter/board_ac696x_btemitter_cfg.h" //蓝牙发射
//#include "board_ac636x_chargebox_outside/board_ac636x_chargebox_outside_cfg.h" //外置充电充电舱
//#include "board_ac636x_chargebox_inside/board_ac636x_chargebox_inside_cfg.h" //内置充电充电舱
//#include "board_ac6366c_chargebox_inside/board_ac6366c_chargebox_inside_cfg.h" //内置充电充电舱(支持无线充电)

#define  DUT_AUDIO_DAC_LDO_VOLT   							DACVDD_LDO_2_90V

#endif
