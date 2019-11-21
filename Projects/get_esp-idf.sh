cd ~/esp
# this will create a folder esp-idf with version 4.0-beta2
git clone -b v4.0-beta2  https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
git submodule update --init --recursive

# this will apply possible patches
# patch modbus implementation for esp32
patch components/freemodbus/common/esp_modbus_master.c ~/esp/GME_Binary/patches/0001-esp-modbus-master.patch
# manage cmd 17
patch components/freemodbus/modbus/functions/mbfuncother.c ~/esp/GME_Binary/patches/0002_manage_cmd17.patch
patch components/freemodbus/modbus/include/mb_m.h ~/esp/GME_Binary/patches/0003_manage_cmd17.patch