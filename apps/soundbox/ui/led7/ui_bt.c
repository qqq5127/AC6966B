#include "includes.h"
#include "ui/ui_api.h"
#include "fm_emitter/fm_emitter_manage.h"
#include "btstack/avctp_user.h"
#include "system/sys_time.h"
#include "rtc/rtc_ui.h"

#if (TCFG_APP_BT_EN)

#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_JL_LED7))
struct rtc_ui_bt_opr {
    void *dev_handle;
    struct ui_rtc_display ui_rtc;
};

static u8 show_bt_delay_cnt = 0;

static struct rtc_ui_bt_opr *__this = NULL;

static void led7_show_bt(void *hd)
{
		struct sys_time current_time;
		
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->Clear_FlashChar(BIT(0) | BIT(1) | BIT(2) | BIT(3));
    dis->setXY(0, 0);

		if(show_bt_delay_cnt < 4)
		{
			dis->clear();
			dis->show_string((u8 *)" bt");
			show_bt_delay_cnt++;
		}
		else
		{
			u8 tmp_buf[5] = {0};
		
			dev_ioctl(__this->dev_handle, IOCTL_GET_SYS_TIME, (u32)&current_time);

			itoa2(current_time.hour, (u8 *)&tmp_buf[0]);
			itoa2(current_time.min, (u8 *)&tmp_buf[2]);
			dis->Clear_FlashChar(BIT(0) | BIT(1) | BIT(2) | BIT(3));

			dis->show_string(tmp_buf);
			dis->flash_icon(LED7_2POINT);
		}

		if(BT_STATUS_CONNECTING <= get_bt_connect_status())
		{
			dis->show_icon(LED7_BT);
		}
		else
		{
			dis->flash_icon(LED7_BT);
		}
	
		
    {		
			extern bool rtc_ui_get_alarm_status(void);

			
	    bool alarm_status;
			alarm_status = 	rtc_ui_get_alarm_status();
			if(alarm_status)
			{
				dis->show_icon(LED7_ALM);
			}
			
		}

    dis->show_icon(LED_BATTERY_NORMAL);
    dis->lock(0);
}

static void led7_show_call(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string((u8 *)" CAL");
    dis->lock(0);
}



static void led7_fm_show_freq(void *hd, void *private, u32 arg)
{
    u8 bcd_number[5] = {0};
    LCD_API *dis = (LCD_API *)hd;
    u16 freq = 0;
    freq = arg;

    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    itoa4(freq, (u8 *)bcd_number);
    if (freq > 999 && freq <= 1999) {
        bcd_number[0] = '1';
    } else {
        bcd_number[0] = ' ';
    }
    dis->show_string(bcd_number);
    dis->show_icon(LED7_DOT);
    dis->lock(0);
}


static void led7_show_wait(void *hd)
{
    LCD_API *dis = (LCD_API *)hd;
    dis->lock(1);
    dis->clear();
    dis->setXY(0, 0);
    dis->show_string((u8 *)" Lod");
    dis->lock(0);
}

static void *ui_open_bt(void *hd)
{
    void *private = NULL;
    ui_set_auto_reflash(500);//设置主页500ms自动刷新
    show_bt_delay_cnt = 0;

    if (!__this) {
        __this =  zalloc(sizeof(struct rtc_ui_bt_opr));
    }
    __this->dev_handle = dev_open("rtc", NULL);

    if (!__this->dev_handle) {
        free(__this);
        __this = NULL;
        return NULL;
    }		
    return private;
}

static void ui_close_bt(void *hd, void *private)
{
    LCD_API *dis = (LCD_API *)hd;
    if (!dis) {
        return;
    }
    if (private) {
        free(private);
    }
    if (__this) {
        free(__this);
        __this = NULL;
    }		
}

static void ui_bt_main(void *hd, void *private) //主界面显示
{
    if (!hd) {
        return;
    }

#if TCFG_APP_FM_EMITTER_EN

    if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) {
        led7_show_call(hd);
    } else {
        u16 fre = fm_emitter_manage_get_fre();
        if (fre != 0) {
            led7_fm_show_freq(hd, private, fre);
        } else {
            led7_show_wait(hd);
        }
    }
#else
    if (BT_STATUS_TAKEING_PHONE == get_bt_connect_status()) {
        led7_show_call(hd);
    } else {
        led7_show_bt(hd);
    }
#endif
}


static int ui_bt_user(void *hd, void *private, u8 menu, u32 arg)//子界面显示 //返回true不继续传递 ，返回false由common统一处理
{
    int ret = true;
    LCD_API *dis = (LCD_API *)hd;
    if (!hd) {
        return false;
    }

    switch (menu) {
    case MENU_BT:
        led7_show_bt(hd);
        break;

    default:
        ret = false;
    }

    return ret;

}

const struct ui_dis_api bt_main = {
    .ui      = UI_BT_MENU_MAIN,
    .open    = ui_open_bt,
    .ui_main = ui_bt_main,
    .ui_user = ui_bt_user,
    .close   = ui_close_bt,
};



#endif
#endif
