## Overview

Rust firmware for the SAMD10D processor on the HMI board

## Layout

`hmi_bsp` is the BSP for the hardware and includes basic pin aliases.

`hmi` is the main firmware.  Includes `main`.

- `i2c_peripheral` implements hardware specific I2C peripheral interrupt handling and state tracking
- `comms` includes basic bus messaging
- `protocol`
- `encoder` handles reading and tracking encoder values using interupts on one of the configured pins

`hmi_inputs` includes the handling of different input devices.

