#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "asm/audio_src.h"
#include "audio_enc.h"
#include "audio_dec.h"
#include "audio_dongle_codec.h"
#include "app_main.h"
#include "clock_cfg.h"
#include "media/pcm_decoder.h"

#if (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DONGLE)

#define DONGLE_USE_PCM_DEC				1 //使用单独的pcm解码做节拍

#define DONGLE_FILL_ZERO_MS				800 //填0

struct dongle_codec_hdl {
    struct audio_encoder encoder;
    s16 output_frame[1152 / 2];             //align 4Bytes
    int pcm_frame[64];                 //align 4Bytes
    u8 pcm_buf[1 * 1024];
    /* u8 pcm_buf[8 * 1024]; */
    cbuffer_t pcm_cbuf;
    u32 start : 1;
    void *cb_priv;
    int (*out_cb)(void *priv, void *buf, int len);

#if DONGLE_FILL_ZERO_MS
    u32 zero_fill_total;
    u32 zero_fill_len;
#endif

#if DONGLE_USE_PCM_DEC
    struct audio_mixer_ch 		mix_ch;
    struct audio_res_wait 		wait;
    struct pcm_decoder 			pcm_dec;		// pcm解码句柄
    struct audio_stream 		*audio_stream;	// 音频流
#endif
};

static struct dongle_codec_hdl *p_dongle = NULL;
struct dongle_emitter_hdl dongle_emitter = {0};

#ifdef CONFIG_MIXER_CYCLIC
static s16 dongle_mix_buff[128 * 2 * 2];
#else
static s16 dongle_mix_buff[8];
#endif

extern struct audio_encoder_task *encode_task;


#if DONGLE_USE_PCM_DEC

static int pcm_fread(void *hdl, void *buf, int len)
{
    len = len / 2;
    memset(buf, 0, len);
    /* putchar('A'); */
    return len;
}
static int pcm_dec_data_handler(struct audio_stream_entry *entry,
                                struct audio_data_frame *in,
                                struct audio_data_frame *out)
{
    struct audio_decoder *decoder = container_of(entry, struct audio_decoder, entry);
    struct pcm_decoder *pcm_dec = container_of(decoder, struct pcm_decoder, decoder);
    struct dongle_codec_hdl *dec = container_of(pcm_dec, struct dongle_codec_hdl, pcm_dec);
    audio_stream_run(&decoder->entry, in);
    /* audio_mixer_ch_pause(&dec->mix_ch, 0); */
    return decoder->process_len;
}
static void pcm_dec_event_handler(struct audio_decoder *decoder, int argc, int *argv)
{
}
static void dec_out_stream_resume(void *p)
{
    struct dongle_codec_hdl *dec = (struct dongle_codec_hdl *)p;

    audio_decoder_resume(&dec->pcm_dec.decoder);
}
static int pcm_dec_start(struct dongle_codec_hdl *stream)
{
    int err = 0;
    if (stream == NULL) {
        return -EINVAL;
    }
    err = pcm_decoder_open(&stream->pcm_dec, &decode_task);
    if (err) {
        return err;
    }
    pcm_decoder_set_event_handler(&stream->pcm_dec, pcm_dec_event_handler, 0);
    pcm_decoder_set_read_data(&stream->pcm_dec, pcm_fread, stream);
    pcm_decoder_set_data_handler(&stream->pcm_dec, pcm_dec_data_handler);

    audio_mixer_ch_open(&stream->mix_ch, &mixer);
    /* audio_mixer_ch_open_head(&stream->mix_ch, &mixer); // 挂载到mixer最前面 */
    audio_mixer_ch_set_src(&stream->mix_ch, 0, 0);
    /* audio_mixer_ch_set_src(&stream->mix_ch, 1, 1); */
    /* audio_mixer_ch_set_no_wait(&stream->mix_ch, 1, 5); // 超时自动丢数 */
    /* audio_mixer_ch_pause(&stream->mix_ch, 1); */
// 数据流串联
    struct audio_stream_entry *entries[8] = {NULL};
    u8 entry_cnt = 0;
    entries[entry_cnt++] = &stream->pcm_dec.decoder.entry;
    entries[entry_cnt++] = &stream->mix_ch.entry;
    stream->audio_stream = audio_stream_open(stream, dec_out_stream_resume);
    audio_stream_add_list(stream->audio_stream, entries, entry_cnt);

    audio_output_set_start_volume(APP_AUDIO_STATE_MUSIC);
    err = audio_decoder_start(&stream->pcm_dec.decoder);
    if (err == 0) {
        printf("pcm_dec_start ok\n");
    }
    return err;
}
static void pcm_dec_stop(struct dongle_codec_hdl *stream)
{
    printf("mic stream dec stop \n\n");
    if (stream) {
        /* audio_decoder_close(&stream->decoder); */
        /* audio_mixer_ch_close(&stream->mix_ch);	 */
        pcm_decoder_close(&stream->pcm_dec);
        audio_mixer_ch_close(&stream->mix_ch);
        if (stream->audio_stream) {
            audio_stream_close(stream->audio_stream);
            stream->audio_stream = NULL;
        }
        /* audio_decoder_task_del_wait(&decode_task, &stream->wait); */
    }
}

static int pcmdec_wait_res_handler(struct audio_res_wait *wait, int event)
{
    int err = 0;
    struct dongle_codec_hdl *stream = container_of(wait, struct dongle_codec_hdl, wait);
    if (event == AUDIO_RES_GET) {
        err = pcm_dec_start(stream);
    } else if (event == AUDIO_RES_PUT) {
        /* pcm_dec_stop(stream); */
    }

    return err;
}
static int pcm_dec_open(struct dongle_codec_hdl *stream, struct audio_fmt *pfmt)
{
    stream->pcm_dec.output_ch_num = stream->pcm_dec.ch_num = pfmt->channel;
    stream->pcm_dec.sample_rate = pfmt->sample_rate;
    if (stream->pcm_dec.output_ch_num == 2) {
        stream->pcm_dec.output_ch_type = AUDIO_CH_LR;
    } else {
        stream->pcm_dec.output_ch_type = AUDIO_CH_DIFF;
    }

    stream->wait.priority = 0;
    stream->wait.preemption = 0;
    stream->wait.protect = 1;
    stream->wait.handler = pcmdec_wait_res_handler;
    return audio_decoder_task_add_wait(&decode_task, &stream->wait);
}

static void pcm_dec_close(struct dongle_codec_hdl *stream)
{
    pcm_dec_stop(stream);
    audio_decoder_task_del_wait(&decode_task, &stream->wait);
}

#endif /*DONGLE_USE_PCM_DEC*/

static int dongle_enc_pcm_get(struct audio_encoder *encoder, s16 **frame, u16 frame_len)
{
    int rlen = 0;
    int dlen = 0;
    struct dongle_codec_hdl *enc = container_of(encoder, struct dongle_codec_hdl, encoder);

    /* printf("l:%d", frame_len); */

    if (!enc->start) {
        return 0;
    }
    if (frame_len > sizeof(enc->pcm_frame)) {
        frame_len = sizeof(enc->pcm_frame);
    }

#if DONGLE_FILL_ZERO_MS
    if (enc->zero_fill_len < enc->zero_fill_total) {
        enc->zero_fill_len += frame_len;
        memset(enc->pcm_frame, 0, frame_len);
        *frame = (s16 *)enc->pcm_frame;
        return frame_len;
    }
#endif

    dlen = cbuf_get_data_len(&enc->pcm_cbuf);
    if (dlen < frame_len) {
        /* putchar('T');*/
        return 0;
    }

    rlen = cbuf_read(&enc->pcm_cbuf, enc->pcm_frame, frame_len);
    audio_stream_resume(&dongle_emitter.entry);

    *frame = (s16 *)enc->pcm_frame;

    return rlen;
}

static void dongle_enc_pcm_put(struct audio_encoder *encoder, s16 *frame)
{
}

static const struct audio_enc_input dongle_enc_input = {
    .fget = dongle_enc_pcm_get,
    .fput = dongle_enc_pcm_put,
};

static int dongle_enc_probe_handler(struct audio_encoder *encoder)
{
    return 0;
}

static int dongle_enc_output_handler(struct audio_encoder *encoder, u8 *frame, int len)
{
    struct dongle_codec_hdl *enc = container_of(encoder, struct dongle_codec_hdl, encoder);
    /* printf("output frame:%d \n", len); */
    /* put_buf(frame, len); */
    if (enc->out_cb) {
        len = enc->out_cb(enc->cb_priv, frame, len);
    }
    return len;
}

const static struct audio_enc_handler dongle_enc_handler = {
    .enc_probe = dongle_enc_probe_handler,
    .enc_output = dongle_enc_output_handler,
};

static void dongle_enc_event_handler(struct audio_encoder *encoder, int argc, int *argv)
{
    printf("dongle_enc_event_handler:0x%x,%d\n", argv[0], argv[0]);
    switch (argv[0]) {
    case AUDIO_ENC_EVENT_END:
        puts("AUDIO_ENC_EVENT_END\n");
        break;
    }
}

void audio_dongle_enc_close(void)
{
    if (!p_dongle) {
        return ;
    }
    p_dongle->start = 0;

    audio_encoder_close(&p_dongle->encoder);

#if DONGLE_USE_PCM_DEC
    pcm_dec_close(p_dongle);
#endif

    local_irq_disable();
    free(p_dongle);
    p_dongle = NULL;
    local_irq_enable();

    audio_encoder_task_close();

    clock_remove_set(DONGLE_ENC_CLK);
}

int audio_dongle_enc_open(struct audio_fmt *pfmt,
                          int (*out_cb)(void *priv, void *buf, int len),
                          void *cb_priv
                         )
{
    struct audio_fmt fmt = {0};

    audio_dongle_enc_close();

    if (pfmt) {
        memcpy(&fmt, pfmt, sizeof(struct audio_fmt));
    } else {
        fmt.coding_type = AUDIO_CODING_MP3;
        /* fmt.coding_type = AUDIO_CODING_WAV; */
        fmt.bit_rate = 128;
        fmt.channel = audio_output_channel_num();
        fmt.sample_rate = DONGLE_OUTPUT_SAMPLE_RATE;//audio_mixer_get_sample_rate(&mixer);
    }
    if (fmt.coding_type == AUDIO_CODING_MP3) {
        if ((fmt.sample_rate < 16000) && (fmt.bit_rate > 64)) {
            fmt.bit_rate = 64;
        }
    }

    audio_encoder_task_open();

    struct dongle_codec_hdl *p_enc = zalloc(sizeof(struct dongle_codec_hdl));
    ASSERT(p_enc);

    clock_add_set(DONGLE_ENC_CLK);

    cbuf_init(&p_enc->pcm_cbuf, p_enc->pcm_buf, sizeof(p_enc->pcm_buf));

#if DONGLE_FILL_ZERO_MS
    p_enc->zero_fill_total = fmt.channel * fmt.sample_rate * 2 * DONGLE_FILL_ZERO_MS / 1000;
    p_enc->zero_fill_len = 0;
#endif

    audio_encoder_open(&p_enc->encoder, &dongle_enc_input, encode_task);
    audio_encoder_set_handler(&p_enc->encoder, &dongle_enc_handler);
    audio_encoder_set_fmt(&p_enc->encoder, &fmt);
    audio_encoder_set_event_handler(&p_enc->encoder, dongle_enc_event_handler, 0);
    audio_encoder_set_output_buffs(&p_enc->encoder, p_enc->output_frame,
                                   sizeof(p_enc->output_frame), 1);
    p_enc->cb_priv = cb_priv;
    p_enc->out_cb = out_cb;
    p_enc->start = 1;
    p_dongle = p_enc;
    audio_encoder_start(&p_enc->encoder);

#if DONGLE_USE_PCM_DEC
    pcm_dec_open(p_dongle, &fmt);
#endif

    return 0;
}

static int audio_stream_dongle_emitter_data_handler(struct audio_stream_entry *entry,
        struct audio_data_frame *in,
        struct audio_data_frame *out)
{
    if (in->data_len == 0) {
        return 0;
    }
    int len = in->data_len;

    local_irq_disable();
    if (p_dongle && p_dongle->start) {
        u32 wlen = cbuf_write(&p_dongle->pcm_cbuf, in->data, len);
        if (wlen != len) {
            log_w("wlen = %d, len = %d", wlen, len);
            /* y_printf(">>>[test]:len = %d, wlen = %d\n", len, wlen); */
        }
        audio_encoder_resume(&p_dongle->encoder);
    }
    local_irq_enable();

    return len;
}

void audio_dongle_emitter_init(void)
{
    audio_mixer_open(&dongle_emitter.mixer);
    /* audio_mixer_set_event_handler(&dongle_emitter.mixer, mixer_event_handler); */
    /* audio_mixer_set_check_sr_handler(&dongle_emitter.mixer, audio_mixer_check_sr); */
    /*初始化mix_buf的长度*/
    audio_mixer_set_output_buf(&dongle_emitter.mixer, dongle_mix_buff, sizeof(dongle_mix_buff));
    u8 ch_num = audio_output_channel_num();
    audio_mixer_set_channel_num(&dongle_emitter.mixer, ch_num);
    audio_mixer_set_sample_rate(&dongle_emitter.mixer, MIXER_SR_SPEC, DONGLE_OUTPUT_SAMPLE_RATE);

    dongle_emitter.entry.data_handler = audio_stream_dongle_emitter_data_handler;

    struct audio_stream_entry *entries[4] = {NULL};
    u8 entry_cnt = 0;
    entries[entry_cnt++] = &dongle_emitter.mixer.entry;
    entries[entry_cnt++] = &dongle_emitter.entry;

    dongle_emitter.mixer.stream = audio_stream_open(&dongle_emitter.mixer, audio_mixer_stream_resume);
    audio_stream_add_list(dongle_emitter.mixer.stream, entries, entry_cnt);

    audio_mixer_ch_open(&dongle_emitter.mix_ch, &dongle_emitter.mixer);
    audio_mixer_ch_set_src(&dongle_emitter.mix_ch, 1, 0);
}


#if TCFG_VIR_UDISK_ENABLE

#include "virtual_mp3_file.h"

static void *dg_file = NULL;
static int dg_file_len = 0;
static int dg_out_cb(void *priv, void *buf, int len)
{
    putchar('@');

    virtual_mp3_file_write(buf, len);
    /* if (dg_file) { */
    /*     fwrite(dg_file, buf, len); */
    /* } */
    dg_file_len += len;
    return len;
}

void dongle_clear_enc_cbuf()
{
    if (p_dongle) {
        cbuf_clear(&p_dongle->pcm_cbuf);
        /* memset(&p_dongle->pcm_buf, 0, 3 * 1024); */
        /* memset(&p_dongle->output_frame, 0, 1152/2 ); */
        /* memset(&p_dongle->pcm_frame, 0, 64); */
    }

}

void dongle_enc_start_ex()
{
    if (p_dongle) {
        p_dongle->start = 1;
    }
}

void dongle_enc_stop_ex()
{
    if (p_dongle) {
        p_dongle->start = 0;
    }
}

void audio_dongle_reset()
{
    audio_dongle_enc_open(NULL, dg_out_cb, NULL);
}

void dongle_enc_close()
{
    if (p_dongle)  {
        y_printf("dongle close \n");
        audio_dongle_enc_close();
        printf("dg_file_len :%d ", dg_file_len);
    }
}

void audio_usbdongle_ctrl(void)
{

    if (p_dongle)  {
        y_printf("dongle close \n");
        audio_dongle_enc_close();
        printf("dg_file_len :%d ", dg_file_len);
    } else {
        y_printf("dongle open \n");
        dg_file_len = 0;
        audio_dongle_enc_open(NULL, dg_out_cb, NULL);
    }

}


static int dg_out_cb_test(void *priv, void *buf, int len)
{
    putchar('@');

    /* virtual_mp3_file_write(buf, len); */
    if (dg_file) {
        fwrite(dg_file, buf, len);
    }
    dg_file_len += len;
    return len;
}

void audio_dongle_test(void)
{
#if 0
    printf("%s,%d \n", __func__, __LINE__);
    dg_file = fopen("storage/sd0/C/DONGLE/AC690000.MP3", "w+");
    printf("%s,%d \n", __func__, __LINE__);
    if (dg_file) {
        u8 buf[128];
        for (int i = 0; i < sizeof(buf); i++) {
            buf[i] = i;
        }
        printf("%s,%d \n", __func__, __LINE__);
        fwrite(dg_file, buf, sizeof(buf));
        printf("%s,%d \n", __func__, __LINE__);
        fclose(dg_file);
        dg_file = NULL;
        printf("%s,%d \n", __func__, __LINE__);
    }
    printf("%s,%d \n", __func__, __LINE__);
#endif

    if (p_dongle)  {
        y_printf("dongle close \n");
        audio_dongle_enc_close();
        if (dg_file) {
            fclose(dg_file);
            dg_file = NULL;
        }
        printf("dg_file_len :%d ", dg_file_len);
    } else {
        y_printf("dongle open \n");
        if (dg_file) {
            fclose(dg_file);
            dg_file = NULL;
        }
        dg_file_len = 0;
#if 0
        dg_file = fopen("storage/sd0/C/DONGLE/AC690000.MP3", "w+");
#else
        u8 name[128] = "storage/sd0/C/DONGLE/AC69****.MP3";
        dg_file = fopen(name, "w+");
#endif
        printf("fopen:0x%x \n", dg_file);
        audio_dongle_enc_open(NULL, dg_out_cb_test, NULL);
    }
}
#endif

#endif /* (AUDIO_OUTPUT_WAY == AUDIO_OUTPUT_WAY_DONGLE) */

