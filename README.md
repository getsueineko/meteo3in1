Данный код предназначен для реализации метеостанции с тремя видами отображения информации на arduino nano и таких датчиках, как  MH-Z19 (PIR датчик углекислого газа CO2), BME280 (датчик атмосферного давления, температуры и влажности) и часах реального времени RTC1307. Для отображения информации используется TFT_ILI9163C (128x128, черная плата) (отсюда http://ru.aliexpress.com/item/1PCS-1-44-128-128-SPI-TFT-LCD-Module-Replace-Nokia-5110-LCD-51/32332271521.html)

В программе реализовано три режима: режим часов (по умолчанию, после включения), общий режим (сводка метео-инфо), графический режим (метео-инфо в режиме крупных графических слайдов). Режимы переключаются с помощью аппаратной кнопки по кругу.

В проекте использовались некоторые модификации стандартных библиотек, чтобы датчики работали корректно.
При использовании библиотеки Adafruit_BME280_Library была исправлена директива:

    #define BME280_ADDRESS (0x76) в файле Adafruit_BME280.h (по умолчанию 0x77)

В Arduino\libraries\TFT_ILI9163C\_settings
Была закоментирована красная плата и раскоментирована черная (иначе будет наблюдаться сдвиг экрана, как правило, вверх), поскольку у меня именно она:

    //#define __144_RED_PCB__//128x128
    #define __144_BLACK_PCB__//128x128

