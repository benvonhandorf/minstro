#![no_std]

#[cfg(feature = "rt")]
extern crate cortex_m_rt;
#[cfg(feature = "rt")]
pub use cortex_m_rt::entry;

pub use atsamd_hal as hal;
pub use hal::ehal;
pub use hal::pac;

use hal::clock::GenericClockController;
use hal::sercom::{
    Sercom1, i2c,
    uart::{self, BaudMode, Oversampling},
};
use hal::time::Hertz;

hal::bsp_pins! {
    PA02 {
        name: row_c
    }
    PA03 {
        name: row_a
    }
    PA04 {
        name: rgb_data
    }
    PA05 {
        name: nav_dir
    }
    PA06 {
        name: nav_a
    }
    PA07 {
        name: row_b
    }
    PA08 {
        /// XIN
        name: vfo_sw
    }
    PA09 {
        /// XOUT
        name: pa09
    }
    PA10 {
        name: nav_b
    }
    PA11 {
        name: nav_sw
    }
    PA14 {
        name: vfo_a
    }
    PA15 {
        name: vfo_b
    }
    PA16 {
        name: vfo_alt_a
    }
    PA17 {
        name: vfo_alt_b
    }
    PA22 {
        name: sda
        aliases: {
            AlternateC: Sda
        }
    }
    PA23 {
        name: scl
        aliases: {
            AlternateC: Scl
        }
    }
    PA24 {
        name: int
    }
    PA25 {
        name: pa25
    }
    PA27 {
        /// RST pin
        name: power_sw
    }
    PA28 {
        /// RST pin
        name: pa28
    }
    PA30 {
        /// SWCLK
        name: pa30
    }
    PA31 {
        /// SWDIO
        name: pa31
    }
}

//pub type UartPads = uart::Pads<Sercom0, UartRx, UartTx>;

/// UART device for the labelled RX & TX pins
//pub type Uart = uart::Uart<uart::Config<UartPads>, uart::Duplex>;

/// Convenience for setting up the D1 and D14 pins to
/// operate as UART RX/TX (respectively) running at the specified baud.
// pub fn uart(
//     clocks: &mut GenericClockController,
//     baud: impl Into<Hertz>,
//     sercom0: pac::SERCOM0,
//     pm: &mut pac::PM,
//     rx: impl Into<UartRx>,
//     tx: impl Into<UartTx>,
// ) -> Uart {
//     let gclk0 = clocks.gclk0();
//     let clock = &clocks.sercom0_core(&gclk0).unwrap();
//     let baud = baud.into();
//     let pads = uart::Pads::default().rx(rx.into()).tx(tx.into());
//     uart::Config::new(pm, sercom0, pads, clock.freq())
//         .baud(baud, BaudMode::Fractional(Oversampling::Bits16))
//         .enable()
// }

/// I2C pads for the labelled I2C peripheral
///
/// You can use these pads with other, user-defined [`i2c::Config`]urations.
pub type I2cPads = i2c::Pads<Sercom1, Sda, Scl>;

/// I2C master for the labelled I2C peripheral
///
/// This type implements [`Read`](ehal::blocking::i2c::Read),
/// [`Write`](ehal::blocking::i2c::Write) and
/// [`WriteRead`](ehal::blocking::i2c::WriteRead).
pub type I2c = i2c::I2c<i2c::Config<I2cPads>>;

/// Convenience for setting up the labelled SDA, SCL pins to
/// operate as an I2C master running at the specified frequency.
pub fn i2c_master(
    clocks: &mut GenericClockController,
    baud: impl Into<Hertz>,
    sercom: pac::SERCOM1,
    pm: &mut pac::PM,
    sda: impl Into<Sda>,
    scl: impl Into<Scl>,
) -> I2c {
    let gclk0 = clocks.gclk0();
    let clock = &clocks.sercom0_core(&gclk0).unwrap();
    let freq = clock.freq();
    let baud = baud.into();
    let pads = i2c::Pads::new(sda.into(), scl.into());
    i2c::Config::new(pm, sercom, pads, freq).baud(baud).enable()
}
