#include "fm_opm_emu_driver.h"

static void fm_opm_emu_driver_write(struct fm_opm_driver *driver, uint8_t reg, uint8_t val) {
	struct fm_opm_emu_driver *emudrv = (struct fm_opm_emu_driver *)driver;

	ym2151_write_reg(&emudrv->opm, reg, val);
}

void fm_opm_emu_driver_init(struct fm_opm_emu_driver *driver, int sample_rate) {
	driver->sample_rate = sample_rate;
	ym2151_init(&driver->opm, 4000000, sample_rate);
	ym2151_reset_chip(&driver->opm);
	driver->fm_opm_driver.write = fm_opm_emu_driver_write;
	fm_opm_driver_init(&driver->fm_opm_driver);
}

void fm_opm_emu_driver_deinit(struct fm_opm_emu_driver *driver) {
	ym2151_shutdown(&driver->opm);
}

int fm_opm_emu_driver_estimate(struct fm_opm_emu_driver *d, int num_samples) {
	return num_samples;
}
// mdxCP/  to reduce memory & cpu ussage, mono sound only / Layer8
void fm_opm_emu_driver_run(struct fm_opm_emu_driver *d, stream_sample_t *outL, int num_samples) {
	ym2151_update_one(&d->opm, outL, num_samples);

}
