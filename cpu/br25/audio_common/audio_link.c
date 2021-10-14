/*****************************************************************
>file name : lib/media/cpu/br22/audio_link.c
>author :
>create time : Fri 7 Dec 2018 14:59:12 PM CST
*****************************************************************/
#include "includes.h"
#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "app_config.h"
#include "asm/clock.h"
#include "asm/iis.h"
#include "audio_link.h"

#if TCFG_IIS_ENABLE

#define ALINK_TEST_ENABLE

#define ALINK_DEBUG_INFO
#ifdef ALINK_DEBUG_INFO
#define alink_printf  printf
#else
#define alink_printf(...)
#endif

static u32 *ALNK_BUF_ADR[] = {
    (u32 *)(&(JL_ALNK0->ADR0)),
    (u32 *)(&(JL_ALNK0->ADR1)),
    (u32 *)(&(JL_ALNK0->ADR2)),
    (u32 *)(&(JL_ALNK0->ADR3)),
};

enum {
    IIS_IO_MCLK = 0u,
    IIS_IO_SCLK 	,
    IIS_IO_LRCK 	,
    IIS_IO_CH0 		,
    IIS_IO_CH1 		,
    IIS_IO_CH2 		,
    IIS_IO_CH3 		,
};

enum {
    MCLK_11M2896K = 0,
    MCLK_12M288K
};

enum {
    MCLK_EXTERNAL	= 0u,
    MCLK_SYS_CLK		,
    MCLK_OSC_CLK 		,
    MCLK_PLL_CLK		,
};

enum {
    MCLK_DIV_1		= 0u,
    MCLK_DIV_2			,
    MCLK_DIV_4			,
    MCLK_DIV_8			,
    MCLK_DIV_16			,
};

enum {
    MCLK_LRDIV_EX	= 0u,
    MCLK_LRDIV_64FS		,
    MCLK_LRDIV_128FS	,
    MCLK_LRDIV_192FS	,
    MCLK_LRDIV_256FS	,
    MCLK_LRDIV_384FS	,
    MCLK_LRDIV_512FS	,
    MCLK_LRDIV_768FS	,
};

static const u8 alink0_IOS_portA[] = {
    IO_PORTA_02, IO_PORTA_03, IO_PORTA_04, IO_PORTA_00, IO_PORTA_01, IO_PORTA_05, IO_PORTA_06,
};
static const u8 alink0_IOS_portB[] = {
    IO_PORTC_02, IO_PORTA_05, IO_PORTA_06, IO_PORTA_00, IO_PORTA_02, IO_PORTA_03, IO_PORTA_04,
};
static u8 alink0_IOS_port[7] = {0};

ALINK_PARM *p_alink_parm;

//==================================================
enum alnk_clock {
    ALINK_CLOCK_12M288K,
    ALINK_CLOCK_11M2896K,
};
static void audio_link_clock_sel(enum alnk_clock clk)
{
    switch (clk) {
    case ALINK_CLOCK_12M288K:
        SFR(JL_CLOCK->CLK_CON2, 6, 4, 5); //160/13
        break;
    case ALINK_CLOCK_11M2896K:
        SFR(JL_CLOCK->CLK_CON2, 6, 4, 0); //192 / 17
        break;
    default:
        break;
    }
}

//==================================================
static void alink_clk_io_in_init(u8 gpio)
{
    gpio_set_direction(gpio, 1);
    gpio_set_pull_down(gpio, 0);
    gpio_set_pull_up(gpio, 1);
    gpio_set_die(gpio, 1);
}


static void *alink_addr(u8 ch)
{
    u8 *buf_addr = NULL; //can be used
    u32 buf_index = 0;
    buf_addr = (u8 *)(p_alink_parm->ch_cfg[ch].buf);

    u8 index_table[4] = {12, 13, 14, 15};
    u8 bit_index = index_table[ch];
    buf_index = (JL_ALNK0->CON0 & BIT(bit_index)) ? 0 : 1;
    buf_addr = buf_addr + ((p_alink_parm->dma_len / 2) * buf_index);

    return buf_addr;
}

___interrupt
static void alink_isr(void)
{
    u16 reg;
    s16 *buf_addr = NULL ;
    u8 ch = 0;

    reg = JL_ALNK0->CON2;

    //channel 0
    if (reg & BIT(4)) {
        ch = 0;
        ALINK_CLR_CH0_PND();
        buf_addr = alink_addr(ALINK_CH0);
        goto __isr_cb;
    }
    //channel 1
    if (reg & BIT(5)) {
        ch = 1;
        ALINK_CLR_CH1_PND();
        buf_addr = alink_addr(ALINK_CH1);
        goto __isr_cb;
    }
    //channel 2
    if (reg & BIT(6)) {
        ch = 2;
        ALINK_CLR_CH2_PND();
        buf_addr = alink_addr(ALINK_CH2);
        goto __isr_cb;
    }
    //channel 3
    if (reg & BIT(7)) {
        ch = 3;
        ALINK_CLR_CH3_PND();
        buf_addr = alink_addr(ALINK_CH3);
        goto __isr_cb;
    }

__isr_cb:
    if (p_alink_parm->ch_cfg[ch].isr_cb) {
        p_alink_parm->ch_cfg[ch].isr_cb(ch, buf_addr, p_alink_parm->dma_len / 2);
    }
}

static void alink_sr(u32 rate)
{
    alink_printf("ALINK_SR = %d\n", rate);
    switch (rate) {
    case ALINK_SR_48000:
        /*12.288Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_44100:
        /*11.289Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_32000:
        /*12.288Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_384FS);
        break ;

    case ALINK_SR_24000:
        /*12.288Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_2);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_22050:
        /*11.289Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_2);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_16000:
        /*12.288/2Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_2);
        ALINK_LRDIV(MCLK_LRDIV_384FS);
        break ;

    case ALINK_SR_12000:
        /*12.288/2Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_4);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_11025:
        /*11.289/2Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_4);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_8000:
        /*12.288/4Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_4);
        ALINK_LRDIV(MCLK_LRDIV_384FS);
        break ;

    default:
        //44100
        /*11.289Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break;
    }
    if (p_alink_parm->role == ALINK_ROLE_SLAVE) {
        ALINK_LRDIV(MCLK_LRDIV_EX);
    }
}


void alink_channel_init(ALINK_PORT port, u8 ch_idx, u8 dir, void (*handle)(u8 ch, s16 *buf, u32 len))
{
    if (!p_alink_parm->ch_cfg[ch_idx].enable) {
        return;
    }
    p_alink_parm->ch_cfg[ch_idx].dir = dir;
    p_alink_parm->ch_cfg[ch_idx].isr_cb = handle;
    p_alink_parm->ch_cfg[ch_idx].buf = malloc(p_alink_parm->dma_len);
    memset(p_alink_parm->ch_cfg[ch_idx].buf, 0xFF, p_alink_parm->dma_len);
    u32 *buf_addr;
    //===================================//
    //           ALNK工作模式            //
    //===================================//
    if (p_alink_parm->mode > ALINK_MD_IIS_RALIGN) {
        ALINK_CHx_MODE_SEL((p_alink_parm->mode - ALINK_MD_IIS_RALIGN), ch_idx);
    } else {
        ALINK_CHx_MODE_SEL(p_alink_parm->mode, ch_idx);
    }
    //===================================//
    //           ALNK CH DMA BUF         //
    //===================================//
    buf_addr = ALNK_BUF_ADR[ch_idx];
    *buf_addr = (u32)(p_alink_parm->ch_cfg[ch_idx].buf);
    //===================================//
    //          ALNK CH DAT IO INIT      //
    //===================================//
    if (p_alink_parm->ch_cfg[ch_idx].dir == ALINK_DIR_RX) {
        gpio_set_direction(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 1);
        gpio_set_pull_down(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 0);
        gpio_set_pull_up(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 1);
        gpio_set_die(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 1);
        ALINK_CHx_DIR_RX_MODE(ch_idx);
    } else {
        gpio_direction_output(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 1);
        ALINK_CHx_DIR_TX_MODE(ch_idx);
    }
}

static void alink_info_dump()
{
    alink_printf("JL_ALNK0->CON0 = 0x%x", JL_ALNK0->CON0);
    alink_printf("JL_ALNK0->CON1 = 0x%x", JL_ALNK0->CON1);
    alink_printf("JL_ALNK0->CON2 = 0x%x", JL_ALNK0->CON2);
    alink_printf("JL_ALNK0->CON3 = 0x%x", JL_ALNK0->CON3);
    alink_printf("JL_ALNK0->LEN  = 0x%x", JL_ALNK0->LEN);
    alink_printf("JL_ALNK0->ADR0 = 0x%x", JL_ALNK0->ADR0);
    alink_printf("JL_ALNK0->ADR1 = 0x%x", JL_ALNK0->ADR1);
    alink_printf("JL_ALNK0->ADR2 = 0x%x", JL_ALNK0->ADR2);
    alink_printf("JL_ALNK0->ADR3 = 0x%x", JL_ALNK0->ADR3);
    alink_printf("JL_CLOCK->CLK_CON2 = 0x%x", JL_CLOCK->CLK_CON2);
}

int alink_init(ALINK_PARM *parm)
{
    if (parm == NULL) {
        return -1;
    }

    ALNK_CON_RESET();

    p_alink_parm = parm;
    if (p_alink_parm->port_select == ALINK0_PORTA) {
        for (int i = 0; i < 7; i++) {
            alink0_IOS_port[i] = alink0_IOS_portA[i];
        }
        JL_IOMAP->CON1 &= ~BIT(23);
        r_printf("iis_portA init\n");
    } else {
        for (int i = 0; i < 7; i++) {
            alink0_IOS_port[i] = alink0_IOS_portB[i];
        }
        JL_IOMAP->CON1 |= BIT(23);
        r_printf("iis_portB init\n");
    }
    request_irq(IRQ_ALINK0_IDX, 3, alink_isr, 0);
    ALINK_MSRC(MCLK_PLL_CLK);	/*MCLK source*/

    //===================================//
    //        输出时钟配置               //
    //===================================//
    if (parm->role == ALINK_ROLE_MASTER) {
        //主机输出时钟
        gpio_direction_output(alink0_IOS_port[IIS_IO_MCLK], 1);
        ALINK_MOE(1);				/*MCLK output to IO*/
        gpio_direction_output(alink0_IOS_port[IIS_IO_LRCK], 1);
        gpio_direction_output(alink0_IOS_port[IIS_IO_SCLK], 1);
        ALINK_SOE(1);				/*SCLK/LRCK output to IO*/
    } else {
        //从机输入时钟
        alink_clk_io_in_init(alink0_IOS_port[IIS_IO_MCLK]);
        ALINK_MOE(0);				/*MCLK input to IO*/
        alink_clk_io_in_init(alink0_IOS_port[IIS_IO_LRCK]);
        alink_clk_io_in_init(alink0_IOS_port[IIS_IO_SCLK]);
        ALINK_SOE(0);				/*SCLK/LRCK output to IO*/
    }
    //===================================//
    //        基本模式/扩展模式          //
    //===================================//
    ALINK_DSPE(p_alink_parm->mode >> 2);

    //===================================//
    //         数据位宽16/32bit          //
    //===================================//
    //注意: 配置了24bit, 一定要选ALINK_FRAME_64SCLK, 因为sclk32 x 2才会有24bit;
    if (p_alink_parm->bitwide == ALINK_LEN_24BIT) {
        ASSERT(p_alink_parm->sclk_per_frame == ALINK_FRAME_64SCLK);
        ALINK_24BIT_MODE();
        //一个点需要4bytes, LR = 2, 双buf = 2
        JL_ALNK0->LEN = p_alink_parm->dma_len / 8; //点数
    } else {
        ALINK_16BIT_MODE();
        //一个点需要2bytes, LR = 2, 双buf = 2
        JL_ALNK0->LEN = p_alink_parm->dma_len / 8; //点数
    }
    //===================================//
    //             时钟边沿选择          //
    //===================================//
    if (p_alink_parm->clk_mode == ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE) {
        SCLKINV(0);
    } else {
        SCLKINV(1);
    }
    //===================================//
    //            每帧数据sclk个数       //
    //===================================//
    if (p_alink_parm->sclk_per_frame == ALINK_FRAME_64SCLK) {
        F32_EN(0);
    } else {
        F32_EN(1);
    }

    //===================================//
    //            设置数据采样率       	 //
    //===================================//
    alink_sr(p_alink_parm->sample_rate);

    return 0;
}

void alink_channel_close(ALINK_PORT port, u8 ch_idx)
{
    gpio_set_pull_up(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 0);
    gpio_set_pull_down(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 0);
    gpio_set_direction(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 1);
    gpio_set_die(alink0_IOS_port[IIS_IO_CH0 + ch_idx], 0);
    ALINK_CHx_CLOSE(1, ch_idx);
    if (p_alink_parm->ch_cfg[ch_idx].buf) {
        free(p_alink_parm->ch_cfg[ch_idx].buf);
        p_alink_parm->ch_cfg[ch_idx].buf = NULL;
    }
    r_printf("alink_ch %d closed\n", ch_idx);
}


int alink_start(ALINK_PORT port)
{
    if (p_alink_parm) {
        ALINK_EN(1);
        return 0;
    }
    return -1;
}

void alink_uninit(ALINK_PORT port)
{
    ALINK_EN(0);
    for (int i = 0; i < 4; i++) {
        if (p_alink_parm->ch_cfg[i].buf) {
            alink_channel_close(port, i);
        }
    }
    p_alink_parm = NULL;
    alink_printf("audio_link_exit OK\n");
}

int alink_sr_set(ALINK_PORT port, u16 sr)
{
    if (p_alink_parm) {
        ALINK_EN(0);
        alink_sr(sr);
        ALINK_EN(1);
        return 0;
    } else {
        return -1;
    }
}

extern ALINK_PARM alink_param;
static u8 alink_init_cnt = 0;
void audio_link_init(ALINK_PORT port)
{
    if (alink_init_cnt == 0) {
        alink_init(&alink_param);
        alink_start(port);
    }
    alink_init_cnt++;
}

void audio_link_uninit(ALINK_PORT port)
{
    alink_init_cnt--;
    if (alink_init_cnt == 0) {
        alink_uninit(port);
    }
}


//===============================================//
//			     for APP use demo                //
//===============================================//

#ifdef ALINK_TEST_ENABLE

short const tsin_441k[441] = {
    0x0000, 0x122d, 0x23fb, 0x350f, 0x450f, 0x53aa, 0x6092, 0x6b85, 0x744b, 0x7ab5, 0x7ea2, 0x7fff, 0x7ec3, 0x7af6, 0x74ab, 0x6c03,
    0x612a, 0x545a, 0x45d4, 0x35e3, 0x24db, 0x1314, 0x00e9, 0xeeba, 0xdce5, 0xcbc6, 0xbbb6, 0xad08, 0xa008, 0x94fa, 0x8c18, 0x858f,
    0x8181, 0x8003, 0x811d, 0x84ca, 0x8af5, 0x9380, 0x9e3e, 0xaaf7, 0xb969, 0xc94a, 0xda46, 0xec06, 0xfe2d, 0x105e, 0x223a, 0x3365,
    0x4385, 0x5246, 0x5f5d, 0x6a85, 0x7384, 0x7a2d, 0x7e5b, 0x7ffa, 0x7f01, 0x7b75, 0x7568, 0x6cfb, 0x6258, 0x55b7, 0x4759, 0x3789,
    0x2699, 0x14e1, 0x02bc, 0xf089, 0xdea7, 0xcd71, 0xbd42, 0xae6d, 0xa13f, 0x95fd, 0x8ce1, 0x861a, 0x81cb, 0x800b, 0x80e3, 0x844e,
    0x8a3c, 0x928c, 0x9d13, 0xa99c, 0xb7e6, 0xc7a5, 0xd889, 0xea39, 0xfc5a, 0x0e8f, 0x2077, 0x31b8, 0x41f6, 0x50de, 0x5e23, 0x697f,
    0x72b8, 0x799e, 0x7e0d, 0x7fee, 0x7f37, 0x7bed, 0x761f, 0x6ded, 0x6380, 0x570f, 0x48db, 0x392c, 0x2855, 0x16ad, 0x048f, 0xf259,
    0xe06b, 0xcf20, 0xbed2, 0xafd7, 0xa27c, 0x9705, 0x8db0, 0x86ab, 0x821c, 0x801a, 0x80b0, 0x83da, 0x8988, 0x919c, 0x9bee, 0xa846,
    0xb666, 0xc603, 0xd6ce, 0xe86e, 0xfa88, 0x0cbf, 0x1eb3, 0x3008, 0x4064, 0x4f73, 0x5ce4, 0x6874, 0x71e6, 0x790a, 0x7db9, 0x7fdc,
    0x7f68, 0x7c5e, 0x76d0, 0x6ed9, 0x64a3, 0x5863, 0x4a59, 0x3acc, 0x2a0f, 0x1878, 0x0661, 0xf42a, 0xe230, 0xd0d0, 0xc066, 0xb145,
    0xa3bd, 0x9813, 0x8e85, 0x8743, 0x8274, 0x8030, 0x8083, 0x836b, 0x88da, 0x90b3, 0x9acd, 0xa6f5, 0xb4ea, 0xc465, 0xd515, 0xe6a3,
    0xf8b6, 0x0aee, 0x1ced, 0x2e56, 0x3ecf, 0x4e02, 0x5ba1, 0x6764, 0x710e, 0x786f, 0x7d5e, 0x7fc3, 0x7f91, 0x7cc9, 0x777a, 0x6fc0,
    0x65c1, 0x59b3, 0x4bd3, 0x3c6a, 0x2bc7, 0x1a41, 0x0833, 0xf5fb, 0xe3f6, 0xd283, 0xc1fc, 0xb2b7, 0xa503, 0x9926, 0x8f60, 0x87e1,
    0x82d2, 0x804c, 0x805d, 0x8303, 0x8833, 0x8fcf, 0x99b2, 0xa5a8, 0xb372, 0xc2c9, 0xd35e, 0xe4da, 0xf6e4, 0x091c, 0x1b26, 0x2ca2,
    0x3d37, 0x4c8e, 0x5a58, 0x664e, 0x7031, 0x77cd, 0x7cfd, 0x7fa3, 0x7fb4, 0x7d2e, 0x781f, 0x70a0, 0x66da, 0x5afd, 0x4d49, 0x3e04,
    0x2d7d, 0x1c0a, 0x0a05, 0xf7cd, 0xe5bf, 0xd439, 0xc396, 0xb42d, 0xa64d, 0x9a3f, 0x9040, 0x8886, 0x8337, 0x806f, 0x803d, 0x82a2,
    0x8791, 0x8ef2, 0x989c, 0xa45f, 0xb1fe, 0xc131, 0xd1aa, 0xe313, 0xf512, 0x074a, 0x195d, 0x2aeb, 0x3b9b, 0x4b16, 0x590b, 0x6533,
    0x6f4d, 0x7726, 0x7c95, 0x7f7d, 0x7fd0, 0x7d8c, 0x78bd, 0x717b, 0x67ed, 0x5c43, 0x4ebb, 0x3f9a, 0x2f30, 0x1dd0, 0x0bd6, 0xf99f,
    0xe788, 0xd5f1, 0xc534, 0xb5a7, 0xa79d, 0x9b5d, 0x9127, 0x8930, 0x83a2, 0x8098, 0x8024, 0x8247, 0x86f6, 0x8e1a, 0x978c, 0xa31c,
    0xb08d, 0xbf9c, 0xcff8, 0xe14d, 0xf341, 0x0578, 0x1792, 0x2932, 0x39fd, 0x499a, 0x57ba, 0x6412, 0x6e64, 0x7678, 0x7c26, 0x7f50,
    0x7fe6, 0x7de4, 0x7955, 0x7250, 0x68fb, 0x5d84, 0x5029, 0x412e, 0x30e0, 0x1f95, 0x0da7, 0xfb71, 0xe953, 0xd7ab, 0xc6d4, 0xb725,
    0xa8f1, 0x9c80, 0x9213, 0x89e1, 0x8413, 0x80c9, 0x8012, 0x81f3, 0x8662, 0x8d48, 0x9681, 0xa1dd, 0xaf22, 0xbe0a, 0xce48, 0xdf89,
    0xf171, 0x03a6, 0x15c7, 0x2777, 0x385b, 0x481a, 0x5664, 0x62ed, 0x6d74, 0x75c4, 0x7bb2, 0x7f1d, 0x7ff5, 0x7e35, 0x79e6, 0x731f,
    0x6a03, 0x5ec1, 0x5193, 0x42be, 0x328f, 0x2159, 0x0f77, 0xfd44, 0xeb1f, 0xd967, 0xc877, 0xb8a7, 0xaa49, 0x9da8, 0x9305, 0x8a98,
    0x848b, 0x80ff, 0x8006, 0x81a5, 0x85d3, 0x8c7c, 0x957b, 0xa0a3, 0xadba, 0xbc7b, 0xcc9b, 0xddc6, 0xefa2, 0x01d3, 0x13fa, 0x25ba,
    0x36b6, 0x4697, 0x5509, 0x61c2, 0x6c80, 0x750b, 0x7b36, 0x7ee3, 0x7ffd, 0x7e7f, 0x7a71, 0x73e8, 0x6b06, 0x5ff8, 0x52f8, 0x444a,
    0x343a, 0x231b, 0x1146, 0xff17, 0xecec, 0xdb25, 0xca1d, 0xba2c, 0xaba6, 0x9ed6, 0x93fd, 0x8b55, 0x850a, 0x813d, 0x8001, 0x815e,
    0x854b, 0x8bb5, 0x947b, 0x9f6e, 0xac56, 0xbaf1, 0xcaf1, 0xdc05, 0xedd3
};
static u16 tx_s_cnt = 0;
int get_sine_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 441) {
            *s_cnt = 0;
        }
        *data++ = tsin_441k[*s_cnt];
        if (ch == 2) {
            *data++ = tsin_441k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}

u32 data_buf[2][ALNK_BUF_POINTS_NUM * 2] __attribute__((aligned(4)));

void handle_tx(u8 ch, void *buf, u16 len)
{
    get_sine_data(&tx_s_cnt, buf, len / 2 / 2, 2);
    /* put_buf(buf, 32); */
#if 0
    s16 *data_tx = (s16 *)buf;
    s16 *data = (s16 *)data_buf;
    for (int i = 0; i < len / sizeof(s16) ; i++) {
        /* data_tx[i] = data[i]; */
        data_tx[i] = 0x5a5a;
    }
#endif
}

void handle_rx(u8 ch, void *buf, u16 len)
{
    /* put_buf(buf, 32); */
#if 0
    s16 *data_rx = (s16 *)buf;
    s16 *data_b = (s16 *)data_buf;
    for (int i = 0; i < len / sizeof(s16) ; i++) {
        data_b[i] = data_rx[i];
    }
#endif
}

ALINK_PARM	test_alink = {
    .port_select = ALINK0_PORTA,            //MCLK:PA2 SCLK:PA3  LRCK:PA4 CH0:PA0 CH1:PA1 CH2:PA5 CH3:PA6
    /* .port_select = ALINK0_PORTB,            //MCLK:PC2 SCLK:PA5  LRCK:PA6 CH0:PA0 CH1:PA2 CH2:PA3 CH3:PA4 */
    .ch_cfg[0].enable = 1,
    .ch_cfg[1].enable = 1,
    .mode = ALINK_MD_IIS,
    /* .role = ALINK_ROLE_SLAVE, */
    .role = ALINK_ROLE_MASTER,
    .clk_mode = ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE,
    .bitwide = ALINK_LEN_16BIT,
    .sclk_per_frame = ALINK_FRAME_64SCLK,
    /* .sclk_per_frame = ALINK_FRAME_32SCLK, */
    .dma_len = ALNK_BUF_POINTS_NUM * 2 * 2 * 2,
    .sample_rate = 44100,
};

void audio_link_test(void)
{
    alink_init(&test_alink);
    alink_channel_init(0, 0, ALINK_DIR_RX, handle_rx);
    alink_channel_init(0, 1, ALINK_DIR_TX, handle_tx);
    alink_start(0);
    alink_info_dump();
}


#endif


#endif

