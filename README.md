# pico_neopixel_vesc_display
a display for vesc devices, interfacing in canbus, using an 8x8 neopixel led array, support of canbus communication and also uart fallback.
More features like speed profiles etc to be added !

## Pinout 
[MCP2515](https://aliexpress.com/item/1005010670587660.html) : 
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

# Demo !

## Batt
https://github.com/user-attachments/assets/0839997a-7c17-4e2e-bb8f-af0fd879651a
## Speed
https://github.com/user-attachments/assets/00370f54-87fa-494c-b39d-fda8ffe59195
