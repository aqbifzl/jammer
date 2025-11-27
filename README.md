a jammer

WIRING:
MISO -- GPIO19
SCK -- GPIO18
MOSI -- GPIO23
CE -- GPIO16
CSN -- GPIO17
GND -- GND
VCC -- 3.3V

NRF works on 2400-2525MHz frequencies, 1 channel = 1 MHz and there are 126
channels

https://docs.nordicsemi.com/bundle/nRF24L01P_PS_v1.0/resource/nRF24L01P_PS_v1.0.pdf

The status of RPD is correct when RX mode is enabled and after a wait time of
Tstby2a +Tdelay_AGC= 130us + 40us

The RF channel frequency determines the center of the channel used by the nRF24L01+. The channel
occupies a bandwidth of less than 1MHz at 250kbps and 1Mbps and a bandwidth of less than 2MHz at
2Mbps. nRF24L01+ can operate on frequencies from 2.400GHz to 2.525GHz. The programming resolu-
tion of the RF channel frequency setting is 1MHz

On nixOS run:
nix develop github:mirrexagon/nixpkgs-esp-dev#esp32-idf

run:
idf.py flash monitor
