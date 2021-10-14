/*
 ****************************************************************
 *File : audio_dc_linein.c
 *Note :
 *
 ****************************************************************
 */


#include "asm/includes.h"
#include "media/includes.h"
#include "media/pcm_decoder.h"
#include "system/includes.h"
#include "effectrs_sync.h"
#include "application/audio_eq_drc_apply.h"
#include "app_config.h"
#include "audio_config.h"
#include "audio_dec.h"
#include "app_config.h"
#include "app_main.h"
#include "audio_enc.h"
#include "clock_cfg.h"
#include "dev_manager.h"
#if TCFG_PCM_ENC2TWS_ENABLE
#include "bt_tws.h"
#endif

#if (TCFG_LINEIN_ENABLE || TCFG_FM_ENABLE)//外部收音走linein

//////////////////////////////////////////////////////////////////////////////

struct linein_dec_hdl {
    struct audio_stream *stream;	// 音频流
    struct pcm_decoder pcm_dec;		// pcm解码句柄
    struct audio_res_wait wait;		// 资源等待句柄
    struct audio_mixer_ch mix_ch;	// 叠加句柄
    struct audio_eq_drc *eq_drc;//eq drc句柄
    u32 id;				// 唯一标识符，随机值
    u32 start : 1;		// 正在解码
    u32 source : 8;		// linein音频源
    void *linein;		// 底层驱动句柄
};


//////////////////////////////////////////////////////////////////////////////

struct linein_dec_hdl *linein_dec = NULL;	// linein解码句柄

//////////////////////////////////////////////////////////////////////////////
void *linein_eq_drc_open(u16 sample_rate, u8 ch_num);
void linein_eq_drc_close(struct audio_eq_drc *eq_drc);

int linein_sample_size(void *hdl);
int linein_sample_total(void *hdl);


//////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------*/
/**@brief    linein解码释放
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void linein_dec_relaese()
{
    if (linein_dec) {
        audio_decoder_task_del_wait(&decode_task, &linein_dec->wait);
        clock_remove(AUDIO_CODING_PCM);
        local_irq_disable();
        free(linein_dec);
        linein_dec = NULL;
        local_irq_enable();
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    linein解码事件处理
   @param    *decoder: 解码器句柄
   @param    argc: 参数个数
   @param    *argv: 参数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void linein_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
    switch (argv[0]) {
    case AUDIO_DEC_EVENT_END:
        if (!linein_dec) {
            log_i("linein_dec handle err ");
            break;
        }

        if (linein_dec->id != argv[1]) {
            log_w("linein_dec id err : 0x%x, 0x%x \n", linein_dec->id, argv[1]);
            break;
        }

        linein_dec_close();
        //audio_decoder_resume_all(&decode_task);
        break;
    }
}


/*----------------------------------------------------------------------------*/
/**@brief    linein解码数据输出
   @param    *entry: 音频流句柄
   @param    *in: 输入信息
   @param    *out: 输出信息
   @return   输出长度
   @note     *out未使用
*/
/*----------------------------------------------------------------------------*/
static int linein_dec_data_handler(struct audio_stream_entry *entry,
                                   struct audio_data_frame *in,
                                   struct audio_data_frame *out)
{
    struct audio_decoder *decoder = container_of(entry, struct audio_decoder, entry);
    struct pcm_decoder *pcm_dec = container_of(decoder, struct pcm_decoder, decoder);
    struct linein_dec_hdl *dec = container_of(pcm_dec, struct linein_dec_hdl, pcm_dec);
    if (!dec->start) {
        return 0;
    }
    audio_stream_run(&decoder->entry, in);
    return decoder->process_len;
}

/*----------------------------------------------------------------------------*/
/**@brief    linein解码数据流激活
   @param    *p: 私有句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void linein_dec_out_stream_resume(void *p)
{
    struct linein_dec_hdl *dec = p;
    audio_decoder_resume(&dec->pcm_dec.decoder);
}

/*----------------------------------------------------------------------------*/
/**@brief    linein解码激活
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void linein_dec_resume(void)
{
    if (linein_dec) {
        audio_decoder_resume(&linein_dec->pcm_dec.decoder);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    计算linein输入采样率
   @param
   @return   采样率
   @note
*/
/*----------------------------------------------------------------------------*/
static int audio_linein_input_sample_rate(void *priv)
{
    struct linein_dec_hdl *dec = (struct linein_dec_hdl *)priv;
    int sample_rate = linein_stream_sample_rate(dec->linein);
    int buf_size = linein_sample_size(dec->linein);
#if TCFG_PCM_ENC2TWS_ENABLE
    if (dec->pcm_dec.dec_no_out_sound) {
        /*TWS的上限在linein输入buffer，下限在tws push buffer*/
        if (buf_size >= linein_sample_total(dec->linein) / 2) {
            sample_rate += (sample_rate * 1 / 10000);
        } else if (tws_api_local_media_trans_check_ready_total() < 1024) {
            sample_rate -= (sample_rate * 1 / 10000);
        }
        return sample_rate;
    }
#endif
    /* if (dec->pcm_dec.sample_rate == 44100 && audio_mixer_get_sample_rate(&mixer) == 44100) { */
    if ((dec->pcm_dec.sample_rate == 44100) && (audio_mixer_get_original_sample_rate_by_type(&mixer, MIXER_SR_SPEC) == 44100)) {
        sample_rate = 44100 + (sample_rate - 44118);
    }
    if (buf_size >= (linein_sample_total(dec->linein) * 3 / 4)) {
        sample_rate += (sample_rate * 1 / 10000);
    }
    if (buf_size <= (linein_sample_total(dec->linein) / 2)) {
        sample_rate -= (sample_rate * 1 / 10000);
    }
    return sample_rate;
}

/*----------------------------------------------------------------------------*/
/**@brief    linein解码开始
   @param
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int linein_dec_start()
{
    int err;
    struct linein_dec_hdl *dec = linein_dec;
    struct audio_mixer *p_mixer = &mixer;

    if (!linein_dec) {
        return -EINVAL;
    }

    err = pcm_decoder_open(&dec->pcm_dec, &decode_task);
    if (err) {
        goto __err1;
    }

    // 打开linein驱动
    dec->linein = linein_sample_open(dec->source, dec->pcm_dec.sample_rate);
    linein_sample_set_resume_handler(dec->linein, linein_dec_resume);

    pcm_decoder_set_event_handler(&dec->pcm_dec, linein_dec_event_handler, dec->id);
    pcm_decoder_set_read_data(&dec->pcm_dec, linein_sample_read, dec->linein);
    pcm_decoder_set_data_handler(&dec->pcm_dec, linein_dec_data_handler);

#if TCFG_PCM_ENC2TWS_ENABLE
    {
        // localtws使用sbc等编码转发
        struct audio_fmt enc_f;
        memcpy(&enc_f, &dec->pcm_dec.decoder.fmt, sizeof(struct audio_fmt));
        enc_f.coding_type = AUDIO_CODING_SBC;
        if (dec->pcm_dec.ch_num == 2) { // 如果是双声道数据，localtws在解码时才变成对应声道
            enc_f.channel = 2;
        }
        int ret = localtws_enc_api_open(&enc_f, LOCALTWS_ENC_FLAG_STREAM);
        if (ret == true) {
            dec->pcm_dec.dec_no_out_sound = 1;
            // 重定向mixer
            p_mixer = &g_localtws.mixer;
            // 关闭资源等待。最终会在localtws解码处等待
            audio_decoder_task_del_wait(&decode_task, &dec->wait);
            if (dec->pcm_dec.output_ch_num != enc_f.channel) {
                dec->pcm_dec.output_ch_num = dec->pcm_dec.decoder.fmt.channel = enc_f.channel;
                if (enc_f.channel == 2) {
                    dec->pcm_dec.output_ch_type = AUDIO_CH_LR;
                } else {
                    dec->pcm_dec.output_ch_type = AUDIO_CH_DIFF;
                }
            }
        }
    }
#endif

    if (!dec->pcm_dec.dec_no_out_sound) {
        audio_mode_main_dec_open(AUDIO_MODE_MAIN_STATE_DEC_LINEIN);
    }

    // 设置叠加功能
    audio_mixer_ch_open_head(&dec->mix_ch, p_mixer);
    audio_mixer_ch_set_no_wait(&dec->mix_ch, 1, 10); // 超时自动丢数
    audio_mixer_ch_follow_resample_enable(&dec->mix_ch, dec, audio_linein_input_sample_rate);
    dec->eq_drc = linein_eq_drc_open(dec->pcm_dec.sample_rate, dec->pcm_dec.output_ch_num);
    // 数据流串联
    struct audio_stream_entry *entries[8] = {NULL};
    u8 entry_cnt = 0;
    entries[entry_cnt++] = &dec->pcm_dec.decoder.entry;
#if TCFG_EQ_ENABLE && TCFG_LINEIN_MODE_EQ_ENABLE
    if (dec->eq_drc) {
        entries[entry_cnt++] = &dec->eq_drc->entry;
    }
#endif


#if SYS_DIGVOL_GROUP_EN
    void *dvol_entry = sys_digvol_group_ch_open("music_linein", -1, NULL);
    entries[entry_cnt++] = dvol_entry;
#endif // SYS_DIGVOL_GROUP_EN


    entries[entry_cnt++] = &dec->mix_ch.entry;
    // 创建数据流，把所有节点连接起来
    dec->stream = audio_stream_open(dec, linein_dec_out_stream_resume);
    audio_stream_add_list(dec->stream, entries, entry_cnt);

    // 设置音频输出音量
    audio_output_set_start_volume(APP_AUDIO_STATE_MUSIC);

    // 开始解码
    dec->start = 1;
    err = audio_decoder_start(&dec->pcm_dec.decoder);
    if (err) {
        goto __err3;
    }
    clock_set_cur();
    return 0;
__err3:
    dec->start = 0;
    linein_eq_drc_close(dec->eq_drc);
    if (dec->linein) {
        linein_sample_close(dec->linein);
        dec->linein = NULL;
    }
    audio_mixer_ch_close(&dec->mix_ch);
#if TCFG_PCM_ENC2TWS_ENABLE
    if (linein_dec->pcm_dec.dec_no_out_sound) {
        linein_dec->pcm_dec.dec_no_out_sound = 0;
        localtws_enc_api_close();
    }
#endif
#if SYS_DIGVOL_GROUP_EN
    sys_digvol_group_ch_close("music_linein");
#endif // SYS_DIGVOL_GROUP_EN


    if (dec->stream) {
        audio_stream_close(dec->stream);
        dec->stream = NULL;
    }


    pcm_decoder_close(&dec->pcm_dec);
__err1:
    linein_dec_relaese();
    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    fm解码关闭
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void __linein_dec_close(void)
{
    if (linein_dec && linein_dec->start) {
        linein_dec->start = 0;

        pcm_decoder_close(&linein_dec->pcm_dec);
        linein_sample_close(linein_dec->linein);
        linein_dec->linein = NULL;

        linein_eq_drc_close(linein_dec->eq_drc);
        audio_mixer_ch_close(&linein_dec->mix_ch);
#if TCFG_PCM_ENC2TWS_ENABLE
        if (linein_dec->pcm_dec.dec_no_out_sound) {
            linein_dec->pcm_dec.dec_no_out_sound = 0;
            localtws_enc_api_close();
        }
#endif
#if SYS_DIGVOL_GROUP_EN
        sys_digvol_group_ch_close("music_linein");
#endif // SYS_DIGVOL_GROUP_EN


        // 先关闭各个节点，最后才close数据流
        if (linein_dec->stream) {
            audio_stream_close(linein_dec->stream);
            linein_dec->stream = NULL;
        }



    }
}

/*----------------------------------------------------------------------------*/
/**@brief    linein解码资源等待
   @param    *wait: 句柄
   @param    event: 事件
   @return   0：成功
   @note     用于多解码打断处理
*/
/*----------------------------------------------------------------------------*/
static int linein_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;
    log_i("linein_wait_res_handler, event:%d\n", event);
    if (event == AUDIO_RES_GET) {
        // 启动解码
        err = linein_dec_start();
    } else if (event == AUDIO_RES_PUT) {
        // 被打断
        __linein_dec_close();
    }

    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    打开linein解码
   @param    source: 音频源
   @param    sample_rate: 采样率
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int linein_dec_open(u8 source, u32 sample_rate)
{
    int err;
    struct linein_dec_hdl *dec;
    dec = zalloc(sizeof(*dec));
    if (!dec) {
        return -ENOMEM;
    }
    linein_dec = dec;
    dec->id = rand32();

    switch (source) {
    case BIT(0):
    case BIT(1):
        dec->pcm_dec.ch_num = 1;
        break;
    case BIT(2):
    case BIT(3):
        dec->pcm_dec.ch_num = 1;
        break;
    default:
        if (audio_adc_support_linein_combined()) {
            /*F版支持一路adc采样linein左右声道混合输入*/
            dec->pcm_dec.ch_num = 1;
        } else {
            log_e("Linein can only select a single channel\n");
            ASSERT(0, "err\n");
        }
        break;
    }
    dec->source = source;

    dec->pcm_dec.output_ch_num = audio_output_channel_num();
    dec->pcm_dec.output_ch_type = audio_output_channel_type();
    dec->pcm_dec.sample_rate = sample_rate;

    dec->wait.priority = 2;
    dec->wait.preemption = 0;
    dec->wait.snatch_same_prio = 1;
    dec->wait.handler = linein_wait_res_handler;
    clock_add(AUDIO_CODING_PCM);


#if TCFG_DEC2TWS_ENABLE
    // 设置localtws重播接口
    localtws_globle_set_dec_restart(linein_dec_push_restart);
#endif

    err = audio_decoder_task_add_wait(&decode_task, &dec->wait);
    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    关闭linein解码
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void linein_dec_close(void)
{
    if (!linein_dec) {
        return;
    }

    __linein_dec_close();
    linein_dec_relaese();
    clock_set_cur();
    log_i("linein dec close \n\n ");
}

/*----------------------------------------------------------------------------*/
/**@brief    linein解码重新开始
   @param    id: 文件解码id
   @return   0：成功
   @return   非0：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int linein_dec_restart(int id)
{
    if ((!linein_dec) || (id != linein_dec->id)) {
        return -1;
    }
    u8 source = linein_dec->source;
    u32 sample_rate = linein_dec->pcm_dec.sample_rate;
    linein_dec_close();
    int err = linein_dec_open(source, sample_rate);
    return err;
}

/*----------------------------------------------------------------------------*/
/**@brief    推送linein解码重新开始命令
   @param
   @return   true：成功
   @return   false：失败
   @note
*/
/*----------------------------------------------------------------------------*/
int linein_dec_push_restart(void)
{
    if (!linein_dec) {
        return false;
    }
    int argv[3];
    argv[0] = (int)linein_dec_restart;
    argv[1] = 1;
    argv[2] = (int)linein_dec->id;
    os_taskq_post_type(os_current_task(), Q_CALLBACK, ARRAY_SIZE(argv), argv);
    return true;
}

/*----------------------------------------------------------------------------*/
/**@brief    linein模式 eq drc 打开
   @param    sample_rate:采样率
   @param    ch_num:通道个数
   @return   句柄
   @note
*/
/*----------------------------------------------------------------------------*/
void *linein_eq_drc_open(u16 sample_rate, u8 ch_num)
{
#if TCFG_EQ_ENABLE

    struct audio_eq_drc *eq_drc = NULL;
    struct audio_eq_drc_parm effect_parm = {0};
#if TCFG_LINEIN_MODE_EQ_ENABLE
    effect_parm.eq_en = 1;

#if TCFG_DRC_ENABLE
#if TCFG_LINEIN_MODE_DRC_ENABLE
    effect_parm.drc_en = 1;
    effect_parm.drc_cb = drc_get_filter_info;
#endif
#endif


    if (effect_parm.eq_en) {
        effect_parm.async_en = 1;
        effect_parm.out_32bit = 1;
        effect_parm.online_en = 1;
        effect_parm.mode_en = 1;
    }

    effect_parm.eq_name = song_eq_mode;
#if TCFG_EQ_DIVIDE_ENABLE
    effect_parm.divide_en = 1;
#endif


    effect_parm.ch_num = ch_num;
    effect_parm.sr = sample_rate;
    effect_parm.eq_cb = eq_get_filter_info;
    eq_drc = audio_eq_drc_open(&effect_parm);

    clock_add(EQ_CLK);
    if (effect_parm.drc_en) {
        clock_add(EQ_DRC_CLK);
    }
#endif
    return eq_drc;
#endif
    return NULL;
}
/*----------------------------------------------------------------------------*/
/**@brief    linein模式 eq drc 关闭
   @param    句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void linein_eq_drc_close(struct audio_eq_drc *eq_drc)
{
#if TCFG_EQ_ENABLE
#if TCFG_LINEIN_MODE_EQ_ENABLE
    if (eq_drc) {
        audio_eq_drc_close(eq_drc);
        eq_drc = NULL;
        clock_remove(EQ_CLK);
#if TCFG_DRC_ENABLE
#if TCFG_LINEIN_MODE_DRC_ENABLE
        clock_remove(EQ_DRC_CLK);
#endif
#endif
    }
#endif
#endif
    return;
}

#endif
