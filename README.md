# DoorbellCamFirmware
Firmware for custom-made **notifier** based on **esp32** module. Notifications are received from a **wireless 433MHz** demodulator and shown through a series of **light effects**, which represent the state of the doorbell's FSM. A demux is connected to the esp32 in order to control a **led ring**; atm implemented animations are just forward and reverse **spinner**.

You can find other parts of the project at:
 - [https://github.com/mc-cat-tty/DoorbellCamDaemon](https://github.com/mc-cat-tty/DoorbellCamDaemon)
 - [https://github.com/mc-cat-tty/DoorbellCamPhy](https://github.com/mc-cat-tty/DoorbellCamPhy)

## Dependencies
 - PIO - Platform IO (suggested)
 - ESP Tool
 - IDF Monitor
 - xtensa-esp32-elf package

You may need to add `export PATH="$HOME/.platformio/penv/bin/:$PATH"` to your `~/.bashrc` or `~/.zshrc` shell configuration file.

## How to build
Development:
```bash
pio run --environment esp32doit-devkit-v1
```

Release:
```bash
pio run --environment release
```

## How to flash
Move into *DoorbellCamFirmware/.pio/build/[esp32doit-devkit-v1|release]*

```bash
esptool.py -p /dev/ttyUSB0 -b 115200 write_flash -z 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

## How to debug
Move into *DoorbellCamFirmware/.pio/build/[esp32doit-devkit-v1|release]*

Dump all symbols:
```bash
readelf -s firmware.elf
```

Translate address to file:line (the manual way):
```bash
addr2line -e firmware.elf 0x400d19b3:0x3ffb7ed0
```

Where `0x400d19b3:0x3ffb7ed0` is one of the address you can find in a esp32 core dump's backtrace.

Traslate a whole backtracke (the smart way):
```bash
idf_monitor.py --port /dev/ttyUSB0 firmware.elf
```