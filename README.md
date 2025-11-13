a jammer

WIRING:
MISO -- GPIO19
SCK -- GPIO18
MOSI -- GPIO23
CE -- GPIO16
CSN -- GPIO17
GND -- GND
VCC -- 3.3V

NRF works on 2400-2525MHz frequencies, 1 channel = 1 MHz and there are 125
channels

https://docs.nordicsemi.com/bundle/nRF24L01P_PS_v1.0/resource/nRF24L01P_PS_v1.0.pdf

The status of RPD is correct when RX mode is enabled and after a wait time of
Tstby2a +Tdelay_AGC= 130us + 40us

On nixOS run:
nix develop github:mirrexagon/nixpkgs-esp-dev#esp32-idf

run:
idf.py flash monitor
