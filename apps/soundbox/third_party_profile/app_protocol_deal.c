#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "app_protocol_api.h"
#include "tone_player.h"
#include "clock_cfg.h"
#include "key_event_deal.h"
#include "bt.h"
#include "app_config.h"
#include "app_main.h"

#if AI_APP_PROTOCOL

#if 1
#define APP_PROTOCOL_LOG    printf
#define APP_PROTOCOL_DUMP   put_buf
#else
#define APP_PROTOCOL_LOG(...)
#define APP_PROTOCOL_DUMP(...)
#endif


int get_bt_tws_connect_status();

void mic_rec_clock_set(void)
{
    clock_add_set(AI_SPEECH_CLK);
}

void mic_rec_clock_recover(void)
{
    clock_remove_set(AI_SPEECH_CLK);
}

static void tone_play_in_app_core(int index)
{
    tone_play_with_callback_by_name(app_protocol_get_tone(index), 1, app_speech_tone_play_end_callback, (void *)index);
}

void app_protocol_tone_play(int index, int tws_sync)
{
    if (app_var.goto_poweroff_flag) {
        APP_PROTOCOL_LOG("shutdown don't play tone:%d\n", index);
        return;
    }
#if TCFG_APP_BT_EN
    if (app_bt_hdl.ignore_discon_tone) {
        APP_PROTOCOL_LOG("ingore discon tone:%d\n", index);
        return;
    }
#endif
#if TCFG_USER_TWS_ENABLE
    if (tws_sync && get_bt_tws_connect_status()) {
        APP_PROTOCOL_LOG("tws_sync play index:%d\n", index);
        app_protocol_tws_sync_send(APP_PROTOCOL_SYNC_TONE, index);
        return;
    }
#endif
#if TCFG_APP_BT_EN
    bt_drop_a2dp_frame_start(); //蓝牙播提示音，丢A2DP数据防止a2dp buf阻塞
#endif
    if (strcmp(os_current_task(), "app_core")) {
        APP_PROTOCOL_LOG("tone play in app core index:%d\n", index);
        app_protocol_post_app_core_callback((int)tone_play_in_app_core, (void *)index); //提示音放到app_core播放
    } else {
        APP_PROTOCOL_LOG("tone play index:%d\n", index);
        tone_play_with_callback_by_name(app_protocol_get_tone(index), 1, app_speech_tone_play_end_callback, (void *)index);
    }
}

extern u8 get_call_status();
int app_protocol_key_event_handler(struct sys_event *event)
{
    int ret = false;
    struct key_event *key = &event->u.key;
    int key_event = event->u.key.event;
    switch (key_event) {
#if APP_PROTOCOL_SPEECH_EN
    case KEY_SEND_SPEECH_START:
        if (get_call_status() != 5) {
            printf("phone is using, no speech allow\n");
            return ret;
        }
        APP_PROTOCOL_LOG("KEY_SEND_SPEECH_START \n");
        app_protocol_start_speech_by_key(event);
        break;
    case KEY_SEND_SPEECH_STOP:
        APP_PROTOCOL_LOG("KEY_SEND_SPEECH_STOP \n");
        app_protocol_stop_speech_by_key();
        break;
#endif
    }
    return ret;
}

#endif

