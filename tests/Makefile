# Host-side tests for the Cloudline controller firmware.
#
#   make -C tests          build and run everything
#   make -C tests shots    also regenerate docs/img/screen_*.svg
#
# Both suites compile the firmware sources unchanged. The logic tests are
# built as C++11 (what avr-gcc uses for Arduino) as well as C++17.

CXX      ?= clang++
CXXFLAGS ?= -O1 -g -Wall -Wextra -Werror -Wno-unused-parameter
BUILD    := ../build
FW       := ../firmware/cloudline_controller
FW_SRC   := $(FW)/cloudline_controller.ino $(FW)/config.h $(FW)/fan_logic.h $(FW)/display_ui.h
SIM_SRC  := test_firmware_sim.cpp sim.cpp sim.h check.h $(wildcard stubs/*.h)
EEPROM   := $(BUILD)/eeprom.bin

.PHONY: all test shots clean
all: test

$(BUILD):
	mkdir -p $@

$(BUILD)/test_fan_logic_cxx11: test_fan_logic.cpp check.h $(FW)/fan_logic.h | $(BUILD)
	$(CXX) -std=c++11 $(CXXFLAGS) -o $@ test_fan_logic.cpp

$(BUILD)/test_fan_logic: test_fan_logic.cpp check.h $(FW)/fan_logic.h | $(BUILD)
	$(CXX) -std=c++17 $(CXXFLAGS) -o $@ test_fan_logic.cpp

$(BUILD)/test_firmware_sim: $(SIM_SRC) $(FW_SRC) | $(BUILD)
	$(CXX) -std=c++17 $(CXXFLAGS) -Istubs -x c++ -o $@ test_firmware_sim.cpp sim.cpp

test: $(BUILD)/test_fan_logic_cxx11 $(BUILD)/test_fan_logic $(BUILD)/test_firmware_sim
	$(BUILD)/test_fan_logic_cxx11 > /dev/null
	$(BUILD)/test_fan_logic
	$(BUILD)/test_firmware_sim boot1 $(EEPROM)
	$(BUILD)/test_firmware_sim boot2 $(EEPROM)
	$(BUILD)/test_firmware_sim diagboot $(EEPROM)
	$(BUILD)/test_firmware_sim wrap $(BUILD)/eeprom_blank.bin
	$(BUILD)/test_firmware_sim calfail $(BUILD)/eeprom_blank.bin
	@echo "ALL TESTS PASSED"

shots: $(BUILD)/test_firmware_sim
	mkdir -p ../docs/img
	$(BUILD)/test_firmware_sim boot1 $(EEPROM) ../docs/img
	$(BUILD)/test_firmware_sim calfail $(BUILD)/eeprom_blank.bin ../docs/img

clean:
	rm -rf $(BUILD)
