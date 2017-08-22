# cdc_com_test
Application to reproduce cdc communication isuue on win10

TEST :
μC : LPC4333JBD144                                debug: NXP LPC-Link 2 Rev B
Using JTAG interface to debug.                    LPCXpresso vers:	8.2.0.201607221326	LPCXpresso

The LPCXPresso project is based on the LPCOpen example project : "lpcopen_2_19_lpcxpresso_ngx_xplorer_4330"  "usbd_rom_cdc_vcom".
It use the virtual COM.
The Device send 225792 bytes representing an image(392*576 bytes) via virtual serial COM port. It send the data by packet of 256 byte. Each packet is compose of a byte sequence : 0x00,0x01,…,0xFF.
When μC received the char’0’ it send back the size of image data (225792) as a uint32_t  (4bytes).
When μC received the char’1’ it send the 225792 data bytes.
