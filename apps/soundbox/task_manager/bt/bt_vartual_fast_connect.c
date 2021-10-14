
/*************************************************************

    此文件函数主要是蓝牙快连，用于主从都是杰理方案，上层协议连接的是标准的蓝牙协议


**************************************************************/
#include "system/includes.h"
#include "app_config.h"
#include "bt_vartual_fast_connect.h"
#include "bt/bt_tws.h"
#include "classic/tws_pair.h"
#include "user_cfg.h"
#include "cpu.h"
#include "app_main.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_error.h"
#include "btstack/bluetooth.h"
#include "bt.h"
#include "clock_cfg.h"

#if TCFG_VIRTUAL_FAST_CONNECT_FOR_EMITTER

u8 tws_auto_pair_enable = 1;
u16 fast_conn_timer = 0;

void bt_set_tws_local_addr(u8 *addr);

u8 fast_conn_code_buf[6] = {0x47, 0x09, 0x77, 0x63, 0x8a, 0x9b};

u8 *tws_set_auto_pair_code(void)
{
    /* printf("\n\nget defualt pair code\n\n"); */
    return fast_conn_code_buf;
}


static void bt_fast_conn_delete_timer()
{
    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
    tws_close_tws_pair();

    if (fast_conn_timer) {
        sys_timeout_del(fast_conn_timer);
        fast_conn_timer = 0;
    }
}

static void fast_connect_task(void *p)
{
    int msg[8];


    while (1) {
        int ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
    }
}
static void tws_fast_conn_and_testbox_conn(void *_sw)
{
    int timeout = 0;
    int sw = (int)_sw;

    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
    tws_close_tws_pair();

    if (sw == 1) {
        timeout = 2000;
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
    } else {
        timeout = 4000;
        tws_fast_conn_with_pair(2);
    }

    sw++;

    if (sw > 1) {
        sw = 0;
    }

    fast_conn_timer =  sys_timeout_add((void *)sw, tws_fast_conn_and_testbox_conn, timeout);

}

/* void fast_connect_record_addr(u8 *addr) */
/* { */
/* #if TCFG_USER_EMITTER_ENABLE */
/*     tws_api_set_connect_aa('R'); */
/* #else */
/*     tws_api_set_connect_aa('L'); */
/* #endif */
/*     syscfg_write(CFG_TWS_REMOTE_ADDR, addr, 6); */
/* } */

int bt_virtual_fast_connect_poweron(u8 type_role)
{
    fast_conn_timer = 0;

    bt_set_tws_local_addr(bt_get_mac_addr());

    task_create(fast_connect_task, NULL, "tws");

#if TCFG_USER_EMITTER_ENABLE
    tws_fast_conn_with_pair(1);
#else
    tws_fast_conn_and_testbox_conn(0);
#endif

    return 0;
}



u8 bt_fast_conn_hci_event_filter(struct bt_event *bt)
{
    switch (bt->event) {
    case HCI_EVENT_VENDOR_REMOTE_TEST:
#if (TCFG_USER_EMITTER_ENABLE == 0)
        if (0 == bt->value) {
            tws_fast_conn_and_testbox_conn(0);
        } else {
            bt_fast_conn_delete_timer();
        }
#endif
        break;
    case HCI_EVENT_USER_CONFIRMATION_REQUEST:
        printf(" HCI_EVENT_USER_CONFIRMATION_REQUEST \n");
        ///<可通过按键来确认是否配对 1：配对   0：取消
        bt_send_pair(1);
        clock_remove_set(BT_CONN_CLK);
        break;
    case HCI_EVENT_DISCONNECTION_COMPLETE :
#if TCFG_USER_EMITTER_ENABLE
        tws_fast_conn_with_pair(1);
#else
        tws_fast_conn_and_testbox_conn(0);
#endif
        break;
    case HCI_EVENT_CONNECTION_COMPLETE:
        printf(" HCI_EVENT_CONNECTION_COMPLETE \n");
        switch (bt->value) {
        case ERROR_CODE_SUCCESS :
            printf("ERROR_CODE_SUCCESS  \n");
#if (TCFG_USER_EMITTER_ENABLE == 0)
            bt_fast_conn_delete_timer();
#endif
            break;
        case ERROR_CODE_SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED :
        case ERROR_CODE_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES:
        case ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR:
        case ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED  :
        case ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION   :
        case ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST :
        case ERROR_CODE_AUTHENTICATION_FAILURE :
        case CUSTOM_BB_AUTO_CANCEL_PAGE:
        case ERROR_CODE_CONNECTION_TIMEOUT:
            printf(" ERROR_CODE_CONNECTION_TIMEOUT \n");
#if TCFG_USER_EMITTER_ENABLE
            tws_fast_conn_with_pair(1);
#else
            tws_fast_conn_and_testbox_conn(0);
#endif
            break;
        default:
            break;
        }
        break;
    }
    return 0;
}



#endif



