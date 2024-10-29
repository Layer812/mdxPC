#ifndef ADPCM_PCM_MIX_DRIVER_H_
#define ADPCM_PCM_MIX_DRIVER_H_

#include "adpcm_driver.h"
// mdxCP/  to reduce memory & cpu ussage, change resample method / Layer8



const int freqtbl[5] = {3900, 5200, 7800, 10400, 15600};

int adpcm_pcm_mix_driver_init(struct adpcm_driver *driver, int sample_rate, int buf_size);
void adpcm_pcm_mix_driver_deinit(struct adpcm_driver *driver);
int adpcm_pcm_mix_driver_estimate(struct adpcm_driver *driver, int buf_size);
int adpcm_pcm_mix_driver_run(struct adpcm_driver *driver, stream_sample_t *buf_l, int buf_size);

#endif /* ADPCM_PCM_MIX_DRIVER_H_ */
