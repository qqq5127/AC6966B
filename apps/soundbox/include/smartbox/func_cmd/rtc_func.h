#ifndef __RTC_FUNC_H__
#define __RTC_FUNC_H__
#include "typedef.h"
#include "app_config.h"

u16 rtc_func_get_ex(void *priv, u8 *buf, u16 buf_size, u8 mask);
bool rtc_func_set_ex(void *priv, u8 *data, u16 len);
#endif
