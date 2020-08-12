Simple FTP server matched with micro SD card reader for TRS-80 Model 100.

This project was heavily inspired by David Stephenson's project called "Model 100 SD Card reader". 

https://www.zx81keyboardadventure.com/2017/08/model-100-micro-sd-card-reader-mytesed.html

His project was built around the Arduino Uno and is a neat way to load and save files to an SD card from the TRS-80 model 100. 

I just added the ftp server part and tested it on Wemos D1 because i liked how easy it was to build with the Wemos.
The display is a 128x32 oled but could have been the oled hat from Wemos. At first the Oled was used for debugging since i could not get the software serial working on the Wemos from lack of GPIOs but turned out becoming a nice feature as it displays it's working status and IP adress when connected. You can also esily power the D1 with a usb power bank.

Parts used are:
Wemos D1
SD card hat for Wemos D1
128x32 white oled
mini RS232 to TTL converter module
4x6cm proto board
DB25 male connector



Code still needs work. Downloading from SD is not implemented and D1 may need a reset between uploads. 

When you run the basic program on the TRS-80, it will ask you for a file name (UPPERCASE) where you enter the name without the ".DO" extension. If you need to upload an ".BA" file to the SD card just save it as ".DO" and proceed as above.



When using your FTP client(FileZilla) to connect to the server, make sure to use FTP not SFTP.Also be sure to use "plain FTP(insecure)".

Most of this page was written with the TRS-80 because it had to be.. and the keyboard on it is really nice to type with.
# FTP_for_TRS-80
