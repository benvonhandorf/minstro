# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Minstro is a remote control head for SDRs (Software Defined Radios) using SoapyRemote server. It features a dual-processor architecture with ESP32-S3 and SAMD10D microcontrollers.

## Build Commands

### ESP32-S3 Firmware (ESP-IDF)
```bash
cd firmware/esp32s3/bringup
idf.py build                    # Build firmware
idf.py flash                    # Flash to device
idf.py monitor                  # Serial monitor
idf.py flash monitor            # Flash and monitor
idf.py menuconfig              # Configure build options
idf.py clean                   # Clean build artifacts
```

### HMI Firmware (Rust)
```bash
cd firmware/hmi/hmi
cargo build --release          # Build firmware
cargo run --release            # Build and run via OpenOCD
```

## Development Setup

### ESP32-S3 Requirements
- ESP-IDF v5.4.1+
- Target device connected to `/dev/ttyACM0`
- VS Code with ESP-IDF extension recommended

### HMI (SAMD10D) Requirements  
- Rust toolchain with ARM Cortex-M0+ target
- OpenOCD for debugging/flashing
- `openocd.cfg` configured in `firmware/hmi/`

## Architecture

### Dual-Processor System
- **ESP32-S3**: Main processor handling WiFi, SD card, GPIO, addressable LEDs
- **SAMD10D**: HMI processor managing rotary encoders, user inputs, local LEDs
- **Communication**: I2C bus (SAMD10D as peripheral at address 0x22)

### ESP32-S3 Code Structure
Located in `firmware/esp32s3/bringup/main/`:
- `main.c` - Main application loop and initialization
- `bringup_*_test.c/h` - Modular hardware test components (LED, I2C, WiFi, GPIO, SD card)
- Each test module runs concurrently with configurable parameters

### HMI Code Structure  
Located in `firmware/hmi/hmi/src/`:
- `main.rs` - Main application and hardware initialization
- `i2c_peripheral.rs` - I2C communication with ESP32-S3
- `encoder.rs` - Rotary encoder input handling
- `comms.rs` - Inter-processor messaging protocol
- Custom BSP in `firmware/hmi/hmi_bsp/`

## Key Dependencies

### ESP32-S3
- ESP-IDF Component Manager dependencies in `idf_component.yml`
- `espressif/led_strip` v2.5.5 for addressable LED control
- Standard ESP-IDF libraries for WiFi, I2C, GPIO, SD card

### HMI (Rust)
- `no_std` embedded environment
- Custom board support package (BSP) architecture
- Timer-based WS2812 LED control implementation

## Hardware Interfaces

### ESP32-S3 Features
- GPIO 48: Addressable LED strip output
- I2C: Master mode for HMI communication
- WiFi: Network scanning and connection testing
- SD card: 4-bit SDIO interface
  - DAT0: GPIO39, DAT1: GPIO38, DAT2: GPIO43, DAT3: GPIO42
  - SCK: GPIO40, CMD: GPIO41, DET: GPIO44
- Various GPIO pins for testing

### SAMD10D (HMI) Features
- I2C peripheral at address 0x22
- Rotary encoder inputs with interrupt handling
- WS2812 addressable LEDs via timer-based protocol
- ADC inputs for analog controls

## Testing

### ESP32-S3 Testing
Run `pytest firmware/esp32s3/bringup/pytest_blink.py` for automated testing.

Test modules can be configured via `idf.py menuconfig` and include:
- LED functionality (GPIO and addressable)
- I2C bus scanning with timing configuration  
- WiFi scanning and optional iperf testing
- GPIO basic functionality
- SD card interface verification

### Development Tips
- Each ESP32-S3 test module runs independently and can be enabled/disabled
- Use `idf.py monitor` to see real-time test output and results
- HMI firmware uses interrupt-driven design for responsive user input
- Communication between processors follows a structured protocol in `protocol.rs`