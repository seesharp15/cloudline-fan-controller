# Host-side tests for the Cloudline controller firmware.
#
#   make -C test          build and run everything
#   make -C test shots    also regenerate docs/img/screen_*.svg
#
# Both suites compile the firmware sources unchanged. The logic tests are
# built as C++11 (the language level avr-gcc uses for Arduino) and C++17.

CXX      ?= g++
CXXFLAGS ?= -O1 -g -Wall -Wextra -Werror -Wno-unused-parameter
BUILD    := ../build
FW       := ../firmware/cloudline_controller
FW_SRC   := $(FW)/app.cpp $(FW)/config.h $(FW)/fan_logic.h $(FW)/display_ui.h
SIM_SRC  := test_firmware_sim.cpp sim.cpp sim.h check.h $(wildcard stubs/*.h stubs/avr/*.h)
SCENARIOS := boot1 powercycle serial oledaddr nodisplay diagboot wrap calfail

.PHONY: all test shots clean no-eeprom pins
all: test

$(BUILD):
	mkdir -p $@

$(BUILD)/test_fan_logic_cxx11: test_fan_logic.cpp check.h $(FW)/fan_logic.h | $(BUILD)
	$(CXX) -std=c++11 $(CXXFLAGS) -o $@ test_fan_logic.cpp

$(BUILD)/test_fan_logic: test_fan_logic.cpp check.h $(FW)/fan_logic.h | $(BUILD)
	$(CXX) -std=c++17 $(CXXFLAGS) -o $@ test_fan_logic.cpp

$(BUILD)/test_firmware_sim: $(SIM_SRC) $(FW_SRC) | $(BUILD)
	$(CXX) -std=c++17 $(CXXFLAGS) -Istubs -o $@ test_firmware_sim.cpp sim.cpp

# The speed setting must live in RAM only: no EEPROM access anywhere.
no-eeprom:
	@if grep -rniE 'EEPROM|eeprom_(write|update|read)' $(FW); then \
	  echo "FAIL: firmware references EEPROM"; exit 1; \
	else echo "no-eeprom: firmware has no EEPROM access"; fi

# Pin numbers must agree across config.h, schematic, wiring view, PINOUT, README.
pins:
	@python3 check_pins.py | tail -1

test: $(BUILD)/test_fan_logic_cxx11 $(BUILD)/test_fan_logic $(BUILD)/test_firmware_sim no-eeprom pins
	$(BUILD)/test_fan_logic_cxx11 > /dev/null
	$(BUILD)/test_fan_logic
	@set -e; for s in $(SCENARIOS); do $(BUILD)/test_firmware_sim $$s; done
	@echo "ALL TESTS PASSED"

shots: $(BUILD)/test_firmware_sim
	mkdir -p ../docs/img
	$(BUILD)/test_firmware_sim boot1 ../docs/img
	$(BUILD)/test_firmware_sim calfail ../docs/img

clean:
	rm -rf $(BUILD)
