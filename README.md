# mdxPC 1.1
MDX (mxdrv file) Player for Cardputer & Cardputer ADV.
It works like [this](https://x.com/i/status/1841840389306909125)<br>
It can play [MDX files]([https://en.wikipedia.org/wiki/VGM_(file_format)](https://w.atwiki.jp/mxdrv/)) from SDcard in your [Cardputer](https://shop.m5stack.com/products/m5stack-cardputer-kit-w-m5stamps3).<br>
## Install
1.Install [M5burner](https://docs.m5stack.com/en/uiflow/m5burner/intro)<br>
2.You can find the firmware on M5Burner or put share code 'WU01pyVHIFUtpn6u' in User Custom/Share Burn for 1.1 <br>
3.After flush the image, put MDX files and PDX files to your SD card (FAT32 formatted) and power on.<br>

## Usage
1.Push 'M' Button, you can see help.<br>
<img width="50%" src ="https://github.com/Layer812/mdxPC/blob/master/380501558-019905da-9d3b-4c67-bfcd-6aebdd8362bf.jpg"><br>
2.Select file by 'Cursor key'(Up/Down/Left/Right) without Fn. Hit 'Space Key' to Start/Stop playing.<br>
3.You can change master volume by +/-. also  change pcm volume by 9/0. 
## Compile
1.Download all of codes into "mdxCP" folder on ArduinoIDE as sketch.<br>
2.Change board type to Cardputer<br>
3.Change Flash size to 8MB
3.Change Flash mode QIO 120Mhz
5.Change partition to Custom<br>
6.Compile and Upload to Cardputer<br>
## Limitations (things to do)
This software provided as No warranty.<br>
Limit numbers.<br>
- MDX File size < 86Kb, PDX File size < 1.3M (Maximum sizes as confirmed)
- Files in the directory < 255, directory depth < 10. Path name length(include path name) < 255.
## Version history

| Version  | Share Code | Change |
|:----------:|:-----------:|:-------------|
| 1.1       |WU01pyVHIFUtpn6u  | Confirm on ADV. Delete some unused file for compile|
| 1.0       |gw6SwJtF4cZuexyl   | Fix bugs and support huge files(see limitation)|
| 0.6       |jxA5yGmQAlh5xcGB   | Make font of title bigger    |
| 0.51       | E75n6gUNNeAGLH7Y    | Adjust function added for ADPCM volume    |
| 0.5       | StQT4Eoo6ViDUrQl        | Adjust balance ADPCM with FM        |
| 0.4       | rLQTaChAORKh0pUh     | Fix bug related to ADPCM sample estimate       |
| 0.3       | EdxVqKAhN72avJvy      | Initial Commit     |

## License
- Rights of mdxtools original code belong to Vimpirefrog.
- Rights of customised code belong to Layer8.
- Please see the LICENSE file for more information.
## Thanks
- [Nyoron-x](https://asmpwx.seesaa.net/article/499317001.html)
- [Loveyan](https://github.com/lovyan03)
- [Gorry](https://gorry.haun.org/mx/)
- [Vampirefrog](https://github.com/vampirefrog/mdxtools)
- [Tamakichi](https://github.com/Tamakichi)
- [Kadoma-san](https://littlelimit.net/misaki.htm)
