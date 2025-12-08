NRF24L01+PA+LNA jammer for 2.4GHz band.

This project aims to create a jammer for devices operating in the [https://en.wikipedia.org/wiki/2.4_GHz_radio_use](2.4GHz band).

### Wiring
Check config.h. Currently we use 2 NRF modules, an SSD1306-compatible display (I2C), and 3 buttons for navigation.

### Theoretical basis
The NRF works on 2400-2525MHz frequencies. 1 channel = 1 MHz and there are 125 channels.

We can choose TX power and speed. Bandwidth depends on speed.
TX power: -18dBm, -12dBm, -6dBm, 0dBm
Speed: 1Mbps, 2Mbps, 250Kbps
Bandwidth: 1MHz or 2MHz

[https://docs.nordicsemi.com/bundle/nRF24L01P_PS_v1.0/resource/nRF24L01P_PS_v1.0.pdf](https://docs.nordicsemi.com/bundle/nRF24L01P_PS_v1.0/resource/nRF24L01P_PS_v1.0.pdf)

The status of RPD is correct when RX after an appropriate delay:
Tstby2a +Tdelay_AGC= 130us + 40us. RPD is used to detect activity on a band.

The RF channel frequency determines the center of the channel used by the nRF24L01+. The channel
occupies a bandwidth of less than 1MHz at 250kbps and 1Mbps and a bandwidth of less than 2MHz at
2Mbps. nRF24L01+ can operate on frequencies from 2.400GHz to 2.525GHz. The programming resolu-
tion of the RF channel frequency setting is 1MHz

build, upload and monitor:
make upload-release monitor
