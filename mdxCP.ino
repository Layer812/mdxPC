#include "M5Cardputer.h"
#include "SD.h"
#include "SPI.h"
#include "esp_task_wdt.h"

#define SD_SPI_SCK_PIN 40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN 12

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
extern "C" {
#include "tools.h"
#include "pdx.h"
#include "mdx.h"
#include "mdx_driver.h"
#include "pcm_timer_driver.h"
#include "adpcm_pcm_mix_driver.h"
#include "fm_opm_emu_driver.h"
#include "mamedef.h"
}

#include "filer.hpp"

// for filer.cpp

bool menuflag = false;

struct fl filelist[LISTMAX];
int filenum;
int dirnum = 0;
uint8_t cdir[PATHMAX] = "/";
uint8_t dirs[DIRMAX][PATHMAX] = {"/", "", "", "", ""};
int sel = 0, disp = 0;
int pcmvol;
#define PCM_SKIP_RATE 300
int pcmskip = PCM_SKIP_RATE;

bool playall;
bool playloop;
bool loopflag;

#define SAMPLING_RATE 44100
#define SAMPLING_BITS 16

#define BUFF_SIZE 2048
TaskHandle_t taskHandle;
xSemaphoreHandle xSemaphore = NULL;
int rd;
int wd;
#define WAV_BUFF_COUNT 12
int16_t wav_buff[WAV_BUFF_COUNT][BUFF_SIZE + 16];
int16_t wav_buff_size[WAV_BUFF_COUNT];

int16_t wav_count = 0;
bool inplay = false;
bool playend = false;

char titlestr[256];
char *gtitle;

int vol=64;

void play_task(void *args) {
  auto spk_cfg = M5.Speaker.config();
  bool r;
  int i;
//  esp_task_wdt_delete(NULL);
  M5.Speaker.begin();
  M5.Speaker.setVolume(vol);
//  M5.Speaker.tone(2000, 100);
  delay(1000);
  spk_cfg.sample_rate = 44100;
  spk_cfg.task_pinned_core = 0;
  M5.Speaker.config(spk_cfg);

  while (1) {
    while(M5.Speaker.isPlaying()){
      vTaskDelay(1);
    }
    while(wav_count == 0){
      xSemaphoreTake(xSemaphore, portMAX_DELAY);
    }
    xSemaphoreGive(xSemaphore);
    r = M5.Speaker.playRaw((const int16_t *)wav_buff[rd], wav_buff_size[rd], SAMPLING_RATE, false, 1);
    wav_buff_size[rd] = 0;
    rd = (rd + 1) % WAV_BUFF_COUNT;
    wav_count--;
  }
}

// TODO: check out of bounds conditions
int pdx_file_load(File pdx_fd, struct pdx_file *f, uint8_t *readbuff, int len, const esp_partition_t *part, int adr, char *wav) {
	if(len == 0) return -1;

  pdx_fd.seek(0, (SeekMode)0);
  pdx_fd.read(readbuff, 1024);

	for(int i = 0; i < 96; i++) {
		int ofs = (readbuff[i * 8] << 24) | (readbuff[i * 8 + 1] << 16) | (readbuff[i * 8 + 2] << 8) | readbuff[i * 8 + 3];
		int l = (readbuff[i * 8 + 4] << 24) | (readbuff[i * 8 + 5] << 16) | (readbuff[i * 8 + 6] << 8) | readbuff[i * 8 + 7];
		if(l > 0 && ofs < len && ofs + l <= len) {
			f->samples[i].ofs = ofs;
  		f->samples[i].len = l;
		} else {
			f->samples[i].ofs = 0;
			f->samples[i].len = 0;
		}
	}

	int16_t *sample_data = (int16_t *)malloc(4096+16);
	int seg = 0, k = 0, l = 0, m = 0;
	for(int i = 0; i < 96; i++) {
	  if(f->samples[i].len == 0) {
     continue;
    }
	  f->samples[i].decoded_data = (int16_t *)&(wav[seg * 4096]);
    if( f->samples[i].decoded_data == NULL){
      printf("malloc error! %d\n", i);
      free(sample_data);
      return(-1);
    }
  	f->samples[i].num_samples = f->samples[i].len * 2;
	  k = f->samples[i].len / 1024 + 1;
    struct adpcm_status st;
 	  adpcm_init(&st);	
	  for(l = 0; l < k; l++){
	    m = (l == (k - 1))? f->samples[i].len % 1024: 1024;
      pdx_fd.seek(f->samples[i].ofs + l * 1024, (SeekMode)0);
      pdx_fd.read(readbuff, 1024);
		  for(int j = 0; j < m; j++) {
			  uint8_t c = readbuff[j];
			  int16_t d1 = adpcm_decode(c & 0x0f, &st);
			  int16_t d2 = adpcm_decode(c >> 4, &st);
		  	sample_data[j * 2] = d1;
			  sample_data[j * 2 + 1] = d2;    
		  }
  	  if (esp_partition_erase_range(part, adr + 4096 * seg  , 4096) != ESP_OK) {
        Serial.printf("Couldn't erase range!\n");
      }
      if(esp_partition_write(part, adr + 4096 * seg, sample_data, 4096) != ESP_OK) {
        Serial.printf("Couldn't erase write!\n");
      }
      seg++;       
	  }
	}
	free(sample_data);
	return 0;
}

int opt_channel_mask = 0xffff;
int opt_loops = 2;
char *opt_output = 0;
int opt_sample_rate = 44100;
int opt_fadeout_rate = 1;

// stream_sample_t bufL[BUFF_SIZE], bufR[BUFF_SIZE];
struct mdx_driver mdx_driver;

struct pcm_timer_driver timer_driver;
struct adpcm_driver adpcm_driver;
struct fm_opm_emu_driver fm_driver;

// stream_sample_t mixBufL[BUFF_SIZE], mixBufR[BUFF_SIZE];
struct mdx_file mdx_fst;
struct pdx_file pdx_fst;
size_t mdx_file_size;

struct stereo_str{
  int r;
  int l;
};

char dirbuf[256];
char *dirname(char *path){
        char *rp;
        strncpy(dirbuf, path, 256);
        if(rp = strrchr(dirbuf, '/')){
                rp[1] = 0;
                return dirbuf;
        }
        return NULL;
}

int find_pdx_file(fs::FS &fs, char *mdx_file_path, const char *pdx_filename, char *out, int out_len) {
	char *dn = dirname(mdx_file_path);
	char buf[1024];
	*out = 0;
	File fd;

	for(int i = 0; i < 2; i++) {
		if(i == 0) {
			snprintf(buf, 256, "%s%s", dn, pdx_filename);
		} else {
			snprintf(buf, 256, "%s%s.PDX", dn, pdx_filename);
		}
		int r = fs.exists(buf);
		if(r) {
			strncpy(out, buf, out_len);
			return 1;
		}

	}
	return 0;
}

#define READBUFF_SIZE 4096

struct mdxhdr{
  int mdxlen;
  char mdxpath[PATH_MAX];
  int pdxlen;
  char pdxpath[PATH_MAX];
};


struct mdxhdr mhtmp;
const esp_partition_t *part;
spi_flash_mmap_handle_t hrom;
char *fdata;

bool load_mdx(fs::FS &fs, const char *mdxpath) {
  File mdx_fd, pdx_fd;
  char *ptr, pdxpath[PATH_MAX], *pdxdata, *mdxdata;
  int mdxlen, pdxlen, cnt = 0, i, j;
  int err;
  struct mdxhdr *mh;

  bool pdxexist = false, pdxload = false, mdxload = false;

  uint8_t *readbuff;
  readbuff = (uint8_t *)malloc(READBUFF_SIZE + 16);
  memset(&mdx_fst, 0, sizeof(struct mdx_file));
  memset(&pdx_fst, 0, sizeof(struct pdx_file));

  #define MDX_LEN  0x20000
  mh = (struct mdxhdr *)fdata;
  mdx_fd = fs.open(mdxpath);
  if (!mdx_fd){
    free(readbuff);
    return true;
  }
  mdxlen = mdx_fd.size();
  if (mdxlen == 0 || mdxlen >= MDX_LEN){
    free(readbuff);
    return true;
  }
  mdx_fd.read(readbuff, READBUFF_SIZE);
  ptr = mdx_file_get_pdxname(readbuff, READBUFF_SIZE);
  pdxexist = (ptr[0] != 0);
  if(pdxexist){
    strncpy((char *)pdxpath, (const char *)ptr, PATH_MAX);
  	find_pdx_file(SD, (char *)mdxpath, (const char *)pdxpath, (char *)readbuff, READBUFF_SIZE);
    strncpy((char *)pdxpath, (const char *)readbuff, PATH_MAX);
  	if(!pdxpath[0]){
      pdxexist = false;
      pdxload = false;
    }else{
     pdxexist = true;
     pdx_fd = fs.open(pdxpath);
     if (!pdx_fd){
        free(readbuff);
        return true;
     }
     pdxlen = pdx_fd.size();
     pdxload = !(pdxlen == mh->pdxlen && !strcmp(pdxpath, mh->pdxpath));
    }
  }
  mdxload = !(mdxlen == mh->mdxlen && !strcmp(mdxpath, mh->mdxpath));
  mdxload = pdxload = true; // avandon bug...

  if(mdxload || pdxload){
    mhtmp.pdxlen = pdxlen;
    mhtmp.mdxlen = mdxlen;
    strncpy(mhtmp.pdxpath, pdxpath, PATH_MAX);
    strncpy(mhtmp.mdxpath, mdxpath, PATH_MAX);
    if (esp_partition_erase_range(part, 0, READBUFF_SIZE) != ESP_OK) {
      Serial.printf("Couldn't erase range!\n");
    }
    if (esp_partition_write(part, 0, &mhtmp, READBUFF_SIZE) != ESP_OK) {
      Serial.printf("Couldn't erase write!\n");
    }
  }

#if 0
  if(mdxload){
    int ml;
    cnt = 0;
    ml = mdxlen;
    while(ml > 0){ 
      mdx_fd.seek(READBUFF_SIZE * cnt, (SeekMode)0);
      mdx_fd.read(readbuff, READBUFF_SIZE);
      if (esp_partition_erase_range(part, READBUFF_SIZE * (cnt + 1), READBUFF_SIZE) != ESP_OK) {
        Serial.printf("Couldn't erase range!\n");
      }
      if (esp_partition_write(part, READBUFF_SIZE * (cnt + 1), readbuff, READBUFF_SIZE) != ESP_OK) {
        Serial.printf("Couldn't erase write!\n");
      }
      cnt++;
      ml -= READBUFF_SIZE;
    }
  }
  mdxdata = &(fdata[READBUFF_SIZE]);
#endif

  mdxdata = (char *)malloc(mdxlen);
  if(mdxdata == NULL){
    printf("Can't alloc %d of memory / Free memory: %d\n", mdxlen, esp_get_free_heap_size());
    free(readbuff);
    return true;
  }
  mdx_fd.seek(0, (SeekMode)0);
  mdx_fd.read((uint8_t *)mdxdata, mdxlen);
	mdx_file_load(&mdx_fst, (uint8_t *)mdxdata, mdxlen);  
  if(pdxexist)
   	pdx_file_load(pdx_fd, &pdx_fst, readbuff, pdxlen, part, MDX_LEN, &(fdata[MDX_LEN])); // todo rewrite seg 
	mdx_driver_load(&mdx_driver, &mdx_fst, &pdx_fst);
  strncpy(titlestr, (const char *)mdx_fst.title, 255);
  gtitle = titlestr;
  pdx_fd.close();
  mdx_fd.close();
  free(readbuff);
  return false;
}

void initall(){
  int i;
  rd = wd = wav_count = 0;
  playend = false;
  for(i = 0; i < WAV_BUFF_COUNT; i++){
    memset(wav_buff[i], 0, BUFF_SIZE * sizeof(int16_t));
    wav_buff_size[i] = 0;
  }
}

SET_LOOP_TASK_STACK_SIZE(8 * 1024);

void setup(){
  int err;
//  disableLoopWDT();
  auto cfg = M5.config();
  part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_PHY, NULL);
  if (part == 0)
    Serial.printf("Couldn't find vgm part!\n");
  err = esp_partition_mmap(part, 0, 0x300000, SPI_FLASH_MMAP_DATA, (const void **)&fdata, &hrom);
  if (err != ESP_OK)
    Serial.printf("Couldn't map vgm part!\n");

  M5Cardputer.begin(cfg);
  int textsize = M5Cardputer.Display.height() / 60;
  if (textsize == 0) {
    textsize = 1;
  }
  M5Cardputer.Display.setTextSize(textsize); 

  xSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xSemaphore);

  xTaskCreatePinnedToCore(play_task, "play_task", 4096, NULL, 1, &taskHandle, 0);
  delay(500);
}

uint8_t fname[PATHMAX] ;
void loop(){
  int i, n = 0, j = 0, q;
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    while (1)
      delay(1);
  }
  gtitle = NULL;
  mdx_driver.max_loops = 2;

  while(1){ 
    pcm_timer_driver_init(&timer_driver, opt_sample_rate);
    adpcm_pcm_mix_driver_init(&adpcm_driver, opt_sample_rate, 0);
    fm_opm_emu_driver_init(&fm_driver, opt_sample_rate);
    mdx_driver_init(
        &mdx_driver,
        (struct timer_driver *)&timer_driver,
        (struct fm_driver *)&fm_driver,
        (struct adpcm_driver *)&adpcm_driver
    );
		initall();
		 
	  disptitle(STATCLR, gtitle);
    filenum = makemdxlist(SD);
  	i = selectfile();
    disptitle(STATLOAD, gtitle);
	  if(!cnvfile(SD, &filelist[i], fname)) //chance
  	  continue;
    if(load_mdx(SD, (const char *)fname))
      continue;
	  if(playall)
	    disptitle(STATALL, gtitle);
	  else if(playloop)
	    disptitle(STATLOOP, gtitle);
	  else
	    disptitle(STATCLR, gtitle); 
    printf("[APP] Free memory: %d\n", esp_get_free_heap_size());
	  inplay = true;
		while(!mdx_driver.ended) {
      hitkey();
      if(playend)
        break;
			int samples_remaining = BUFF_SIZE;
			int pos;
			wd = (wd + 1) % WAV_BUFF_COUNT;
  		memset(wav_buff[wd], 0, BUFF_SIZE * sizeof(short));
      wav_buff_size[wd] = 0;
	    pos = 0;
			while(samples_remaining > 0) {
			  int timer_samples = pcm_timer_driver_estimate(&timer_driver, samples_remaining);		
				int fm_samples = fm_opm_emu_driver_estimate(&fm_driver, samples_remaining);
	  		int samples = timer_samples;
				if(fm_samples < samples)
					samples = fm_samples;
				adpcm_pcm_mix_driver_run(&adpcm_driver, (stream_sample_t *)&(wav_buff[wd][pos]), samples);
				fm_opm_emu_driver_run(&fm_driver,(stream_sample_t *)&(wav_buff[wd][pos]), samples);
				pcm_timer_driver_advance(&timer_driver, samples);

				samples_remaining -= samples;
        pos += samples;
			}
	    wav_count++;
	    wav_buff_size[wd] = BUFF_SIZE;
	    do{
        vTaskDelay(1);
	      xSemaphoreGive(xSemaphore);
	    } while (wav_count == (WAV_BUFF_COUNT - 2));
    }
    inplay = false;
    delay(1000);
    gtitle = NULL;
    fm_opm_emu_driver_deinit(&fm_driver);
    pcm_timer_driver_deinit(&timer_driver);
    adpcm_pcm_mix_driver_deinit(&adpcm_driver);
    mdx_driver_free(&mdx_driver);
	}
}
