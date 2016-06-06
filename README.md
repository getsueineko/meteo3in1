При использовании библиотеки Adafruit_BME280_Library была исправлена директива
#define BME280_ADDRESS (0x76) в файле Adafruit_BME280.h

В Arduino\libraries\TFT_ILI9163C\_settings
Была закоментирована красная плата и раскоментирована черная, поскольку у меня именно она
//#define __144_RED_PCB__//128x128
#define __144_BLACK_PCB__//128x128
