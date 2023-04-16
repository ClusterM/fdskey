# FDSKey
Open source, cheap and easy to build Famicom Disk System drive emulator.

Work in progress. It's not finished yet.

TODO: add photo

How you can use it:
* Run disk images for Famicom Disk System from SD card 
* Dump disks using read FDS drive without additional hardware/cables
* Write disks using read FDS drive without additional hardware/cables

Requirements:
* Famicom
* Famicom Disk System RAM adaptor
* Micro SD card

# How to build it
You'll need few cheap components.

1. PCB

TODO: add PCB photo

PCB is designed in [DipTrace](https://diptrace.com/) software.

You can order it from any PCB factory (e.g. [jlcpcb.com](jlcpcb.com)) using gerber files.
* PCB thickness: 1.6mm
* Gold fingers are recommended

TODO: add gerber filenames.

2. STM32G0B0CET microcontroller

![image](https://user-images.githubusercontent.com/4236181/232314493-1ec8e30e-3a7c-4811-aa55-ce00b48657be.png)

You can easyly buy it on [mouser.com](https://www.mouser.com/c/?q=STM32G0B0CET) or [taobao.com](https://s.taobao.com/search?q=STM32G0B0CET).

3. OLED display module

![image](https://user-images.githubusercontent.com/4236181/232314733-8415926e-7fd4-463e-8dfe-214b7c0596d0.png)

![image](https://user-images.githubusercontent.com/4236181/232314774-186cd89f-30fd-4f91-9653-37cfe8fef6e9.png)

It's very popular OLED display, search for "SSD1306 0.91 inch OLED 128x32 4-pin" on [aliexpress.com](aliexpress.com), [ebay.com](ebay.com) or [taobao.com](taobao.com).

WARNING: some unscrupulous sellers can sell you used burned out display.

4. Micro SD card socket (push-push, 8-pin + card detect pin = 9-pin)

![image](https://user-images.githubusercontent.com/4236181/232315515-5448f67a-dd0d-40c4-9347-7212eabafad3.png)

![image](https://user-images.githubusercontent.com/4236181/232315553-8d20c2c3-7c77-4bec-bd75-0b12cd5d0591.png)

It's very popular push-push socket, you can find it on [aliexpress.com](aliexpdress.com). Also, you can search for "112J-TDAR-R01" model but there are many compatible models with other names. You can always edit PCB for other socket model.

5. LD1117S33 stabilizer

![image](https://user-images.githubusercontent.com/4236181/232316501-0c0928cc-6963-4bbd-998f-32091fde20a6.png)

You can buy it in any electronic components store.

6. Four SMD buttons

![image](https://user-images.githubusercontent.com/4236181/232316667-556b9a1f-eef8-4035-806b-d7917b8ea483.png)

Search for "3X4X2.5H SMD" buttons, it's easy to find.

7. Resistors and capacitors

There are very few of them:
* C1, C2 - two 22uF ceramic capacitors, 0603 size
* C3, C4 - two 100nF ceramic capacitors, 0402 size (0603 will be fine too)
* R1, R2 - two 1.5K resistors, 0402 size (0603 will be fine too), actually you can also use any values from 1K to 10K

You can buy it in any electronic components store.

8. Plastic case

Work in progress.
