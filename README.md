# mdxPC 0.51
MDX (mxdrv file) Player for Cardputer 
It works like [this](https://x.com/i/status/1841840389306909125)<br>
It can play [MDX files]([https://en.wikipedia.org/wiki/VGM_(file_format)](https://w.atwiki.jp/mxdrv/)) from SDcard in your [Cardputer](https://shop.m5stack.com/products/m5stack-cardputer-kit-w-m5stamps3).<br>
## Install
1.Install [M5burner](https://docs.m5stack.com/en/uiflow/m5burner/intro)<br>
2.Put share code 'E75n6gUNNeAGLH7Y' in User Custom/Share Burn for 0.51 <br>
3.After flush the image, put MDX files and PDX files to your SD card (FAT32 formatted) and power on.<br>

## Usage
1.Push 'M' Button, you can see help.<br>
<img width="50%" src ="https://github.com/user-attachments/assets/019905da-9d3b-4c67-bfcd-6aebdd8362bf"><br>
2.Select file by 'Cursor key'(Up/Down) without Fn. Hit 'Space Key' to Start/Stop playing.<br>
## Compile
1.Download all of codes into "mdxCP" folder on ArduinoIDE as sketch.<br>
2.Change board type to Cardputer<br>
3.Change partition to Custom<br>
4.Compile and Upload to Cardputer<br>
## Limitations (things to do)
This software provided as No warranty.
- Limit numbers, MDX File size < 20Kb, PDX File size < 2MB, files in the directory < 255, directory depth < 7. Path name < 255.
- Contain bugs...
## Version history

| Version  | Share Code | Change |
|:----------:|:-----------:|:-------------|
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
