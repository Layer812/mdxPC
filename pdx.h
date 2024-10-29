#ifndef PDX_H_
#define PDX_H_

#include <stdint.h>

#define PDX_NUM_SAMPLES 96

struct pdx_sample {
// mdxCP/ pdx file load on flash / Layer8
	int ofs;
	int len;
	int16_t *decoded_data;
	int num_samples;
};

struct pdx_file {
	struct pdx_sample samples[PDX_NUM_SAMPLES];
	int num_samples;
};

// int pdx_file_load(struct pdx_file *pdx, uint8_t *data, int data_len);

#endif /* PDX_H_ */
