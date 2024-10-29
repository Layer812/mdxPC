// 
// 美咲フォントドライバ v1.1 by たま吉さん 2018/02/12
// 内部フラッシュメモリバージョン
// 
// 修正 2018/02/12,Arduino Uno(AVR)でのchar符号付き挙動の弊害対応
// 修正 2019/07/16, 半角全角変換テーブルのミス修正
// 修正 2019/07/16, １フォントあたり7バイトに圧縮
//

#include <Arduino.h>
#include "misakiSJIS.h"
#include "misakiSJISFontData.h"

// 半角全角変換テーブル
PROGMEM static const uint16_t zentable[] = {
0x8140,0x8149,0x8168,0x8194,0x8190,0x8193,0x8195,0x8166,
0x8169,0x816A,0x8196,0x817B,0x8143,0x817C,0x8144,0x815E,
0x824F,0x8250,0x8251,0x8252,0x8253,0x8254,0x8255,0x8256,
0x8257,0x8258,0x8146,0x8147,0x8183,0x8181,0x8184,0x8148,
0x8197,0x8260,0x8261,0x8262,0x8263,0x8264,0x8265,0x8266,
0x8267,0x8268,0x8269,0x826A,0x826B,0x826C,0x826D,0x826E,
0x826F,0x8270,0x8271,0x8272,0x8273,0x8274,0x8275,0x8276,
0x8277,0x8278,0x8279,0x816D,0x818F,0x816E,0x814F,0x8151,
0x8165,0x8281,0x8282,0x8283,0x8284,0x8285,0x8286,0x8287,
0x8288,0x8289,0x828A,0x828B,0x828C,0x828D,0x828E,0x828F,
0x8290,0x8291,0x8292,0x8293,0x8294,0x8295,0x8296,0x8297,
0x8298,0x8299,0x829A,0x816F,0x8162,0x8170,0x8160,0x8142,
0x8175,0x8176,0x8141,0x8145,0x8392,0x8340,0x8342,0x8344,
0x8346,0x8348,0x8383,0x8385,0x8387,0x8362,0x815B,0x8341,
0x8343,0x8345,0x8347,0x8349,0x834A,0x834C,0x834E,0x8350,
0x8352,0x8354,0x8356,0x8358,0x835A,0x835C,0x835E,0x8360,
0x8363,0x8365,0x8367,0x8369,0x836A,0x836B,0x836C,0x836D,
0x836E,0x8371,0x8374,0x8377,0x837A,0x837D,0x837E,0x8380,
0x8381,0x8382,0x8384,0x8386,0x8388,0x8389,0x838A,0x838B,
0x838C,0x838D,0x838F,0x8393,0x814A,0x814B,};

// 全角判定
inline uint8_t isZenkaku(uint8_t c){
   return (((c>=0x81)&&(c<=0x9f))||((c>=0xe0)&&(c<=0xfc))) ? 1:0;
}

// nバイト読込
// rcvdata: 読込データ格納アドレス
// n      : 読込みデータ数
// 戻り値  : 読み込んだバイト数
//
byte Sequential_read(uint32_t address, byte* rcvdata, byte n)  {
  for (uint16_t i = 0; i < n ; i++) 
    rcvdata[i] = pgm_read_byte(address + fdata + i);
 return n;
}

// フォントコード検索
// (コードでテーブルを参照し、フォントコードを取得する)
// sjiscode(in) SJISコード
// 戻り値    該当フォントがある場合 フォントコード(0-FTABLESIZE)
//           該当フォントが無い場合 -1

int16_t findcode(uint16_t sjiscode)  {
 int16_t  t_p = 0;            //　検索範囲上限
 int16_t  e_p = FTABLESIZE-1; //  検索範囲下限
 int16_t  pos;
 uint16_t d = 0;
 uint8_t  flg_stop = 0;
 
 while(true) {
   pos = t_p + ((e_p - t_p+1)>>1);
   d = (uint16_t)pgm_read_word(ftable+pos);
   if (d == sjiscode) {
     // 等しい
     flg_stop = 1;
     break;
   } else if (sjiscode > d) {
     // 大きい
     t_p = pos + 1;
     if (t_p > e_p) {
       break;
     }
   } else {
     // 小さい
    e_p = pos -1;
    if (e_p < t_p) 
      break;
   }
 } 

 if (!flg_stop) {
    return -1;
 }
 return pos;    
}

//
// SJISに対応する美咲フォントデータ8バイトを取得する
//   data(out): フォントデータ格納アドレス
//   sjis(in): SJISコード
//   戻り値: true 正常終了１, false 異常終了
//
boolean getFontDataBySJIS(byte* fontdata, uint16_t sjis) {
  int16_t code;
  boolean rc = false;

  if ( 0 > (code  = findcode(sjis))) { 
    code = findcode(0x81A0); 
    rc = false;  
  }
  if ( FONT_LEN  == Sequential_read((code)*7, fontdata, (byte)FONT_LEN) ) {
       rc =  true;
	   fontdata[7]=0; // 8バイト目に0をセット
  }
  return rc;
}

//
// SJIS半角コードをSJIS全角コードに変換する
// (変換できない場合は元のコードを返す)
//   sjis(in): SJIS文字コード
//   戻り値: 変換コード
uint16_t HantoZen(uint16_t sjis) {
  if (sjis < 0x20 || sjis > 0xdf) 
     return sjis;
  if (sjis < 0xa1)
    return  pgm_read_word(&zentable[sjis-0x20]);
  return pgm_read_word(&zentable[sjis+95-0xa1]);
}

// 指定したSJIS文字列の先頭のフォントデータの取得
//   data(out): フォントデータ格納アドレス
//   pSJIS(in): SJIS文字列
//   h2z(in)  : true :半角を全角に変換する false: 変換しない 
//   戻り値   : 次の文字列位置、取得失敗の場合NULLを返す
//
char* getFontData(byte* fontdata, char *pSJIS, bool h2z) {
  uint16_t sjis = 0;

  if (pSJIS == NULL || *pSJIS == 0) {
    return NULL;
  }
 
  sjis = (uint8_t)*pSJIS;  
  if ( isZenkaku(sjis) ) {
    sjis<<=8;
    pSJIS++;
    if (*pSJIS == 0) {
      return NULL;
    }
    sjis += (uint8_t)*pSJIS;
  }  
  pSJIS++;

  if (h2z) {
    sjis = HantoZen(sjis);
  }
  if (false == getFontDataBySJIS(fontdata, sjis)) {
    return NULL;
  }
  return (pSJIS);
}

// フォントデータテーブル先頭アドレス取得
// 戻り値 フォントデータテーブル先頭アドレス(PROGMEM上)
const uint8_t* getFontTableAddress() {
	return fdata;
}
