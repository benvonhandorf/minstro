#![no_std]
#![no_main]

use core::borrow::Borrow;

#[cfg(not(feature = "use_semihosting"))]
use panic_halt as _;
#[cfg(feature = "use_semihosting")]
use panic_semihosting as _;

use kib_board as bsp;

use bsp::entry;
use bsp::hal;
use bsp::pac;

use cortex_m::interrupt as interrupt_helpers;
use cortex_m::peripheral::NVIC;
use pac::interrupt;
use pac::{CorePeripherals, Peripherals};

use hal::clock::GenericClockController;
use hal::delay::Delay;
use hal::prelude::*;
use hal::sercom::i2c;
use hal::sercom::Sercom;
use hal::time::*;
use hal::timer::*;

fn main() {
    //Initial hardware configuration
    let mut peripherals = Peripherals::take().unwrap();
    let mut core = CorePeripherals::take().unwrap();
    let mut clocks = GenericClockController::with_internal_32kosc(
        peripherals.GCLK,
        &mut peripherals.PM,
        &mut peripherals.SYSCTRL,
        &mut peripherals.NVMCTRL,
    );

    let gclk0 = clocks.gclk0();
    let tc12 = &clocks.tc1_tc2(&gclk0).unwrap();

    let pins = bsp::Pins::new(peripherals.PORT);

    
}
