# pico_neopixel_vesc_display
a display for vesc devices, interfacing in canbus, using an 8x8 array


## Pinout 
MCP2515 : 
- p9 int
- p10 sck
- p11 si
- p12 so
- p13 cs

[WS2812 8x8 array](https://aliexpress.com/item/1005010801161509.html) 
- p2

Vesc1 UART :
- p0(tx) to vesc rx 
- p1(rx) to vesc tx

Vesc2 UART :
- p4(tx) to vesc rx 
- p5(rx) to vesc tx