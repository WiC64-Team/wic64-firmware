#!/bin/bash
VERSION="$(git describe --tags --dirty)"
DIR="../wic64-firmware-binaries-${VERSION}"
TAR="${DIR}-${VERSION}"

idf.py reconfigure build

mkdir -p "${DIR}"
cp -v build/bootloader/bootloader.bin "${DIR}"
cp -v build/partition_table/partition-table.bin "${DIR}"
cp -v build/ota_data_initial.bin "${DIR}"
cp -v build/wic64.bin "${DIR}"

PREFIX_SH='esptool.py -p $1'
PREFIX_BAT='esptool.py -p %1'
ARGS='-b 460800 --before default_reset --after hard_reset --chip esp32  write_flash --flash_mode dio --flash_size detect --flash_freq 40m 0x1000 bootloader.bin 0x8000 partition-table.bin 0xd000 ota_data_initial.bin 0x10000 wic64.bin'

echo "$PREFIX_SH $ARGS" > "${DIR}/flash.sh"
echo "$PREFIX_BAT $ARGS" > "${DIR}/flash.bat"

README=$(cat <<'EOF'
1. Install esptool.py
2. Disconnect WiC64 from the C64 userport
3. Connect WiC64 to your PC via USB
4. Run the flash script for your system

Linux:
   sh flash.sh /dev/ttyUSB0

Windows:
    flash.bat COM1

Adjust the argument to the actual serial port used on your system
EOF
)

echo "${README}" > "${DIR}/README.txt"

tar vczf "${DIR}.tar.gz" "${DIR}" && rm -rf "${DIR}"
