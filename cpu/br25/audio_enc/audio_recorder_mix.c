#include "audio_recorder_mix.h"
#include "audio_enc.h"
#include "audio_dec.h"
#include "media/pcm_decoder.h"
#include "btstack/btstack_task.h"
#include "btstack/avctp_user.h"
#include "audio_splicing.h"
#include "app_main.h"
#include "stream_entry.h"
#include "stream_sync.h"
#include "stream_src.h"

#if (RECORDER_MIX_EN)

#define RECORDER_MIX_CODING_TYPE			 AUDIO_CODING_MP3
#define RECORDER_MIX_PHONE_CODING_TYPE		 AUDIO_CODING_WAV
#define	RECORDER_MIX_SAMPLERATE				 (32000)//(32000L)
#define RECORDER_MIX_PHONE_OUT_PCM_BUF_SIZE  (1024+512)

//填0数据流控制句柄
struct __zero_stream {
    struct audio_stream 			*audio_stream;
    struct audio_decoder 			decoder;
    struct audio_res_wait 			wait;
    struct pcm_decoder 				pcm_dec;
    struct audio_mixer_ch 			mix_ch;
};

struct __recorder {
    struct __zero_stream  *zero;
    cbuffer_t			  *phone_cbuf;
    u16					  timer;
    u8					  phone_active;
};

//总控制句柄
struct __recorder_mix {
    struct __recorder 		*recorder;
    struct __stream_src 	*src;
    /* struct audio_src_handle *src; */
    struct __stream_entry 	*stream_entry;
    u8					    phone_chl;
    u16					  	phone_sr;
};

static u8 phone_out_pcm_buf[RECORDER_MIX_PHONE_OUT_PCM_BUF_SIZE] sec(.enc_file_mem);
static u8 phone_out_pcm_temp_buf[128 * 2] sec(.enc_file_mem);
static cbuffer_t phone_out_pcm_cbuf sec(.enc_file_mem);

struct __recorder_mix g_recorder_mix = {0};
#define __this (&g_recorder_mix)

//*----------------------------------------------------------------------------*/
/**@brief    通话音频数据混合写接口
   @param
   		data:输入数据地址
		len:输入数据长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
u32 recorder_mix_sco_data_write(u8 *data, u16 len)
{
    //putchar('.');
    u32 wlen = 0;
    if (__this->recorder) {
        if (__this->recorder->phone_cbuf) {
            u32 wlen = cbuf_write(__this->recorder->phone_cbuf, data, len);
            if (!wlen) {
                putchar('s');
            }
        }
    }
    return len;
}

static __attribute__((always_inline)) void recorder_mix_sco(s16 *data, u16 len, u8 channel, u8 *temp_buf)
{
    u16 rlen = len;
    s32 temp;
    s16 *read_buf = (s16 *)temp_buf;
    if (channel == 1) {
        if (cbuf_read(__this->recorder->phone_cbuf, read_buf, rlen)) {
            for (int i = 0; i < rlen / 2; i++) {
                temp = 	data[i];
                temp += read_buf[i];
                data[i] = data_sat_s16(temp);
            }
        } else {
            putchar('U');
        }
    } else if (channel == 2) {
        rlen = rlen >> 1;
        if (cbuf_read(__this->recorder->phone_cbuf, read_buf + (rlen >> 1), rlen)) {
            pcm_single_to_dual(read_buf, read_buf + (rlen >> 1), rlen);
            for (int i = 0; i < len / 2; i++) {
                temp = 	data[i];
                temp += read_buf[i];
                data[i] = data_sat_s16(temp);
            }
        } else {
            putchar('u');
        }
    } else if (channel > 2) {
        printf("channel over limit, rec no support\n");
    }
}

static int recoder_mix_soc_data_mix_pro_handle(struct audio_stream_entry *entry,  struct audio_data_frame *in)
{
    if (__this->recorder && __this->recorder->phone_cbuf) {
        u16 once_len = sizeof(phone_out_pcm_temp_buf);
        u16 once_points = once_len >> 1;
        u16 cnt = in->data_len / once_len;
        u16 remain = in->data_len % once_len;
        int i;
        /* printf("in->data_len = %d %d ", in->data_len, cnt); */
        for (i = 0; i < cnt; i++) {
            recorder_mix_sco(in->data + once_points * i, once_len, in->channel, phone_out_pcm_temp_buf);
        }
        if (remain) {
            recorder_mix_sco(in->data + once_points * i, remain, in->channel, phone_out_pcm_temp_buf);
        }
    }
    return 0;
}

//*----------------------------------------------------------------------------*/
/**@brief    录音数据流录音数据截获回调
   @param
		priv:私有参数
		data:数据
		len:数据长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static int recorder_mix_data_callback(void *priv, struct audio_data_frame *in)
{
    if (recorder_is_encoding()) {
        //printf("%d ", in->data_len);
        int wlen = recorder_userdata_to_enc(in->data, in->data_len);
        if (wlen != in->data_len) {
            /* putchar('R'); */
        }
        return in->data_len;
    }
    return in->data_len;
}

//*----------------------------------------------------------------------------*/
/**@brief    录音数据流节点激活接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void recorder_mix_data_stream_resume(void)
{
    if (__this->stream_entry)	{
        /* putchar('A');		 */
        audio_stream_resume(&__this->stream_entry->entry);
    }
}
//*----------------------------------------------------------------------------*/
/**@brief    录音数据流串接处理
   @param
		start_entry:分支起始数据流节点
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void recorder_mix_audio_stream_entry_add(struct audio_stream_entry *start_entry)
{
    u8 entry_cnt = 0;
    struct audio_stream_entry *entries[8] = {NULL};

    ///变采样
#if 0
    __this->src = zalloc(sizeof(struct audio_src_handle));
    ASSERT(__this->src);
    u8 ch_num = audio_output_channel_num();//跟mix通道数一致
    audio_hw_src_open(__this->src, ch_num, SRC_TYPE_RESAMPLE);
    audio_hw_src_set_rate(__this->src, 0, RECORDER_MIX_SAMPLERATE);
#else
    __this->src = stream_src_open(RECORDER_MIX_SAMPLERATE, 1);
#endif

    if (__this->src) {
        __this->src->entry.prob_handler = recoder_mix_soc_data_mix_pro_handle;
    }

    //录音取数节点
    __this->stream_entry = stream_entry_open(NULL, recorder_mix_data_callback, 1);

    //数据流串接处理
    entries[entry_cnt++] = start_entry;//起始节点与mix输出最后前一个节点分流
    if (__this->src) {
        entries[entry_cnt++] = &__this->src->entry;//变采样节点
    }
    entries[entry_cnt++] = &__this->stream_entry->entry;//录音取数节点
    for (int i = 0; i < entry_cnt - 1; i++) {
        audio_stream_add_entry(entries[i], entries[i + 1]);
    }
    printf("recorder_mix_audio_stream_entry_add ok~~~~~~~\n");
}
//*----------------------------------------------------------------------------*/
/**@brief    混合录音数据流删除处理
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void recorder_mix_audio_stream_entry_del(void)
{
    if (__this->src)	{
        stream_src_close(&__this->src);
    }
    if (__this->stream_entry) {
        stream_entry_close(&__this->stream_entry);
    }
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码读接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static int zero_pcm_fread(void *hdl, void *buf, u32 len)
{
    len = len / 2;
    memset(buf, 0, len);
    /* putchar('A'); */
    return len;
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码数据流数据处理接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static int zero_pcm_dec_data_handler(struct audio_stream_entry *entry,
                                     struct audio_data_frame *in,
                                     struct audio_data_frame *out)
{
    struct audio_decoder *decoder = container_of(entry, struct audio_decoder, entry);
    struct pcm_decoder *pcm_dec = container_of(decoder, struct pcm_decoder, decoder);
    audio_stream_run(&decoder->entry, in);
    return decoder->process_len;
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码数据流事件回调
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void zero_pcm_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码数据流激活接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void zero_pcm_dec_out_stream_resume(void *p)
{
    struct __zero_stream *zero = (struct __zero_stream *)p;
    if (zero) {
        audio_decoder_resume(&zero->pcm_dec.decoder);
    }
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码开始接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static int zero_pcm_dec_start(struct __zero_stream *zero)
{
    int err = 0;
    if (zero == NULL) {
        return -EINVAL;
    }
    err = pcm_decoder_open(&zero->pcm_dec, &decode_task);
    if (err) {
        return err;
    }
    pcm_decoder_set_event_handler(&zero->pcm_dec, zero_pcm_dec_event_handler, 0);
    pcm_decoder_set_read_data(&zero->pcm_dec, (void *)zero_pcm_fread, zero);
    pcm_decoder_set_data_handler(&zero->pcm_dec, zero_pcm_dec_data_handler);

    audio_mixer_ch_open(&zero->mix_ch, &mixer);
    /* audio_mixer_ch_set_src(&zero->mix_ch, 1, 0); */
    //audio_mixer_ch_set_no_wait(&zero->mix_ch, 1, 10); // 超时自动丢数

// 数据流串联
    struct audio_stream_entry *entries[8] = {NULL};
    u8 entry_cnt = 0;
    entries[entry_cnt++] = &zero->pcm_dec.decoder.entry;
    entries[entry_cnt++] = &zero->mix_ch.entry;
    zero->audio_stream = audio_stream_open(zero, zero_pcm_dec_out_stream_resume);
    audio_stream_add_list(zero->audio_stream, entries, entry_cnt);

    err = audio_decoder_start(&zero->pcm_dec.decoder);
    if (err == 0) {
        printf("zero_pcm_dec_start ok\n");
    }
    return err;
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码停止接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void zero_pcm_dec_stop(struct __zero_stream *zero)
{
    printf("[%s, %d] \n\n", __FUNCTION__, __LINE__);
    if (zero) {
        /* audio_decoder_close(&zero->decoder); */
        /* audio_mixer_ch_close(&zero->mix_ch);	 */
        pcm_decoder_close(&zero->pcm_dec);
        audio_mixer_ch_close(&zero->mix_ch);
        if (zero->audio_stream) {
            audio_stream_close(zero->audio_stream);
            zero->audio_stream = NULL;
        }
        /* audio_decoder_task_del_wait(&decode_task, &zero->wait); */
    }
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码启动等待处理接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static int zero_pcm_dec_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;
    struct __zero_stream *zero = container_of(wait, struct __zero_stream, wait);
    if (event == AUDIO_RES_GET) {
        err = zero_pcm_dec_start(zero);
    } else if (event == AUDIO_RES_PUT) {
        /* zero_pcm_dec_stop(zero); */
    }
    return err;
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码关闭
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void zero_pcm_dec_close(struct __zero_stream **hdl)
{
    if (hdl && (*hdl)) {
        struct __zero_stream *zero = *hdl;
        zero_pcm_dec_stop(zero);
        audio_decoder_task_del_wait(&decode_task, &zero->wait);
        local_irq_disable();
        free(zero);
        *hdl = NULL;
        local_irq_enable();
    }
}
//*----------------------------------------------------------------------------*/
/**@brief    填0解码打开接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static struct __zero_stream *zero_pcm_dec_open(void)
{
    struct __zero_stream *zero = zalloc(sizeof(struct __zero_stream));
    if (zero == NULL) {
        return NULL;
    }
    zero->pcm_dec.ch_num = 2;
    zero->pcm_dec.output_ch_num = audio_output_channel_num();
    zero->pcm_dec.output_ch_type = audio_output_channel_type();
    zero->pcm_dec.sample_rate = RECORDER_MIX_SAMPLERATE;
    zero->wait.priority = 0;
    zero->wait.preemption = 0;
    zero->wait.protect = 1;
    zero->wait.handler = zero_pcm_dec_wait_res_handler;
    printf("[%s], zero->pcm_dec.sample_rate:%d\n", __FUNCTION__, zero->pcm_dec.sample_rate);
    int err = audio_decoder_task_add_wait(&decode_task, &zero->wait);
    if (err) {
        printf("[%s, %d] fail\n\n", __FUNCTION__, __LINE__);
        zero_pcm_dec_close(&zero);
    }
    return zero;
}

//*----------------------------------------------------------------------------*/
/**@brief    混合录音编码器错误处理
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void recorder_mix_encode_err_callback(void)
{
    recorder_mix_stop();
}
//*----------------------------------------------------------------------------*/
/**@brief    混合录音编码器停止
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void recorder_mix_encode_stop(struct __recorder *hdl)
{
    recorder_encode_stop();
}
//*----------------------------------------------------------------------------*/
/**@brief    混合录音编码器开始
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static int recorder_mix_encode_start(struct __recorder *hdl)
{
    if (hdl == NULL) {
        return -1;
    }
    struct record_file_fmt fmt = {0};
    char folder[] = {REC_FOLDER_NAME};         //录音文件夹名称
    char filename[] = {"AC69****"};     //录音文件名，不需要加后缀，录音接口会根据编码格式添加后缀

#if (TCFG_NOR_REC)
    char logo[] = {"rec_nor"};		//外挂flash录音
#elif (FLASH_INSIDE_REC_ENABLE)
    char logo[] = {"rec_sdfile"};		//内置flash录音
#else
    char *logo = dev_manager_get_phy_logo(dev_manager_find_active(0));//普通设备录音，获取最后活动设备
#endif

    u16 sr = RECORDER_MIX_SAMPLERATE;
    fmt.dev = logo;
    fmt.folder = folder;
    fmt.filename = filename;
    fmt.channel = audio_output_channel_num();//跟mix通道数一致
    if (hdl->phone_active) {
        fmt.coding_type = RECORDER_MIX_PHONE_CODING_TYPE;
        fmt.sample_rate = __this->phone_sr;;
    } else {
        fmt.coding_type = RECORDER_MIX_CODING_TYPE;
        fmt.sample_rate = sr;
    }
    fmt.cut_head_time = 0;//300;            //录音文件去头时间,单位ms
    fmt.cut_tail_time = 0;//300;            //录音文件去尾时间,单位ms
    fmt.limit_size = 3000;              //录音文件大小最小限制， 单位byte
    fmt.source = ENCODE_SOURCE_MIX;     //录音输入源
    fmt.err_callback = recorder_mix_encode_err_callback;

    printf("[%s], fmt.sample_rate = %d, channel = %d, coding_type = %d\n", __FUNCTION__, fmt.sample_rate, fmt.channel, fmt.coding_type);
    if (fmt.channel > 2) {
        return -1;
    }

    int ret = recorder_encode_start(&fmt);
    if (ret) {
        log_e("[%s] fail !!, dev = %s\n", __FUNCTION__, logo);
    } else {
        log_e("[%s] succ !!, dev = %s\n", __FUNCTION__, logo);
    }
    return ret;
}

void recorder_mix_timer_deal(void *priv)
{
    if (__this->recorder) {
        int sec = recorder_get_enc_time();
        //printf("%d\n", sec);
    }
}

//*----------------------------------------------------------------------------*/
/**@brief    混合录音销毁处理接口
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void recorder_mix_destroy(struct __recorder **hdl)
{
    if (hdl == NULL || *hdl == NULL) {
        return ;
    }

    struct __recorder *recorder = *hdl;
    if (recorder->timer) {
        sys_timer_del(recorder->timer);
        recorder->timer = 0;
    }
    recorder_mix_encode_stop(recorder);
    zero_pcm_dec_close(&recorder->zero);
    if (__this->src) {
        stream_src_set_target_rate(__this->src, RECORDER_MIX_SAMPLERATE);
    }
    local_irq_disable();
    free(*hdl);
    *hdl = NULL;
    local_irq_enable();
}
//*----------------------------------------------------------------------------*/
/**@brief    混合录音开始
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int recorder_mix_start(void)
{
    if (__this->recorder) {
        recorder_mix_stop();
    }
    printf("%s begin\n", __FUNCTION__);
    mem_stats();
    struct __recorder *hdl = (struct __recorder *)zalloc(sizeof(struct __recorder));
    if (!hdl) {
        return false;
    }

    u8 call_status = get_call_status();
    if ((call_status != BT_CALL_HANGUP) && (call_status == BT_CALL_INCOMING)) {
        printf("phone incomming, no active, no record!!!!\n");
        return false;
    }

    if (app_var.siri_stu) {
        printf("siri active, no record!!!\n");
        return false;
    }

    hdl->phone_active = 0;
    if (call_status != BT_CALL_HANGUP) {
#if (RECORDER_MIX_BT_PHONE_EN == 0)
        printf("RECORDER_MIX_BT_PHONE_EN = 0 !!\n");
        return false;
#endif
        hdl->phone_active = 1;
        hdl->phone_cbuf = &phone_out_pcm_cbuf;
        cbuf_init(hdl->phone_cbuf, phone_out_pcm_buf, RECORDER_MIX_PHONE_OUT_PCM_BUF_SIZE);
        if (__this->src) {
            stream_src_set_target_rate(__this->src, __this->phone_sr);
        }
    } else {
        hdl->phone_cbuf = NULL;
    }

    if (hdl->phone_active == 0) {
        hdl->zero = zero_pcm_dec_open();
        if (hdl->zero == NULL) {
            recorder_mix_destroy(&hdl);
            return false;
        }
    }

    int ret = recorder_mix_encode_start(hdl);
    if (ret) {
        recorder_mix_destroy(&hdl);
        return false;
    }

    hdl->timer = sys_timer_add(NULL, recorder_mix_timer_deal, 1000);

    local_irq_disable();
    __this->recorder = hdl;
    local_irq_enable();
    printf("%s ok\n", __FUNCTION__);
    mem_stats();
    return true;
}
//*----------------------------------------------------------------------------*/
/**@brief    混合录音停止
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void recorder_mix_stop(void)
{
    if (__this->recorder) {
        recorder_mix_destroy(&__this->recorder);
    }
}

int recorder_mix_get_status(void)
{
    return (__this->recorder ? 1 : 0);
}


u32 recorder_mix_get_coding_type(void)
{
    //696默认返回MP3, 因为资源各种限制， MIC录音及非通话混合录音使用MP3编码，通话使用WAV编码
    return AUDIO_CODING_MP3;
}


void recorder_mix_call_status_change(u8 active)
{
    printf("[%s] = %d\n", __FUNCTION__, active);
    if (active) {
        if (__this->recorder && (__this->recorder->phone_active == 0)) {
            //如果启动录音的时候不是通话状态， 先停止之前的录音, 通话录音需要手动重新开
            printf("phone rec unactive !!, stop cur rec first\n");
            recorder_mix_stop();
        }
    } else {
        //
        /* recorder_mix_stop(); */
    }
}


void recorder_mix_bt_status_event(struct bt_event *e)
{
    switch (e->event) {
    case BT_STATUS_PHONE_INCOME:
        break;
    case BT_STATUS_PHONE_OUT:
        break;
    case BT_STATUS_PHONE_ACTIVE:
        break;
    case BT_STATUS_PHONE_HANGUP:
        recorder_mix_stop();
        break;
    case BT_STATUS_PHONE_NUMBER:
        break;
    case BT_STATUS_SCO_STATUS_CHANGE:
        if (e->value != 0xff) {
            //电话激活， 先停止当前录音
            recorder_mix_stop();
        } else {

        }
        break;
    case BT_STATUS_VOICE_RECOGNITION:
        if (e->value) { //如果是siri语音状态，停止录音
            /* recorder_mix_stop(); */
        }
        break;
    default:
        break;
    }
}

void recorder_mix_pcm_set_info(u16 sample_rate, u8 channel)
{
    __this->phone_sr = sample_rate;
    __this->phone_chl = channel;
    printf("%s, %d, %d\n", __FUNCTION__, sample_rate, channel);
}


#endif//RECORDER_MIX_EN


