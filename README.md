# nordic-generic-01
zephyr based project with basic peripherals - oled, uart, button, led



west init -m https://github.com/nrfconnect/sdk-nrf --mr v2.5-branch .

west update --narrow -o=--depth=1

west build --pristine --board mdbt42q_nrf52 --build-dir ./build ./