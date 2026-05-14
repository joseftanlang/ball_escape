#ifndef __BUZZER_MUSIC_H__
#define __BUZZER_MUSIC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// Single tone definition. Duration unit is 10 ms.
typedef struct {
	uint16_t frequency;
	uint8_t  duration;
} Tone_TypeDef;

typedef enum {
	BUZZER_SOUND_CLICK = 0,
	BUZZER_SOUND_BANG,
	BUZZER_SOUND_USB_CONNECTED,
	BUZZER_SOUND_USB_DISCONNECTED,
	BUZZER_SOUND_LETS_GO,
	BUZZER_SOUND_STARTUP,
	BUZZER_SOUND_3BEEP,
	BUZZER_SOUND_WELCOME,
	BUZZER_SOUND_GOODBYE,
	BUZZER_SOUND_HIGHSCORE,
	BUZZER_SOUND_LOWSCORE,
	BUZZER_SOUND_SUPER_MARIO,
	BUZZER_SOUND_JINGLE_BELLS,
	BUZZER_SOUND_MARIO_KART,
	BUZZER_SOUND_PIRATES_CARIBBEAN,
	BUZZER_SOUND_MAX,
	BUZZER_VIETNAM_NATIONAL_ANTHEM,
} buzzer_sound_t;

extern const Tone_TypeDef tones_click[];
extern const Tone_TypeDef tones_bang[];
extern const Tone_TypeDef tones_usb_connected[];
extern const Tone_TypeDef tones_usb_disconnected[];
extern const Tone_TypeDef tones_lets_go[];
extern const Tone_TypeDef tones_startup[];
extern const Tone_TypeDef tones_3beep[];
extern const Tone_TypeDef tones_welcome[];
extern const Tone_TypeDef tones_goodbye[];
extern const Tone_TypeDef tones_highscore[];
extern const Tone_TypeDef tones_lowscore[];
extern const Tone_TypeDef tones_smb[];
extern const Tone_TypeDef tones_jingle_bells[];
extern const Tone_TypeDef tones_mario_kart[];
extern const Tone_TypeDef tones_pirates[];
extern const Tone_TypeDef vietnam_national_anthem[];

#ifdef __cplusplus
}
#endif

#endif // __BUZZER_MUSIC_H__
