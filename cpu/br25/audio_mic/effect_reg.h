#ifndef __EFFECT_REG_H__
#define __EFFECT_REG_H__

#include "system/includes.h"
#include "audio_mic/mic_stream.h"
// #include "pitchshifter/pitchshifter_api.h"
#include "effect_cfg.h"
#include "application/audio_eq.h"
#include "effect_reg_echo.h"
#include "effect_reg_reverb.h"
#include "application/audio_howling.h"
#include "application/audio_pitch.h"
#include "application/audio_noisegate.h"
#include "application/audio_echo_reverb.h"
#include "application/audio_dig_vol.h"
#include "application/audio_energy_detect.h"

struct __mic_effect_parm {
    u32	 effect_config;
    u32	 effect_run;
    u16	 sample_rate;

};


extern const struct __mic_effect_parm 				effect_parm_default;
extern const struct __mic_stream_parm 				effect_mic_stream_parm_default;
extern const REVERBN_PARM_SET 						effect_reverb_parm_default;
extern const EF_REVERB_FIX_PARM 					effect_echo_fix_parm_default;
extern const ECHO_PARM_SET 							effect_echo_parm_default;
extern const PITCH_SHIFT_PARM	 					effect_pitch_parm_default;
extern const NOISEGATE_PARM 						effect_noisegate_parm_default;
extern audio_dig_vol_param 		 			effect_dvol_default_parm;
extern const SHOUT_WHEAT_PARM_SET 					effect_shout_wheat_default;
extern const LOW_SOUND_PARM_SET 					effect_low_sound_default;
extern const HIGH_SOUND_PARM_SET 					effect_high_sound_default;
extern struct __effect_mode_cfg 				effect_tool_parm;
extern       audio_energy_detect_param	    		 mic_energy_det_param;
#endif// __EFFECT_REG_H__

