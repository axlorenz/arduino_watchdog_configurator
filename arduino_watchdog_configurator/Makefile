SRC=arduino_watchdog_configurator.ino
#OUT=$(SRC:.ino=.ino.with_bootloader.bin)
OUT_DIR=build

AC=arduino-cli
BFLAGS=-b arduino:avr:uno

# Add the crc folder as an include directory
CFLAGS=compile $(BFLAGS) --build-property build.extra_flags="-Icrc -Iarray" --output-dir $(OUT_DIR)

ifeq ($(OS),Windows_NT)
	PORT=COM21
else
	PORT=/dev/ttyS20
endif

PFLAGS=-p $(PORT)
UFLAGS=upload $(BFLAGS) $(PFLAGS) --input-dir $(OUT_DIR)

all: compile upload

.PHONY: compile upload clean

compile:
	$(AC) $(CFLAGS) $(SRC)

upload:
	$(AC) $(UFLAGS)

clean:
	rm -rf $(OUT_DIR)