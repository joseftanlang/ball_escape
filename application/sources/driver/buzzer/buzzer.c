#include <buzzer.h>

void buzzer_irq( void ) {
}

// Initialize buzzer output
void BUZZER_Init(void) {
}

// Turn on buzzer with specified frequency
// input:
//   freq - PWM frequency for buzzer (Hz)
//   duration - duration of buzzer work (tens ms: 1 -> 10ms sound duration)
void BUZZER_Enable(uint16_t freq, uint32_t duration) {
	(void)freq;
	(void)duration;
}

// Turn off buzzer
void BUZZER_Disable(void) {
}

// Start playing tones sequence
// input:
//   tones - pointer to tones array
void BUZZER_PlayTones(const Tone_TypeDef * tones) {
	(void)tones;
}
