#ifndef __IIS_H__
#define __IIS_H__

#include "generic/typedef.h"

#define ALNK_CON_RESET()	do {JL_ALNK0->CON0 = 0; JL_ALNK0->CON1 = 0; JL_ALNK0->CON2 = 0; JL_ALNK0->CON3 = 0;} while(0)

#define ALINK_DSPE(x)		SFR(JL_ALNK0->CON0, 6, 1, x)
#define ALINK_SOE(x)		SFR(JL_ALNK0->CON0, 7, 1, x)
#define ALINK_MOE(x)		SFR(JL_ALNK0->CON0, 8, 1, x)
#define F32_EN(x)           SFR(JL_ALNK0->CON0, 9, 1, x)
#define SCLKINV(x)         	SFR(JL_ALNK0->CON0,10, 1, x)
#define ALINK_EN(x)         SFR(JL_ALNK0->CON0,11, 1, x)
#define ALINK_24BIT_MODE()	(JL_ALNK0->CON1 |= (BIT(2) | BIT(6) | BIT(10) | BIT(14)))
#define ALINK_16BIT_MODE()	(JL_ALNK0->CON1 &= (~(BIT(2) | BIT(6) | BIT(10) | BIT(14))))

#define ALINK_IIS_MODE()	(JL_ALNK0->CON1 |= (BIT(2) | BIT(6) | BIT(10) | BIT(14)))

#define ALINK_CHx_DIR_TX_MODE(ch)	(JL_ALNK0->CON1 &= (~(1 << (3 + 4 * ch))))
#define ALINK_CHx_DIR_RX_MODE(ch)	(JL_ALNK0->CON1 |= (1 << (3 + 4 * ch)))

#define ALINK_CHx_MODE_SEL(val, ch)		(JL_ALNK0->CON1 |= (val << (0 + 4 * ch)))
#define ALINK_CHx_CLOSE(val, ch)		(JL_ALNK0->CON1 &= ~(val << (0 + 4 * ch)))

#define ALINK_CLR_CH0_PND()		(JL_ALNK0->CON2 |= BIT(0))
#define ALINK_CLR_CH1_PND()		(JL_ALNK0->CON2 |= BIT(1))
#define ALINK_CLR_CH2_PND()		(JL_ALNK0->CON2 |= BIT(2))
#define ALINK_CLR_CH3_PND()		(JL_ALNK0->CON2 |= BIT(3))

#define ALINK_MSRC(x)		SFR(JL_ALNK0->CON3, 0, 2, x)
#define ALINK_MDIV(x)		SFR(JL_ALNK0->CON3, 2, 3, x)
#define ALINK_LRDIV(x)		SFR(JL_ALNK0->CON3, 5, 3, x)

#endif
