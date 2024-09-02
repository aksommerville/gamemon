TARGETS:=tiny pico lib demo

# https://arduino.cc
tiny_ARDUINO_HOME:=/home/andy/.arduino15
tiny_ARDUINO_IDE_HOME:=/home/andy/.local/share/umake/electronics/arduino/
tiny_GCC_VERSION:=7-2017q4
tiny_PORT:=ttyACM0

# https://github.com/raspberrypi/pico-sdk
pico_PICO_SDK:=/home/andy/proj/thirdparty/pico-sdk
pico_ELF2UF2:=$(pico_PICO_SDK)/build/elf2uf2/elf2uf2
pico_GCCPFX:=arm-none-eabi-
pico_MOUNTPOINT:=/media/andy/RPI-RP2
