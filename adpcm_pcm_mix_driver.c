#include <string.h>

#include "adpcm_pcm_mix_driver.h"
// mdxCP/  to reduce memory & cpu ussage, change resample method / Layer8
#include "mamedef.h"

uint16_t adpcm_mixer_calc_vol(uint8_t vol) {
	const uint8_t vol_00_0f[] = {
		0x6B, 0x6F, 0x71, 0x74, 0x76, 0x79, 0x7B, 0x7D,
		0x80, 0x82, 0x84, 0x87, 0x8A, 0x8C, 0x8F, 0x91,
	};
	const uint16_t vol_40_a0[] = {
		5, 6, 6, 7, 7, 8, 9, 10, 10, 11, 12, 14, 15, 16, 18, 20, 21,
		23, 25, 29, 31, 33, 37, 41, 46, 50, 54, 60, 66, 72, 80, 89,
		97, 107, 117, 130, 142, 156, 173, 189, 205, 226, 246, 267,
		308, 328, 369, 410, 431, 492, 533, 594, 656, 717, 799, 861,
		963, 1045, 1147, 1270, 1393, 1536, 1700, 1864, 2048, 2253,
		2479, 2724, 2991, 3298, 3625, 3994, 4383, 4834, 5325, 5837,
		6431, 7087, 7783, 8561, 9442, 10363, 11387, 12555, 13824,
		15217, 16733, 18371, 20255, 22221, 24454, 26932, 29696,
		32768, 36127, 39732, 43541
	};

	if(vol <= 15)
		return vol_40_a0[vol_00_0f[vol] - 0x40];
	if(vol >= 0x40 && vol <= 0xa0)
		return vol_40_a0[vol - 0x40];

	return 0;
}

int adpcm_mix_driver_channel_set_volume(struct adpcm_driver_channel *chan, uint8_t volume) {
	chan->volume = adpcm_mixer_calc_vol(volume);

	return 0;
}

int adpcm_mix_driver_channel_init(struct adpcm_driver_channel *channel, int chno) {
	channel->chdata = NULL;
	channel->data_len = 0;
  channel->chno = chno;
	adpcm_mix_driver_channel_set_volume(channel, 15);
	channel->data_pos = 0;
  channel->skip = channel->cnt= 0;
  channel->fin = 0;

	return 0;
}

void adpcm_mix_driver_channel_deinit(struct adpcm_driver_channel *channel) {
	channel->chdata = NULL;
	channel->data_len = 0;
	channel->data_pos = 0;
  channel->skip = channel->cnt= 0;
  channel->fin = 0;
}

int adpcm_mix_driver_channel_is_active(struct adpcm_driver_channel *channel) {
	return (channel->data_len != 0);
}

stream_sample_t adpcm_mix_driver_channel_get_sample(struct adpcm_driver_channel *channel, int k) {
	if(channel->data_len == 0 || channel->fin == 1){
		return 0;
  }
  // mdxCP/ quick & dirty hack for resample  / Layer8
	if(channel->cnt < channel->skip){
		channel->cnt += 320;
		return 0;
	}
	channel->cnt = 0;
	stream_sample_t sample = channel->chdata[channel->data_pos];
  if(channel->data_pos >= channel->data_len){
    channel->data_pos = 0;
    channel->fin = 1;
  }else{
	  channel->data_pos++;
  }
	sample = channel->volume * sample / 1024;
	if(sample > 32767) sample = 32767;
	if(sample < -32767) sample = -32767;
	return sample;
}

int adpcm_mix_driver_channel_play(struct adpcm_driver_channel *channel, short *data, int data_len, uint8_t freq_num, uint8_t volume, uint8_t slot) {
	channel->chdata = data;
	channel->data_len = data_len;
	channel->freq_num = freq_num;
	channel->skip = 44100 * 100 / (freqtbl[freq_num] + 1);
	channel->cnt = 0;
  channel->fin = 0;
	channel->slot = slot;
	adpcm_mix_driver_channel_set_volume(channel, volume);
	channel->data_pos = 0;
	return 0;
}

int adpcm_mix_driver_channel_stop(struct adpcm_driver_channel *chan) {
	chan->chdata = 0;
	chan->data_len = 0;
	chan->volume = 0;
	chan->data_pos = 0;
	chan->cnt = 0;
  chan->fin = 0;
	return 0;
}

int adpcm_mix_driver_channel_set_freq(struct adpcm_driver_channel *chan, uint8_t freq_num) {
	chan->freq_num = freq_num;
	return 0;
}
// mdxCP/  to reduce memory & cpu ussage, adpcm decode at begining / Layer8
int adpcm_pcm_mix_driver_alloc_buffers(struct adpcm_driver *driver, int buf_size) {
	return 0;
}

int adpcm_pcm_mix_driver_play(struct adpcm_driver *driver, uint8_t channel,  stream_sample_t *data, int data_len, uint8_t freq_num, uint8_t volume, uint8_t slot) {
	return adpcm_mix_driver_channel_play(&driver->channels[channel], data, data_len, freq_num, volume, slot);
}

int adpcm_pcm_mix_driver_stop(struct adpcm_driver *driver, uint8_t channel) {
	return adpcm_mix_driver_channel_stop(&driver->channels[channel]);
}

int adpcm_pcm_mix_driver_set_freq(struct adpcm_driver *driver, uint8_t channel, uint8_t freq) {
	return adpcm_mix_driver_channel_set_freq(&driver->channels[channel], freq);
}

int adpcm_pcm_mix_driver_set_volume(struct adpcm_driver *driver, uint8_t channel, uint8_t vol) {
	return adpcm_mix_driver_channel_set_volume(&driver->channels[channel], vol);
}
int adpcm_pcm_mix_driver_set_pan(struct adpcm_driver *driver, uint8_t pan) {
	driver->pan = pan & 0x03;
	return 0;
}

int adpcm_pcm_mix_driver_init(struct adpcm_driver *driver, int sample_rate, int buf_size) {
	adpcm_driver_init(driver);
	driver->play = adpcm_pcm_mix_driver_play;
	driver->stop = adpcm_pcm_mix_driver_stop;
	driver->set_freq = adpcm_pcm_mix_driver_set_freq;
	driver->set_volume = adpcm_pcm_mix_driver_set_volume;
	driver->set_pan = adpcm_pcm_mix_driver_set_pan;

	for(int i = 0; i < 8; i++) {
		adpcm_mix_driver_channel_init(&driver->channels[i], i);
	}
	return 0;
}

void adpcm_pcm_mix_driver_deinit(struct adpcm_driver *driver) {
	for(int i = 0; i < 8; i++) {
		adpcm_mix_driver_channel_deinit(&driver->channels[i]);
	}
}
int adpcm_pcm_mix_driver_run(struct adpcm_driver *driver, stream_sample_t *buf_l, int buf_size) {
  int j, k;
  struct adpcm_driver_channel *chanp;

	for(j = 0; j < 8; j++) {
    chanp = &(driver->channels[j]); 
    if(chanp->data_len == 0){
      continue;
    }
		for(k = 0; k < buf_size; k++) {
			buf_l[k] += adpcm_mix_driver_channel_get_sample(chanp, k);
	  }
	}
	return 0;
}
