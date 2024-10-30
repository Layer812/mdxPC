#ifndef ADPCM_DRIVER_H_
#define ADPCM_DRIVER_H_

#include <stdint.h>

// mdxCP/  to reduce memory & cpu ussage, change resample method / Layer8
#include "adpcm.h"
#include "mamedef.h"

struct adpcm_driver_channel {
	// 0 = 3.9kHz
	// 1 = 5.2kHz
	// 2 = 7.8kHz
	// 3 = 10.4kHz
	// 4 = 15.6kHz
	int freq_num;
	int skip;
	int cnt;
  int fin;

	int volume;

	stream_sample_t *chdata;

	int data_len;
	int data_pos;

  int chno;
  uint8_t slot;
};

struct adpcm_driver {
	uint8_t pan;

	int (*play)(struct adpcm_driver *d, uint8_t channel, short *data, int len, uint8_t freq, uint8_t vol, uint8_t slot);
	int (*stop)(struct adpcm_driver *d, uint8_t channel);
	int (*set_volume)(struct adpcm_driver *d, uint8_t channel, uint8_t vol);
	int (*set_freq)(struct adpcm_driver *d, uint8_t channel, uint8_t freq);
	int (*set_pan)(struct adpcm_driver *d, uint8_t pan);

	struct adpcm_driver_channel channels[8];
	struct adpcm_status encoder_status;
};

void adpcm_driver_init(struct adpcm_driver *driver);
int adpcm_driver_play(struct adpcm_driver *d, uint8_t channel, short *data, int len, uint8_t freq, uint8_t vol, uint8_t slot);

int adpcm_driver_stop(struct adpcm_driver *d, uint8_t channel);
int adpcm_driver_set_freq(struct adpcm_driver *d, uint8_t channel, uint8_t freq);
int adpcm_driver_set_volume(struct adpcm_driver *d, uint8_t channel, uint8_t vol);
int adpcm_driver_set_pan(struct adpcm_driver *d, uint8_t pan);

#endif /* ADPCM_DRIVER_H_ */
