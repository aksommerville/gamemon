tiny_MIDDIR:=mid/tiny
tiny_OUTDIR:=out/tiny

tiny_OPT_ENABLE:=tiny

tiny_TOOLCHAIN:=$(tiny_ARDUINO_HOME)/packages/arduino/tools/arm-none-eabi-gcc/$(tiny_GCC_VERSION)

# -Wp,-w disables warnings about redefined macros, which we need because digitalPinToInterrupt gets defined twice in the TinyCircuits headers.
# FMN_IMAGE_CAPS=16 means we only support FMN_IMAGE_FMT_Y2
tiny_CCOPT:=-mcpu=cortex-m0plus -mthumb -c -g -Os -MMD -std=gnu11 -ffunction-sections -fdata-sections \
  -nostdlib --param max-inline-insns-single=500 -Wp,-w
tiny_CXXOPT:=$(filter-out -std=gnu11,$(tiny_CCOPT)) -std=gnu++11  -fno-threadsafe-statics -fno-rtti -fno-exceptions
tiny_CCWARN:=-Wno-expansion-to-defined -Wno-redundant-decls
tiny_CCDEF:=-DF_CPU=48000000L -DARDUINO=10819 -DARDUINO_SAMD_ZERO -DARDUINO_ARCH_SAMD -D__SAMD21G18A__ -DUSB_VID=0x03EB -DUSB_PID=0x8009 \
  -DUSBCON "-DUSB_MANUFACTURER=\"TinyCircuits\"" "-DUSB_PRODUCT=\"TinyArcade\"" -DCRYSTALLESS \
  $(foreach U,$(tiny_OPT_ENABLE),-DFMN_USE_$U=1) \
  -DFMN_IMAGE_CAPS=16 -DFMN_TILESIZE=8
  
tiny_CCINC:=-Isrc \
  -I$(tiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Include/ \
  -I$(tiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS-Atmel/1.2.0/CMSIS/Device/ATMEL/ \
  -I$(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino \
  -I$(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade \
  -I$(tiny_ARDUINO_IDE_HOME)/libraries/SD/src \
  -I$(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/Wire \
  -I$(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/HID \
  -I$(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/SPI 
  
tiny_CC:=$(tiny_TOOLCHAIN)/bin/arm-none-eabi-gcc $(tiny_CCOPT) $(tiny_CCWARN) $(tiny_CCDEF) $(tiny_CCINC)
tiny_CXX:=$(tiny_TOOLCHAIN)/bin/arm-none-eabi-g++ $(tiny_CXXOPT) $(tiny_CCWARN) $(tiny_CCDEF) $(tiny_CCINC)
tiny_AS:=$(tiny_TOOLCHAIN)/bin/arm-none-eabi-gcc -xassembler-with-cpp $(tiny_CCOPT) $(tiny_CCWARN) $(tiny_CCDEF) $(tiny_CCINC)

tiny_LDOPT_SOLO:=-T$(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade/linker_scripts/gcc/flash_with_bootloader.ld 
tiny_LDOPT_HOSTED:=-T$(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade/linker_scripts/gcc/link_for_menu.ld 
tiny_LDOPT:=-Os -Wl,--gc-sections -save-temps \
  --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m0plus -mthumb \
  -Wl,--check-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align 
tiny_LDPOST:=-Wl,--start-group -L$(tiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Lib/GCC/ -larm_cortexM0l_math -lm -Wl,--end-group
tiny_LD_SOLO:=$(tiny_TOOLCHAIN)/bin/arm-none-eabi-g++ $(tiny_LDOPT_SOLO) $(tiny_LDOPT)
tiny_LD_HOSTED:=$(tiny_TOOLCHAIN)/bin/arm-none-eabi-g++ $(tiny_LDOPT_HOSTED) $(tiny_LDOPT)

tiny_OBJCOPY:=$(tiny_TOOLCHAIN)/bin/arm-none-eabi-objcopy

#TODO Are any of these unnecessary?
tiny_EXTFILES:= \
  $(tiny_ARDUINO_IDE_HOME)/libraries/SD/src/File.cpp \
  $(tiny_ARDUINO_IDE_HOME)/libraries/SD/src/SD.cpp \
  $(tiny_ARDUINO_IDE_HOME)/libraries/SD/src/utility/Sd2Card.cpp \
  $(tiny_ARDUINO_IDE_HOME)/libraries/SD/src/utility/SdFile.cpp \
  $(tiny_ARDUINO_IDE_HOME)/libraries/SD/src/utility/SdVolume.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/Wire/Wire.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/HID/HID.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/SPI/SPI.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade/variant.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/pulse_asm.S \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/samd21_host.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/WInterrupts.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/avr/dtostrf.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/cortex_handlers.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/hooks.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/itoa.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/delay.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/pulse.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/startup.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_analog.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_shift.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_digital.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_private.c \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/IPAddress.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Print.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Reset.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/SERCOM.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Stream.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Tone.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/CDC.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/PluggableUSB.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/USBCore.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Uart.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/WMath.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/WString.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/abi.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/main.cpp \
  $(tiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/new.cpp

# Now do our usual build stuff...

tiny_SRCFILES:=$(filter src/tiny/%,$(SRCFILES))

# TODO Generate menu tsv.
#tiny_MENU_SPLASH:=$(tiny_OUTDIR)/gamemon.tsv
#$(tiny_MENU_SPLASH):src/opt/tiny/menu_splash.png;$(PRECMD) echo "TODO generate $@ from $<" ; exit 1
#tiny-all:$(tiny_MENU_SPLASH)

tiny_CFILES:=$(filter %.c %.cpp %.S,$(tiny_SRCFILES)) $(tiny_EXTFILES)
tiny_OFILES:=$(patsubst src/%,$(tiny_MIDDIR)/%, \
  $(patsubst $(tiny_ARDUINO_IDE_HOME)/%,$(tiny_MIDDIR)/%, \
  $(patsubst $(tiny_ARDUINO_HOME)/%,$(tiny_MIDDIR)/%, \
  $(addsuffix .o,$(basename $(tiny_CFILES))) \
)))
-include $(tiny_OFILES:.o=.d)

define tiny_SRCRULES
  $(tiny_MIDDIR)/%.o:$1/%.c  ;$$(PRECMD) $(tiny_CC) -o $$@ $$<
  $(tiny_MIDDIR)/%.o:$1/%.cpp;$$(PRECMD) $(tiny_CXX) -o $$@ $$<
  $(tiny_MIDDIR)/%.o:$1/%.S  ;$$(PRECMD) $(tiny_AS) -o $$@ $$<
endef
$(foreach S,src $(tiny_MIDDIR) $(tiny_ARDUINO_HOME) $(tiny_ARDUINO_IDE_HOME),$(eval $(call tiny_SRCRULES,$S)))

tiny_OFILES_HOSTED:=$(filter-out %/tiny_upload.o,$(tiny_OFILES))
tiny_OFILES_SOLO:=$(tiny_OFILES_HOSTED)

tiny_HOSTED_ELF:=$(tiny_MIDDIR)/gamemon-hosted.elf
$(tiny_HOSTED_ELF):$(tiny_OFILES_HOSTED);$(PRECMD) $(tiny_LD_HOSTED) -o$@ $(tiny_OFILES_HOSTED) $(tiny_LDPOST)

tiny_SOLO_ELF:=$(tiny_MIDDIR)/gamemon-solo.elf
$(tiny_SOLO_ELF):$(tiny_OFILES_SOLO);$(PRECMD) $(tiny_LD_SOLO) -o$@ $(tiny_OFILES_SOLO) $(tiny_LDPOST)

tiny_HOSTED_BIN:=$(tiny_OUTDIR)/gamemon-hosted.bin
tiny_SOLO_BIN:=$(tiny_OUTDIR)/gamemon-solo.bin
tiny-all:$(tiny_HOSTED_BIN) $(tiny_SOLO_BIN)
$(tiny_HOSTED_BIN):$(tiny_HOSTED_ELF);$(PRECMD) $(tiny_OBJCOPY) -O binary $< $@
$(tiny_SOLO_BIN):$(tiny_SOLO_ELF);$(PRECMD) $(tiny_OBJCOPY) -O binary $< $@

# Beware that "running" means flashing the device. It will always boot into Gamemon after this.
tiny-run:$(tiny_SOLO_BIN); \
  stty -F /dev/$(tiny_PORT) 1200 ; \
  sleep 2 ; \
  $(tiny_ARDUINO_HOME)/packages/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=$(tiny_PORT) -U true -i -e -w $(tiny_SOLO_BIN) -R

# Flash the TinyArcade menu program, to revert a flash of Gamemon.
tiny-menu:; \
  stty -F /dev/$(tiny_PORT) 1200 ; \
  sleep 2 ; \
  $(tiny_ARDUINO_HOME)/packages/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=$(tiny_PORT) -U true -i -e -w etc/ArcadeMenu.ino.bin -R
