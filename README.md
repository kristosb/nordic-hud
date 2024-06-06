# nordic-generic-01
zephyr based project with basic peripherals - oled, uart, button, led


py -m venv .venv
.venv\Scripts\activate
pip3 install -U west
west init -m https://github.com/kristosb/nordic-hud --mr main hud-workspace
cd hud-workspace
west update

#west init -m https://github.com/nrfconnect/sdk-nrf --mr v2.5-branch .

#west update --narrow -o=--depth=1

#west build --pristine --board mdbt42q_nrf52 --build-dir ./build ./