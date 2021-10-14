#include "app_config.h"
#include "asm/clock.h"
#include "system/timer.h"
/* #include "asm/uart_dev.h" */
#include "max30102.h"
#include "algorithm.h"
#include "asm/cpu.h"
#include "generic/typedef.h"
#include "generic/gpio.h"

#define TCFG_MAX30102_DEV_ENABLE  1
#if defined(TCFG_MAX30102_DEV_ENABLE) && TCFG_MAX30102_DEV_ENABLE

#undef LOG_TAG_CONST
#define LOG_TAG     "[max30102]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

/*****************************algorithm 1*******************************/
//反序映射表
static const unsigned long ReserveSeries[256] = {
    0, 128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208, 48, 176, 112, 240, 8, 136, 72, 200,
    40, 168, 104, 232, 24, 152, 88, 216, 56, 184, 120, 248, 4, 132, 68, 196, 36, 164, 100, 228,
    20, 148, 84, 212, 52, 180, 116, 244, 12, 140, 76, 204, 44, 172, 108, 236, 28, 156, 92, 220,
    60, 188, 124, 252, 2, 130, 66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50, 178, 114, 242,
    10, 138, 74, 202, 42, 170, 106, 234, 26, 154, 90, 218, 58, 186, 122, 250, 6, 134, 70, 198,
    38, 166, 102, 230, 22, 150, 86, 214, 54, 182, 118, 246, 14, 142, 78, 206, 46, 174, 110, 238,
    30, 158, 94, 222, 62, 190, 126, 254, 1, 129, 65, 193, 33, 161, 97, 225, 17, 145, 81, 209,
    49, 177, 113, 241, 9, 137, 73, 201, 41, 169, 105, 233, 25, 153, 89, 217, 57, 185, 121, 249,
    5, 133, 69, 197, 37, 165, 101, 229, 21, 149, 85, 213, 53, 181, 117, 245, 13, 141, 77, 205,
    45, 173, 109, 237, 29, 157, 93, 221, 61, 189, 125, 253, 3, 131, 67, 195, 35, 163, 99, 227,
    19, 147, 83, 211, 51, 179, 115, 243, 11, 139, 75, 203, 43, 171, 107, 235, 27, 155, 91, 219,
    59, 187, 123, 251, 7, 135, 71, 199, 39, 167, 103, 231, 23, 151, 87, 215, 55, 183, 119, 247,
    15, 143, 79, 207, 47, 175, 111, 239, 31, 159, 95, 223, 63, 191, 127, 255
};

//旋转因子，为了适应整数运算，这里进行归一化
/* WN[i]_Reality = WN[i]/100000   */
static const ComplexNum WN[128] = {
    {100000, 0}, {99969, 2454}, {99879, 4906}, {99729, 7356}, {99518, 9801},
    {99247, 12241}, {98917, 14673}, {98527, 17096}, {98078, 19508}, {97570, 21910},
    {97003, 24298}, {96377, 26671}, {95693, 29028}, {94952, 31368}, {94154, 33689},
    {93299, 35989}, {92387, 38268}, {91421, 40524}, {90398, 42755}, {89322, 44961},
    {88192, 47139}, {87008, 49289}, {85772, 51410}, {84485, 53499}, {83147, 55557},
    {81758, 57580}, {80320, 59569}, {78834, 61523}, {77301, 63439}, {75720, 65317},
    {74095, 67155}, {72424, 68954}, {70710, 70710}, {68954, 72424}, {67155, 74095},
    {65317, 75720}, {63439, 77301}, {61523, 78834}, {59569, 80320}, {57580, 81758},
    {55557, 83147}, {53499, 84485}, {51410, 85772}, {49289, 87008}, {47139, 88192},
    {44961, 89322}, {42755, 90398}, {40524, 91421}, {38268, 92387}, {35989, 93299},
    {33689, 94154}, {31368, 94952}, {29028, 95693}, {26671, 96377}, {24298, 97003},
    {21910, 97570}, {19508, 98078}, {17096, 98527}, {14673, 98917}, {12241, 99247},
    {9801, 99518}, {7356, 99729}, {4906, 99879}, {2454, 99969}, {0, 100000},
    {-2454, 99969}, {-4906, 99879}, {-7356, 99729}, {-9801, 99518}, {-12241, 99247},
    {-14673, 98917}, {-17096, 98527}, {-19508, 98078}, {-21910, 97570}, {-24298, 97003},
    {-26671, 96377}, {-29028, 95693}, {-31368, 94952}, {-33689, 94154}, {-35989, 93299},
    {-38268, 92387}, {-40524, 91421}, {-42755, 90398}, {-44961, 89322}, {-47139, 88192},
    {-49289, 87008}, {-51410, 85772}, {-53499, 84485}, {-55557, 83147}, {-57580, 81758},
    {-59569, 80320}, {-61523, 78834}, {-63439, 77301}, {-65317, 75720}, {-67155, 74095},
    {-68954, 72424}, {-70710, 70710}, {-72424, 68954}, {-74095, 67155}, {-75720, 65317},
    {-77301, 63439}, {-78834, 61523}, {-80320, 59569}, {-81758, 57580}, {-83147, 55557},
    {-84485, 53499}, {-85772, 51410}, {-87008, 49289}, {-88192, 47139}, {-89322, 44961},
    {-90398, 42755}, {-91421, 40524}, {-92387, 38268}, {-93299, 35989}, {-94154, 33689},
    {-94952, 31368}, {-95693, 29028}, {-96377, 26671}, {-97003, 24298}, {-97570, 21910},
    {-98078, 19508}, {-98527, 17096}, {-98917, 14673}, {-99247, 12241}, {-99518, 9801},
    {-99729, 7356}, {-99879, 4906}, {-99969, 2454}
};


//蝶形运算，计算过程中进行了归一化，否则数据溢出，发生削波现象
static void  FunButterFlyCalculate(ComplexNum *Xi1, ComplexNum *Xi2, ComplexNum *Xo1, ComplexNum *Xo2, int WN_n)
{
    ComplexNum TimesPool = {0, 0};

    TimesPool.RealNum = Xi2->RealNum * WN[WN_n].RealNum - Xi2->ImagNum * WN[WN_n].ImagNum;
    TimesPool.ImagNum = Xi2->RealNum * WN[WN_n].ImagNum + Xi2->ImagNum * WN[WN_n].RealNum;

    Xo1->RealNum = (Xi1->RealNum * 100000 + TimesPool.RealNum) / 100000;
    Xo1->ImagNum = (Xi1->ImagNum * 100000 + TimesPool.ImagNum) / 100000;

    Xo2->RealNum = (Xi1->RealNum * 100000 - TimesPool.RealNum) / 100000;
    Xo2->ImagNum = (Xi1->ImagNum * 100000 - TimesPool.ImagNum) / 100000;
}

static void  FFT_256(ComplexNum *XI, ComplexNum *XO)
{
    int i = 0, j = 0, k = 0;
    for (i = 0; i < 256; i++) {
        *(XO + i) = *(XI + ReserveSeries[i]);
    }
    for (i = 0; i < 256; i++) {
        *(XI + i) = *(XO + i);
    }

    for (i = 1; i <= 128; i <<= 1) {
        for (j = 0; j < 128 / i; j++) {
            for (k = 0; k < i; k++) {
                FunButterFlyCalculate(XI + j * i * 2 + k, XI + j * i * 2 + k + i, (XO + j * i * 2 + k), (XO + j * i * 2 + k + i), k * (128 / i));
            }
        }
        for (k = 0; k < 256; k++) {
            *(XI + k) = *(XO + k);
        }
    }
}

static void IFFT_256(ComplexNum *XI, ComplexNum *XO)
{
    unsigned int i;
    volatile int64_t k;

    for (i = 0; i < 256; i++) {
        k = (XI + i)->ImagNum;
        k = -k ;
        (XI + i)->ImagNum = k;
    }

    FFT_256(XI, XO);

}



/*==================== 心率计算相关函数 ==================*/
u32  RedBuffer[256] = {0};
u32  IrBuffer[256] = {0};
u32   RedAverageTemp[256] = {0};
u32   IrAverageTemp[256] = {0};

static s32  RedAnalysisBuff[256] = {0};


ComplexNum      XI[256] = {{0, 0}};
ComplexNum      XO[256] = {{0, 0}};

u64    FFT_Power[256] = {0};

/*
 * * 函数：FirstOrderDifferential(u32 *PPG_In,const u32 Len, s32 *PPG_Out)
 * * 描述：一阶高通滤波器，即一阶微分
 * * 参数：*PPG_In：待处理的PPG信号序列；  Len：待处理的PPG信号长度；  *PPG_Out：处理后的序列
 * * 返回：无
 * */
static void  FirstOrderDifferential(u32 *PPG_In, const u32 Len, s32 *PPG_Out)
{
    u16 i;
    for (i = 0; i < Len - 1; i++) {
        (*PPG_Out) = ((*(PPG_In + 1)) - (*(PPG_In)));
        PPG_Out ++ ;
        PPG_In  ++ ;
    }
}

/*
 * * 函数：TreeNumMin(MAX30102_S16 x, MAX30102_S16 y, MAX30102_S16 z)
 * * 描述：三阶最小值函数，获取三者中的最小值
 * * 参数：x,y,z三个有符号整数
 * * 返回：最小值
 * */
static s32 TreeNumMin(s32 x, s32 y, s32 z)
{
    s32 min;
    if (x < y) {
        min = x;
    } else {
        min = y;
    }
    if (min < z) {
        return min;
    } else {
        return z;
    }
}

/*
 * * 函数：MinFilter3Rank(s32 *Fx,const u32 Len,s32 *Fo)
 * * 描述：三阶最小值滤波器，用于去除椒盐噪声
 * * 参数：*Fx：待处理的序列； Len：序列的长度； *Fo：处理后的序列
 * * 返回：无
 * */
static void  MinFilter3Rank(s32 *Fx, const u32 Len, s32 *Fo)
{
    u16 i;
    for (i = 0; i < Len - 2; i++) {
        (*Fo) = TreeNumMin(*(Fx), *(Fx + 1), *(Fx + 2));
        Fo ++ ;
        Fx ++ ;
    }
}
/*
 * * 函数：AverageFilter4Rank(MAX30102_S16 *Fx,const u16 Len, MAX30102_S16 *Fx_Out)
 * * 描述：二阶均值滤波器
 * * 参数：*Fx：待处理的序列； Len：序列的长度； *Fo：处理后的序列
 * * 返回：无
 * */
static void AverageFilter2Rank(s32 *Fx, const u32 Len, s32 *Fo)
{
    u16 i;
    for (i = 0; i < Len - 1; i++) {
        (*Fo) = ((*(Fx)) + (*(Fx + 1))) / 2;
        Fo  ++ ;
        Fx  ++ ;
    }
}
/*
 * * 函数：FindFirstPeak(MAX30102_S64 *FFT_Power)
 * * 描述：找出第一个峰值，即找出基波频率点
 * * 参数：FFT_Power ： FFT后的幅值
 * * 返回：第一个峰值，即基波频率点
 * */
static u16 FindFirstPeak(u64 *FFT_Power)
{
    u16 i;
    u64  FirstPeak = 0;

    FirstPeak = 0;
    for (i = 1; i < 255; i++) {
        if (FirstPeak == 0) {
            if ((FFT_Power[i] > FFT_Power[i - 1]) && (FFT_Power[i] > FFT_Power[i + 1])) {
                FirstPeak = FFT_Power[i];
            }

        } else {
            if ((FFT_Power[i] > FFT_Power[i - 1]) && (FFT_Power[i] > FFT_Power[i + 1])) {
                if (FFT_Power[i] > FirstPeak * 10) {
                    if (FFT_Power[i] > 1000000) {
                        return i;
                    }
                } else {
                    FirstPeak = FFT_Power[i];
                }
            }

        }

    }
    return 0;
}

/*
 * * 函数：MAX30102_GetHeartRate(void)
 * * 描述：获取心率
 * * 参数：无
 * * 返回：HR：计算出的心率
 * */
u16  MAX30102_GetHeartRate()
{
    u32 *pRed = NULL, *pIr = NULL;
    u64 SampTimeMs = 0;
    u32 Temp = 0;
    u16 HR = 0, i = 0;

    pRed = RedBuffer;
    pIr  = IrBuffer;
    i = 0;
    u64 time = jiffies_msec();
    while (1) {
        while (JL_PORTA->IN & BIT(1)); //wait New FIFO Data Ready
        max30102_read_fifo(pIr, pRed);
        i++;
        pRed ++;
        pIr  ++;
        /* log_info("i:%d, time:%d",i,jiffies_msec()); */
        if (i == 256) {
            break;
        }
    }
    SampTimeMs = jiffies_msec();
    SampTimeMs -= time;
    /* log_info("collect time:%dms",SampTimeMs); */
    FirstOrderDifferential(RedBuffer, 256, RedAnalysisBuff);
    // MinFilter3Rank(RedAnalysisBuff,499,RedAnalysisBuff);
    // 经过试验测试，使用均值滤波的心率计算效果要好些，这里不使用最小值滤波
    // 最小值滤波的适用于在椒盐噪声较大的环境中使用，一般情况下使用均值滤波即可
    AverageFilter2Rank(RedAnalysisBuff, 255, RedAnalysisBuff);
    for (i = 0; i < 256; i++) {
        XI[i].RealNum = (float)RedAnalysisBuff[i];
        XI[i].ImagNum = 0;
        XO[i].RealNum = 0;
        XO[i].ImagNum = 0;
    }

    FFT_256(XI, XO);
    for (i = 0; i < 256; i++) {
        FFT_Power[i] = XO[i].ImagNum * XO[i].ImagNum + XO[i].RealNum * XO[i].RealNum;
    }

    HR = FindFirstPeak(FFT_Power);

    Temp = HR * 60000;
    HR = (u16)(Temp / SampTimeMs);
    if ((Temp - HR * SampTimeMs) > SampTimeMs / 2) { //进行四舍五入
        HR += 1;
    }

    return HR;
}


#include <math.h>
/*
 * * 函数：GetRedAcAndDc(u32 *PPG_Red ,u32 *RedAC, u32 *RedDC)
 * * 描述：计算Red中的交流分量和直流分量
 * * 参数：*PPG_Red:PPG信号中的Red序列，*RedAc:PPG中的RED的交流分量； *RedDc：PPG中的RED直流分量
 * * 返回：无
 * */
static void  GetRedAcAndDc(u32 *PPG_Red, double *RedAC, double *RedDC)
{
    u16 i = 0;
    for (i = 0; i < 256; i++) {
        XI[i].RealNum = (u64)(*(PPG_Red + i));
        XI[i].ImagNum = 0;
    }
    FFT_256(XI, XO);

    for (i = 0; i < 256; i++) {
        *RedDC += (*(PPG_Red + i));
    }

    *RedDC /= 256;

    for (i = 1; i < 256; i++) {
        *RedAC += sqrt(XO[i].ImagNum * XO[i].ImagNum + XO[i].RealNum * XO[i].RealNum);
    }

}

static void Average5Filter(u32 *PPG_RedIn, u32 *PPG_IrIn, u32 *PPG_RedOut, u32 *PPG_IrOut)
{
    unsigned int i;
    *PPG_RedOut = *PPG_RedIn;
    *(PPG_RedOut + 1) = *(PPG_RedIn + 1);
    *(PPG_RedOut + 254) = *(PPG_RedIn + 254);
    *(PPG_RedOut + 255) = *(PPG_RedIn + 255);

    *PPG_IrOut = *PPG_IrIn;
    *(PPG_IrOut + 1) = *(PPG_IrIn + 1);
    *(PPG_IrOut + 254) = *(PPG_IrIn + 254);
    *(PPG_IrOut + 255) = *(PPG_IrIn + 255);
    for (i = 2; i < 254; i++) {
        RedAverageTemp[i] = (*(PPG_RedIn + i - 2) + * (PPG_RedIn + i - 1) + * (PPG_RedIn + i) + * (PPG_RedIn + i + 1) + * (PPG_RedIn + i + 2)) / 5;
        IrAverageTemp[i] = (*(PPG_IrIn + i - 2) + * (PPG_IrIn + i - 1) + * (PPG_IrIn + i) + * (PPG_IrIn + i + 1) + * (PPG_IrIn + i + 2)) / 5;
    }

    for (i = 2; i < 254; i++) {
        *(PPG_RedOut + i) = RedAverageTemp[i];
        *(PPG_IrOut + i) = IrAverageTemp[i];
    }

}

/*
 * * 函数：GetIrAcAndDc(u32 *PPG_Ir ,u32 *IrAC, u32 *IrDC)
 * * 描述：计算IR中的交流分量和直流分量
 * * 参数：*PPG_RIr:PPG信号中的IR序列，*IrAc:PPG中的IR的交流分量； *IrDc：PPG中的IR直流分量
 * * 返回：无
 * */
static void  GetIrAcAndDc(u32 *PPG_Ir, double *IrAC, double *IrDC)
{
    u16 i = 0;
    for (i = 0; i < 256; i++) {
        XI[i].RealNum = (u64)(*(PPG_Ir + i));
        XI[i].ImagNum = 0;
    }
    FFT_256(XI, XO);

    for (i = 0; i < 256; i++) {
        *IrDC += (*(PPG_Ir + i));
    }

    *IrDC /= 256;

    for (i = 1; i < 256; i++) {
        *IrAC += sqrtf(XO[i].ImagNum * XO[i].ImagNum + XO[i].RealNum * XO[i].RealNum);
    }

}

/*
 * * 函数：MAX30102_GetSPO2
 * * 描述：获取MAX30102血氧浓度
 * * 参数：无
 * * 返回：SPO2
 **/
float MAX30102_GetSPO2(void)
{
    double SPO2;
    double R_Temp;
    double RedAC_Temp = 0, RedDC_Temp = 0;
    double IrAC_Temp = 0, IrDC_Temp = 0;
    Average5Filter(RedBuffer, IrBuffer, RedBuffer, IrBuffer);
    GetRedAcAndDc(RedBuffer, &RedAC_Temp, &RedDC_Temp);
    GetIrAcAndDc(IrBuffer, &IrAC_Temp, &IrDC_Temp);

    R_Temp = (IrAC_Temp / IrDC_Temp) / (RedAC_Temp / RedDC_Temp);
    log_info("R:%d", R_Temp);
    SPO2 = -45.060 * R_Temp * R_Temp + 30.354 * R_Temp + 94.845;

    if (SPO2 > 75 && SPO2 < 100) {
        return SPO2;
    }

    return 0;
}




/************************algorithm 2****************************/

/*****************calculate the heart rate and spo2 level**************/
/*Sort indices according to descending order */
static void max301x_sort_indices_descend(int *pn_x, int *pn_indx, int n_size)
{
    int i, j, n_temp;
    for (i = 1; i < n_size; i++) {
        n_temp = pn_indx[i];
        for (j = i; j > 0 && pn_x[n_temp] > pn_x[pn_indx[j - 1]]; j--) {
            pn_indx[j] = pn_indx[j - 1];
        }
        pn_indx[j] = n_temp;
    }
}
/* Sort array in ascending order */
static void max301x_sort_ascend(int *pn_x, int n_size)
{
    int i, j, n_temp;
    for (i = 1; i < n_size; i++) {
        n_temp = pn_x[i];
        for (j = i; j > 0 && n_temp < pn_x[j - 1]; j--) {
            pn_x[j] = pn_x[j - 1];
        }
        pn_x[j] = n_temp;
    }
}

/*Remove peaks separated by less than MIN_DISTANCE */
static void max301x_remove_close_peaks(int *pn_locs, int *pn_npks, int *pn_x, int n_min_distance)
{

    int i, j, n_old_npks, n_dist;

    /* Order peaks from large to small */
    max301x_sort_indices_descend(pn_x, pn_locs, *pn_npks);

    for (i = -1; i < *pn_npks; i++) {
        n_old_npks = *pn_npks;
        *pn_npks = i + 1;
        for (j = i + 1; j < n_old_npks; j++) {
            n_dist =  pn_locs[j] - (i == -1 ? -1 : pn_locs[i]);   // lag-zero peak of autocorr is at index -1
            if (n_dist > n_min_distance || n_dist < -n_min_distance) {
                pn_locs[(*pn_npks)++] = pn_locs[j];
            }
        }
    }

    /* Resort indices longo ascending order */
    max301x_sort_ascend(pn_locs, *pn_npks);
}

/*Find all peaks above MIN_HEIGHT */
static void max301x_peaks_above_min_height(int *pn_locs, int *pn_npks, int  *pn_x, int n_size, int n_min_height)
{
    int i = 1, n_width;
    *pn_npks = 0;

    while (i < n_size - 1) {
        if (pn_x[i] > n_min_height && pn_x[i] > pn_x[i - 1]) {       // find left edge of potential peaks
            n_width = 1;
            while (i + n_width < n_size && pn_x[i] == pn_x[i + n_width]) { // find flat peaks
                n_width++;
            }
            if (pn_x[i] > pn_x[i + n_width] && (*pn_npks) < 15) {    // find right edge of peaks
                pn_locs[(*pn_npks)++] = i;
                /* for flat peaks, peak location is left edge */
                i += n_width + 1;
            } else {
                i += n_width;
            }
        } else {
            i++;
        }
    }
}

/* Find at most MAX_NUM peaks above MIN_HEIGHT separated by at least MIN_DISTANCE */

static void max301x_find_peaks(int *pn_locs, int *pn_npks, int *pn_x, int n_size, int n_min_height, int n_min_distance, int n_max_num)
{
    max301x_peaks_above_min_height(pn_locs, pn_npks, pn_x, n_size, n_min_height);
    max301x_remove_close_peaks(pn_locs, pn_npks, pn_x, n_min_distance);
    *pn_npks = *pn_npks < n_max_num ? *pn_npks : n_max_num;
}


static const u16 auw_hamm[31] = { 41,    276,    512,    276,     41 }; //Hamm=  long16(512* hamming(5)');
//uch_spo2_table is computed as  -45.060*ratioAverage* ratioAverage + 30.354 *ratioAverage + 94.845 ;
static const u8 uch_spo2_table[184] = { 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99,
                                        99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
                                        100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97,
                                        97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91,
                                        90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81,
                                        80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67,
                                        66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50,
                                        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29,
                                        28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5,
                                        3, 2, 1
                                      } ;
static  int an_dx[ BUFFER_SIZE - MA4_SIZE]; // delta
static  int an_x[ BUFFER_SIZE]; //ir
static  int an_y[ BUFFER_SIZE]; //red


/** Calculate the heart rate and SpO2 level
* \param[in]    *pun_ir_buffer           - IR sensor data buffer
* \param[in]    n_ir_buffer_length      - IR sensor data buffer length
* \param[in]    *pun_red_buffer          - Red sensor data buffer
* \param[out]   *pn_spo2                - Calculated SpO2 value
* \param[out]   *pch_spo2_valid         - 1 if the calculated SpO2 value is valid
* \param[out]   *pn_heart_rate          - Calculated heart rate value
* \param[out]   *pch_hr_valid           - 1 if the calculated heart rate value is valid
*/
#if Calculate_type
void max301x_heart_rate_and_oxygen_saturation(u32 *pun_ir_buffer,  int n_ir_buffer_length, u32 *pun_red_buffer, int *pn_spo2, char *pch_spo2_valid, int *pn_heart_rate, char  *pch_hr_valid)
{
    u32 un_ir_mean, un_only_once ;
    int k, n_i_ratio_count;
    int i, s, m, n_exact_ir_valley_locs_count, n_middle_idx;
    int n_th1, n_npks, n_c_min;
    int an_ir_valley_locs[15] ;
    int an_exact_ir_valley_locs[15] ;
    int an_dx_peak_locs[15] ;
    int n_peak_interval_sum;

    int n_y_ac, n_x_ac;
    int n_spo2_calc;
    int n_y_dc_max, n_x_dc_max;
    int n_y_dc_max_idx, n_x_dc_max_idx;
    int an_ratio[5], n_ratio_average;
    int n_nume,  n_denom ;
    /* remove DC of ir signal */
    un_ir_mean = 0;
    for (k = 0 ; k < n_ir_buffer_length ; k++) {
        un_ir_mean += pun_ir_buffer[k] ;
    }
    un_ir_mean = un_ir_mean / n_ir_buffer_length ;
    for (k = 0 ; k < n_ir_buffer_length ; k++) {
        an_x[k] =  pun_ir_buffer[k] - un_ir_mean ;
    }

    /* 4 pt Moving Average */
    for (k = 0; k < BUFFER_SIZE - MA4_SIZE; k++) {
        n_denom = (an_x[k] + an_x[k + 1] + an_x[k + 2] + an_x[k + 3]);
        an_x[k] =  n_denom / (int)4;
    }

    /* get difference of smoothed IR signal */
    for (k = 0; k < BUFFER_SIZE - MA4_SIZE - 1;  k++) {
        an_dx[k] = (an_x[k + 1] - an_x[k]);
    }

    /* 2-pt Moving Average to an_dx */
    for (k = 0; k < BUFFER_SIZE - MA4_SIZE - 2; k++) {
        an_dx[k] = (an_dx[k] + an_dx[k + 1]) / 2 ;
    }

    /* hamming window */
    /* flip wave form so that we can detect valley with peak detector */
    for (i = 0 ; i < BUFFER_SIZE - HAMMING_SIZE - MA4_SIZE - 2 ; i++) {
        s = 0;
        for (k = i; k < i + HAMMING_SIZE ; k++) {
            s -= an_dx[k] * auw_hamm[k - i] ;
        }
        an_dx[i] = s / (int)1146; // divide by sum of auw_hamm
    }

    n_th1 = 0; // threshold calculation
    for (k = 0 ; k < BUFFER_SIZE - HAMMING_SIZE ; k++) {
        n_th1 += ((an_dx[k] > 0) ? an_dx[k] : ((int)0 - an_dx[k])) ;
    }
    n_th1 = n_th1 / (BUFFER_SIZE - HAMMING_SIZE);
    /* peak location is acutally index for sharpest location of raw signal since we flipped the signal */
    max301x_find_peaks(an_dx_peak_locs, &n_npks, an_dx, BUFFER_SIZE - HAMMING_SIZE, n_th1, 8, 5);//peak_height, peak_distance, max_num_peaks

    n_peak_interval_sum = 0;
    if (n_npks >= 2) {
        for (k = 1; k < n_npks; k++) {
            n_peak_interval_sum += (an_dx_peak_locs[k] - an_dx_peak_locs[k - 1]);
        }
        n_peak_interval_sum = n_peak_interval_sum / (n_npks - 1);
        *pn_heart_rate = (int)(6000 / n_peak_interval_sum); // beats per minutes
        *pch_hr_valid  = 1;
    } else  {
        *pn_heart_rate = -999;
        *pch_hr_valid  = 0;
    }
    for (k = 0 ; k < n_npks ; k++) {
        an_ir_valley_locs[k] = an_dx_peak_locs[k] + HAMMING_SIZE / 2;
    }

    /* raw value : RED(=y) and IR(=X) */
    /* we need to assess DC and AC value of ir and red PPG.  */
    for (k = 0 ; k < n_ir_buffer_length ; k++)  {
        an_x[k] =  pun_ir_buffer[k] ;
        an_y[k] =  pun_red_buffer[k] ;
    }

    /* find precise min near an_ir_valley_locs */
    n_exact_ir_valley_locs_count = 0;
    for (k = 0 ; k < n_npks ; k++) {
        un_only_once = 1;
        m = an_ir_valley_locs[k];
        n_c_min = 16777216; //2^24;
        if (m + 5 <  BUFFER_SIZE - HAMMING_SIZE  && m - 5 > 0) {
            for (i = m - 5; i < m + 5; i++)
                if (an_x[i] < n_c_min) {
                    if (un_only_once > 0) {
                        un_only_once = 0;
                    }
                    n_c_min = an_x[i] ;
                    an_exact_ir_valley_locs[k] = i;
                }
            if (un_only_once == 0) {
                n_exact_ir_valley_locs_count ++ ;
            }
        }
    }
    if (n_exact_ir_valley_locs_count < 2) {
        *pn_spo2 =  -999 ; // do not use SPO2 since signal ratio is out of range
        *pch_spo2_valid  = 0;
        return;
    }
    /* 4 pt MA */
    for (k = 0; k < BUFFER_SIZE - MA4_SIZE; k++) {
        an_x[k] = (an_x[k] + an_x[k + 1] + an_x[k + 2] + an_x[k + 3]) / (int)4;
        an_y[k] = (an_y[k] + an_y[k + 1] + an_y[k + 2] + an_y[k + 3]) / (int)4;
    }

    /*using an_exact_ir_valley_locs , find ir-red DC andir-red AC for SPO2 calibration ratio */
    /*finding AC/DC maximum of raw ir * red between two valley locations */
    n_ratio_average = 0;
    n_i_ratio_count = 0;

    for (k = 0; k < 5; k++) {
        an_ratio[k] = 0;
    }
    for (k = 0; k < n_exact_ir_valley_locs_count; k++) {
        if (an_exact_ir_valley_locs[k] > BUFFER_SIZE) {
            *pn_spo2 =  -999 ; // do not use SPO2 since valley loc is out of range
            *pch_spo2_valid  = 0;
            return;
        }
    }
    /* find max between two valley locations */
    /* and use ratio betwen AC compoent of Ir & Red and DC compoent of Ir & Red for SPO2  */
    for (k = 0; k < n_exact_ir_valley_locs_count - 1; k++) {
        n_y_dc_max = -16777216 ;
        n_x_dc_max = - 16777216;
        if (an_exact_ir_valley_locs[k + 1] - an_exact_ir_valley_locs[k] > 10) {
            for (i = an_exact_ir_valley_locs[k]; i < an_exact_ir_valley_locs[k + 1]; i++) {
                if (an_x[i] > n_x_dc_max) {
                    n_x_dc_max = an_x[i];
                    n_x_dc_max_idx = i;
                }
                if (an_y[i] > n_y_dc_max) {
                    n_y_dc_max = an_y[i];
                    n_y_dc_max_idx = i;
                }
            }
            n_y_ac = (an_y[an_exact_ir_valley_locs[k + 1]] - an_y[an_exact_ir_valley_locs[k] ]) * (n_y_dc_max_idx - an_exact_ir_valley_locs[k]); //red
            n_y_ac =  an_y[an_exact_ir_valley_locs[k]] + n_y_ac / (an_exact_ir_valley_locs[k + 1] - an_exact_ir_valley_locs[k])  ;

            n_y_ac =  an_y[n_y_dc_max_idx] - n_y_ac;   // subracting linear DC compoenents from raw
            n_x_ac = (an_x[an_exact_ir_valley_locs[k + 1]] - an_x[an_exact_ir_valley_locs[k] ]) * (n_x_dc_max_idx - an_exact_ir_valley_locs[k]); // ir
            n_x_ac =  an_x[an_exact_ir_valley_locs[k]] + n_x_ac / (an_exact_ir_valley_locs[k + 1] - an_exact_ir_valley_locs[k]);
            n_x_ac =  an_x[n_y_dc_max_idx] - n_x_ac;     // subracting linear DC compoenents from raw
            n_nume = (n_y_ac * n_x_dc_max) >> 7 ; //prepare X100 to preserve floating value
            n_denom = (n_x_ac * n_y_dc_max) >> 7;
            if (n_denom > 0  && n_i_ratio_count < 5 &&  n_nume != 0) {
                an_ratio[n_i_ratio_count] = (n_nume * 100) / n_denom ; //formular is ( n_y_ac *n_x_dc_max) / ( n_x_ac *n_y_dc_max) ;
                n_i_ratio_count++;
            }
        }
    }

    max301x_sort_ascend(an_ratio, n_i_ratio_count);
    n_middle_idx = n_i_ratio_count / 2;

    if (n_middle_idx > 1) {
        n_ratio_average = (an_ratio[n_middle_idx - 1] + an_ratio[n_middle_idx]) / 2;    // use median
    } else {
        n_ratio_average = an_ratio[n_middle_idx ];
    }

    if (n_ratio_average > 2 && n_ratio_average < 184) {
        n_spo2_calc = uch_spo2_table[n_ratio_average] ;
        *pn_spo2 = n_spo2_calc ;
        *pch_spo2_valid  = 1;//  float_SPO2 =  -45.060*n_ratio_average* n_ratio_average/10000 + 30.354 *n_ratio_average/100 + 94.845 ;  // for comparison with table
    } else {
        *pn_spo2 =  -999 ; // do not use SPO2 since signal ratio is out of range
        *pch_spo2_valid  = 0;
    }
}

#else
void max301x_heart_rate_and_oxygen_saturation(u32 *pun_ir_buffer,  int n_ir_buffer_length, u32 *pun_red_buffer, int *pn_spo2, char *pch_spo2_valid, int *pn_heart_rate, char  *pch_hr_valid)
{
    uint32_t un_ir_mean;
    int32_t k, n_i_ratio_count;
    int32_t i, n_exact_ir_valley_locs_count, n_middle_idx;
    int32_t n_th1, n_npks;
    int32_t an_ir_valley_locs[15] ;
    int32_t n_peak_interval_sum;

    int32_t n_y_ac, n_x_ac;
    int32_t n_spo2_calc;
    int32_t n_y_dc_max, n_x_dc_max;
    int32_t n_y_dc_max_idx = 0;
    int32_t n_x_dc_max_idx = 0;
    int32_t an_ratio[5], n_ratio_average;
    int32_t n_nume, n_denom ;

    // calculates DC mean and subtract DC from ir
    un_ir_mean = 0;
    for (k = 0 ; k < n_ir_buffer_length ; k++) {
        un_ir_mean += pun_ir_buffer[k] ;
    }
    un_ir_mean = un_ir_mean / n_ir_buffer_length ;

    // remove DC and invert signal so that we can use peak detector as valley detector
    for (k = 0 ; k < n_ir_buffer_length ; k++) {
        an_x[k] = -1 * (pun_ir_buffer[k] - un_ir_mean) ;
    }

    // 4 pt Moving Average
    for (k = 0; k < BUFFER_SIZE - MA4_SIZE; k++) {
        an_x[k] = (an_x[k] + an_x[k + 1] + an_x[k + 2] + an_x[k + 3]) / (int)4;
    }
    // calculate threshold
    n_th1 = 0;
    for (k = 0 ; k < BUFFER_SIZE ; k++) {
        n_th1 +=  an_x[k];
    }
    n_th1 =  n_th1 / (BUFFER_SIZE);
    if (n_th1 < 30) {
        n_th1 = 30;    // min allowed
    }
    if (n_th1 > 60) {
        n_th1 = 60;    // max allowed
    }

    for (k = 0 ; k < 15; k++) {
        an_ir_valley_locs[k] = 0;
    }
    // since we flipped signal, we use peak detector as valley detector
    max301x_find_peaks(an_ir_valley_locs, &n_npks, an_x, BUFFER_SIZE, n_th1, 4, 15);  //peak_height, peak_distance, max_num_peaks
    n_peak_interval_sum = 0;
    if (n_npks >= 2) {
        for (k = 1; k < n_npks; k++) {
            n_peak_interval_sum += (an_ir_valley_locs[k] - an_ir_valley_locs[k - 1]) ;
        }
        n_peak_interval_sum = n_peak_interval_sum / (n_npks - 1);
        *pn_heart_rate = (int32_t)((FreqS * 60) / n_peak_interval_sum);
        *pch_hr_valid  = 1;
    } else  {
        *pn_heart_rate = -999; // unable to calculate because # of peaks are too small
        *pch_hr_valid  = 0;
    }

    //  load raw value again for SPO2 calculation : RED(=y) and IR(=X)
    for (k = 0 ; k < n_ir_buffer_length ; k++)  {
        an_x[k] =  pun_ir_buffer[k] ;
        an_y[k] =  pun_red_buffer[k] ;
    }

    // find precise min near an_ir_valley_locs
    n_exact_ir_valley_locs_count = n_npks;

    //using exact_ir_valley_locs , find ir-red DC andir-red AC for SPO2 calibration an_ratio
    //  //finding AC/DC maximum of raw

    n_ratio_average = 0;
    n_i_ratio_count = 0;
    for (k = 0; k < 5; k++) {
        an_ratio[k] = 0;
    }
    for (k = 0; k < n_exact_ir_valley_locs_count; k++) {
        if (an_ir_valley_locs[k] > BUFFER_SIZE) {
            *pn_spo2 =  -999 ; // do not use SPO2 since valley loc is out of range
            *pch_spo2_valid  = 0;
            return;
        }
    }
    // find max between two valley locations
    //   // and use an_ratio betwen AC compoent of Ir & Red and DC compoent of Ir & Red for SPO2
    for (k = 0; k < n_exact_ir_valley_locs_count - 1; k++) {
        n_y_dc_max = -16777216 ;
        n_x_dc_max = -16777216;
        if (an_ir_valley_locs[k + 1] - an_ir_valley_locs[k] > 3) {
            for (i = an_ir_valley_locs[k]; i < an_ir_valley_locs[k + 1]; i++) {
                if (an_x[i] > n_x_dc_max) {
                    n_x_dc_max = an_x[i];
                    n_x_dc_max_idx = i;
                }
                if (an_y[i] > n_y_dc_max) {
                    n_y_dc_max = an_y[i];
                    n_y_dc_max_idx = i;
                }
            }
            n_y_ac = (an_y[an_ir_valley_locs[k + 1]] - an_y[an_ir_valley_locs[k] ]) * (n_y_dc_max_idx - an_ir_valley_locs[k]); //red
            n_y_ac =  an_y[an_ir_valley_locs[k]] + n_y_ac / (an_ir_valley_locs[k + 1] - an_ir_valley_locs[k])  ;
            n_y_ac =  an_y[n_y_dc_max_idx] - n_y_ac;   // subracting linear DC compoenents from raw
            n_x_ac = (an_x[an_ir_valley_locs[k + 1]] - an_x[an_ir_valley_locs[k] ]) * (n_x_dc_max_idx - an_ir_valley_locs[k]); // ir
            n_x_ac =  an_x[an_ir_valley_locs[k]] + n_x_ac / (an_ir_valley_locs[k + 1] - an_ir_valley_locs[k]);
            n_x_ac =  an_x[n_y_dc_max_idx] - n_x_ac;     // subracting linear DC compoenents from raw
            n_nume = (n_y_ac * n_x_dc_max) >> 7 ; //prepare X100 to preserve floating value
            n_denom = (n_x_ac * n_y_dc_max) >> 7;
            if (n_denom > 0  && n_i_ratio_count < 5 &&  n_nume != 0) {
                an_ratio[n_i_ratio_count] = (n_nume * 100) / n_denom ; //formular is ( n_y_ac *n_x_dc_max) / ( n_x_ac *n_y_dc_max) ;
                n_i_ratio_count++;
            }
        }
    }
    // choose median value since PPG signal may varies from beat to beat
    max301x_sort_ascend(an_ratio, n_i_ratio_count);
    n_middle_idx = n_i_ratio_count / 2;

    if (n_middle_idx > 1) {
        n_ratio_average = (an_ratio[n_middle_idx - 1] + an_ratio[n_middle_idx]) / 2;    // use median
    } else {
        n_ratio_average = an_ratio[n_middle_idx ];
    }

    if (n_ratio_average > 2 && n_ratio_average < 184) {
        n_spo2_calc = uch_spo2_table[n_ratio_average] ;
        *pn_spo2 = n_spo2_calc ;
        *pch_spo2_valid  = 1;//  float_SPO2 =  -45.060*n_ratio_average* n_ratio_average/10000 + 30.354 *n_ratio_average/100 + 94.845 ;  // for comparison with table
    } else {
        *pn_spo2 =  -999 ; // do not use SPO2 since signal an_ratio is out of range
        *pch_spo2_valid  = 0;
    }
}
#endif
int32_t hr_buf[2][16];
int32_t hrAvg[2];
int32_t hrBuffFilled[2];
int32_t hrValidCnt[2] = {0};
int32_t hrTimeout[2] = {0};
int32_t hrSum;
int32_t hrThrowOutSamp = 0;
//data_type:0:spo2;1:heart_rate
int32_t calculate_average(int pn_data, char  *pch_data_valid, u8 data_type)
{
    u8 i;
    if ((*pch_data_valid == 1) && (data_type ? ((pn_data < 170) && (pn_data > 40)) : (pn_data > 79)))
        /* if ((ch_spo2_valid == 1) && (n_spo2 > 59)) */
    {
        *pch_data_valid = 0;
        hrTimeout[data_type] = 0;
        if (hrValidCnt[data_type] == 4) {
            hrThrowOutSamp = 1;
            hrValidCnt[data_type] = 0;
            for (i = 12; i < 16; i++) {
                if (pn_data < hr_buf[data_type][i] + 10) {
                    hrThrowOutSamp = 0;
                    hrValidCnt[data_type]   = 4;
                }
            }
        } else {
            hrValidCnt[data_type] = hrValidCnt[data_type] + 1;
        }

        if (hrThrowOutSamp == 0) {
            *pch_data_valid = 1;
            for (i = 0; i < 15; i++) { // Shift New Sample into buffer
                hr_buf[data_type][i] = hr_buf[data_type][i + 1];
            }
            hr_buf[data_type][15] = pn_data;
            if (hrBuffFilled[data_type] < 16) { // Update buffer fill value
                hrBuffFilled[data_type] = hrBuffFilled[data_type] + 1;
            }
            // Take moving average
            hrSum = 0;
            if (hrBuffFilled[data_type] < 2) {
                hrAvg[data_type] = 0;
            } else if (hrBuffFilled[data_type] < 4) {
                for (i = 14; i < 16; i++) {
                    hrSum = hrSum + hr_buf[data_type][i];
                }
                hrAvg[data_type] = hrSum >> 1;
            } else if (hrBuffFilled[data_type] < 8) {
                for (i = 12; i < 16; i++) {
                    hrSum = hrSum + hr_buf[data_type][i];
                }
                hrAvg[data_type] = hrSum >> 2;
            } else if (hrBuffFilled[data_type] < 16) {
                for (i = 8; i < 16; i++) {
                    hrSum = hrSum + hr_buf[data_type][i];
                }
                hrAvg[data_type] = hrSum >> 3;
            } else {
                for (i = 0; i < 16; i++) {
                    hrSum = hrSum + hr_buf[data_type][i];
                }
                hrAvg[data_type] = hrSum >> 4;
            }
        }
        hrThrowOutSamp = 0;
    } else {
        *pch_data_valid = 0;
        hrValidCnt[data_type] = 0;
        if (hrTimeout[data_type] == 4) {
            hrAvg[data_type] = 0;
            hrBuffFilled[data_type] = 0;
        } else {
            hrTimeout[data_type]++;
        }
    }
    return hrAvg[data_type];
}

#endif
