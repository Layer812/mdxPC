#include <M5Cardputer.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>
#include <ESP32-targz.h>
#include "filer.hpp"
#include "misakiSJIS.h"

extern int vol;
extern bool loopflag;
extern bool menuflag;
extern bool playall;
extern bool playend;
extern char *gtitle;
extern int sel;
extern int disp;
extern int pcmvol;


extern struct fl filelist[];
extern int filenum;
extern uint8_t cdir[];
extern uint8_t dirs[][PATHMAX];
extern int dirnum;

const char statstr[STATNUM][STATMAX] = {"         ", "loading ", "play loop", "play all "}; 
// ビットパターン表示
// d: 8ビットパターンデータ
//
void bitdisp(byte x, byte y, uint8_t d ) {
  for (byte i=0; i<8;i++) {
    if (d & 0x80>>i) {
      if(x + i < 232 && y < 64){
          M5.Display.drawPixel(x + i ,y , WHITE);
      }
    }      
  }
}
// フォントパターンを画面に表示する
// 引数
// x,y 表示位置
//  pUTF8 表示する文字列
// ※半角文字は全角文字に置き換えを行う
//
void drawJPChar(byte x, byte y, char *data) {
  byte buf[60][8];  //160x8ドットのバナー表示パターン
  int n = 0;
  // バナー用パターン作成
  for (byte i=0; i < 60 && data[i] != 0; i++) {
    data = getFontData(&buf[i][0], data, false);  // フォントデータの取得    
    n++;
  }
  // ドット表示
  for (byte i=0; i < 8; i++) {
    for (byte j=0; j < n; j++){
        bitdisp(x + (j * 8) ,y + i , buf[j][i]);
    }
  }
}

void disptitle(int stat, char *title){
  M5Cardputer.Display.setCursor(0, 0);
  M5Cardputer.Display.fillRect(0, 0, 240, 25, BLACK);
  M5Cardputer.Display.setTextColor(OLIVE);
  M5Cardputer.Display.printf("mdxPC %.1f ", VERSION);
  M5Cardputer.Display.setTextColor(GREEN);
  M5Cardputer.Display.printf("%s", statstr[stat % STATNUM]);
  M5Cardputer.Display.drawLine(0,27,240,27,OLIVE);
  if(title){
    drawJPChar(0,18,title);
//    printf("ti: %x %s\n", title, title);
  }
}

void dispmenu(){
  M5Cardputer.Lcd.fillRect(20,20,200,100,BLUE);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 22);
  M5.Lcd.println("   m: menu");
  M5.Lcd.println("   ");
  M5.Lcd.println("   a: play all");
  M5.Lcd.println("   +-: maser vol");
  M5.Lcd.println("   90: pcm vol");
  M5.Lcd.println("   \" \": play/stop");
}



void hitkey(){
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isKeyPressed(' ')){
      playend = true;
      playall = false;
     // printf("space pressed \n");
    }
    if (M5Cardputer.Keyboard.isKeyPressed('=')){
      vol += 20;
      if(vol > 255)
        vol = 255;
     // printf("plus pressed \n");
      M5.Speaker.setVolume(vol);
    }
    if (M5Cardputer.Keyboard.isKeyPressed('-')){
      vol -= 20;
      if(vol < 0)
        vol = 0;
      //printf("minus pressed \n");
      M5.Speaker.setVolume(vol);
    }
    if (M5Cardputer.Keyboard.isKeyPressed('9')){
      pcmvol -= 20;
      if(vol < -255)
        vol = -255;
    }
    if (M5Cardputer.Keyboard.isKeyPressed('0')){
      pcmvol += 20;
      if(pcmvol > 255)
        pcmvol = 255;
    }
    if (M5Cardputer.Keyboard.isKeyPressed('a')){
      playall = !playall;
      loopflag = false;
      disptitle(playall? STATALL: STATCLR, gtitle);
      //printf("a pressed \n");
    }
    if (M5Cardputer.Keyboard.isKeyPressed('m')){
      menuflag = !menuflag;
      if(menuflag)
        dispmenu();
      //printf("m pressed \n");
    }
    if (M5Cardputer.Keyboard.isKeyPressed('l')){
      loopflag = !loopflag;
      //printf("p pressed \n");
    }
  }
}

bool ismdxfile(const char *fileName) {
  int i; char exttmp[5];
  exttmp[4] = 0;
  const char *ext = strrchr(fileName, '.');
  if(ext == NULL)
    return false;
  for(i = 0; i < 4; i++)
    exttmp[i] = tolower(ext[i]);

  return (strcmp(".mdx", exttmp) == 0);
}

int makemdxlist(fs::FS &fs) {
  int i = 0;
 // printf("ck1\n");
   //printf("ck6 %s\n", cdir);
  File root = fs.open((const char *)cdir);
  if (!root.isDirectory()) {
    return 0;
  }
  //printf("ck7 %d\n", dirnum);
  memset(filelist, 0, sizeof(struct fl) * LISTMAX);
  if(dirnum > 0){
    strcpy((char *)filelist[0].filename, (char *)dirs[dirnum - 1]);
    filelist[0].type = TYPE_UDIR;
    i++;
  }
  File file = root.openNextFile();
  while (file) {
 //   printf("ck8 %s\n", file.name());
    if (i == LISTMAX)
      break;
//    sprintf((char *)filelist[i].filename, "/%.*s", PATHMAX - 2, file.name());
//    printf("ck4 %d %s\n", i, filelist[i]);
    if (ismdxfile((const char *)file.name())){
      memcpy(filelist[i].filename, file.name(), PATHMAX);
      filelist[i].type = TYPE_MDX;
      i++;
    }
    if(file.isDirectory()){
      memcpy(filelist[i].filename, file.name(), PATHMAX);
      filelist[i].type = TYPE_SDIR;
      i++;
    }
//    printf("ck5 %d %s\n", i, filelist[i]);
    file = root.openNextFile();
//    printf("ck6\n");
  }
  return i;
}

void dispfiles(int disp, int sel, bool start){
  int i;
  menuflag = false;
  M5Cardputer.Display.fillRect(0,28, 240,135, BLACK);
  M5Cardputer.Display.setCursor(0, 28);

//  M5Cardputer.Display.fillScreen(BLACK);
  for (i = 0; i < DISPMAX; i++){
    if(filelist[i + disp].type == TYPE_SDIR || filelist[i + disp].type == TYPE_UDIR)
      M5Cardputer.Display.setTextColor(LIGHTGREY);  
    if((i + disp)== sel)
      M5Cardputer.Display.setTextColor(BLACK, start? YELLOW: GREEN);
    M5Cardputer.Display.printf("%.*s\n", 20, filelist[i + disp].filename);
    M5Cardputer.Display.setTextColor(WHITE);
  }
}


int selectfile(){
    dispfiles(disp, sel, false);
    if(playall){
      if(sel < (filenum - 1)){
        sel++;
        disp++;
        dispfiles(disp, sel, true);
        return sel;
      }else{
        playall = false;
        sel = disp = 0;
        dispfiles(disp, sel, false);
      }
    }
    while(1){
      M5Cardputer.update();
      if (M5Cardputer.Keyboard.isChange()) {
          if (M5Cardputer.Keyboard.isKeyPressed(';')){
            if(sel != 0){
              if(sel == disp)
                disp--;
              sel--;
            }
            dispfiles(disp, sel, false);
          }
          if (M5Cardputer.Keyboard.isKeyPressed('.')){
            if(sel < (DISPMAX - 1)){
              sel++;
            }else{
              if(sel < (filenum - 1)){
                sel++;
                disp++;
              }
            }
            dispfiles(disp, sel, false);
          }
          if (M5Cardputer.Keyboard.isKeyPressed(',')){
            if(sel - DISPMAX > 0){
              sel -= DISPMAX;
              disp = sel;
            }else{
              sel = disp = 0;
            }
            dispfiles(disp, sel, false);
          }
          if (M5Cardputer.Keyboard.isKeyPressed('/')){
             if(sel < filenum - DISPMAX){
              sel += DISPMAX;
              disp += DISPMAX;
            }
            dispfiles(disp, sel, false);
          }
          if (M5Cardputer.Keyboard.isKeyPressed(' ')){
            dispfiles(disp, sel, true);
            return sel;
          }
          if (M5Cardputer.Keyboard.isKeyPressed('=')){
            vol += 20;
            if(vol > 255)
              vol = 255;
            M5.Speaker.setVolume(vol);
          }
          if (M5Cardputer.Keyboard.isKeyPressed('-')){
            vol -= 20;
            if(vol < 0)
              vol = 0;
            M5.Speaker.setVolume(vol);
          }       
          if (M5Cardputer.Keyboard.isKeyPressed('m')){
            dispmenu();
            //printf("m pressed \n");
          }
          if (M5Cardputer.Keyboard.isKeyPressed('a')){
            if(filelist[sel].type != TYPE_MDX)
              continue;
            playall = true;
            loopflag = false;
            disptitle(playall? STATALL: STATCLR, gtitle);
            return sel;
          }
      }
    }
}

bool cnvfile(fs::FS &fs, struct fl *srct, uint8_t *dst){
  bool ret = true;
  //printf("ck4 %d\n", srct->type);
  switch(srct->type){
    case TYPE_MDX:
      sprintf((char *)dst, "%s%s%s", cdir, (dirnum > 0)?"/":"",(char *)srct->filename);
//      printf("chk spri1: %s, %d\n", (char *)dst, strlen((char *)dst));
      break;
    case TYPE_SDIR:
      if(dirnum == DIRMAX-1)
        break;
      dirnum++;   
      sprintf((char *)dirs[dirnum], "%s%s%s", (char *)cdir, (dirnum > 1)?"/":"",(char *)srct->filename);
//      printf("chk spri3: %s, %d, %d\n", (char *)dirs[dirnum], strlen((char *)dirs[dirnum]), dirnum);
      strcpy((char *)cdir, (char *)dirs[dirnum]);
      sel = disp = 0;
      ret = false;
      break;
    case TYPE_UDIR:
      dirnum--;   
      strcpy((char *)cdir, (char *)dirs[dirnum]);
//      printf("chk spri4: %s, %d, %d\n", (char *)dirs[dirnum], strlen((char *)dirs[dirnum]), dirnum);
      sel = disp = 0;
      ret = false;
      break;
  }
//  printf("ck5 %s %s %d\n", cdir, dst, ret);
  return ret;
}

void filerinit(){

}