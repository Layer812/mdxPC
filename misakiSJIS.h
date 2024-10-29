// 
// misakiSJIS.h 美咲フォントドライバ ヘッダーファイル v1.0 by たま吉さん 2018/01/31
// 内部フラッシュメモリバージョン
//
// 修正 2019/07/16, １フォントあたり7バイトに圧縮
// 修正 2019/07/16, FTABLESIZEを実配列から算出に変更
//

#ifndef misakiSJIS_h
#define misakiSJIS_h

#include <Arduino.h>

#define FTABLESIZE     (sizeof ftable / sizeof ftable[0])       // フォントテーブルデータサイズ
#define FONT_LEN       7                                        // 1フォントのバイト数
int16_t findcode(uint16_t  sjiscode) ;                          // フォントコード検索
boolean getFontDataBySJIS(byte* fontdata, uint16_t sjis) ;      // SJISに対応する美咲フォントデータ8バイトを取得
uint16_t HantoZen(uint16_t sjis);                               // SJIS半角コードをSJIS全角コードに変換
char* getFontData(byte* fontdata,char *pSJIS,bool h2z=false);   // フォントデータ取得
const uint8_t*  getFontTableAddress();                          // フォントデータテーブル先頭アドレス取得

#endif
