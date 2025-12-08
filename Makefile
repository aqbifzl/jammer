# FQBN for your board
FQBN ?= esp32:esp32:esp32doit-devkit-v1

# Serial port
PORT ?= /dev/ttyUSB0

# Serial baud rate
BAUD ?= 115200

# Build dirs
BUILD_DEBUG := build/debug
BUILD_RELEASE := build/release

# Arduino CLI executable
ARDUINO_CLI := arduino-cli

# Default build release
all: release

debug:
	$(ARDUINO_CLI) compile \
		--fqbn $(FQBN) \
		--build-path $(BUILD_DEBUG) \
		--optimize-for-debug \
		--jobs 0 \
		--verbose

release:
	$(ARDUINO_CLI) compile \
		--fqbn $(FQBN) \
		--build-path $(BUILD_RELEASE) \
		--jobs 0 \
		--verbose

upload-debug: debug
	$(ARDUINO_CLI) upload \
		-p $(PORT) \
		--fqbn $(FQBN) \
		--input-dir $(BUILD_DEBUG)

upload-release: release
	$(ARDUINO_CLI) upload \
		-p $(PORT) \
		--fqbn $(FQBN) \
		--input-dir $(BUILD_RELEASE)

monitor:
	$(ARDUINO_CLI) monitor -p $(PORT) --config baudrate=$(BAUD)
	
clean:
	rm -rf build

.PHONY: all debug release upload-debug upload-release clean monitor
