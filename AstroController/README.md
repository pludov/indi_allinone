# Building


## Install PIO

Following instructions at:
https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html#super-quick-macos-linux

```bash
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
ln -s ~/.platformio/penv/bin/platformio ~/.local/bin/platformio
ln -s ~/.platformio/penv/bin/pio ~/.local/bin/pio
ln -s ~/.platformio/penv/bin/piodebuggdb ~/.local/bin/piodebuggdb
```



## Build

```
platformio pkg  install
platformio run  --environment release
```

This create the files
  * .pio/build/release/firmware.elf
  * .pio/build/release/firmware.bin
  * .pio/build/release/firmware.uf2

