#![no_std]
#![no_main]

mod comms;
mod encoder;
mod i2c_peripheral;
mod protocol;

use core::borrow::Borrow;

use hmi_bsp::ehal;
use hmi_bsp::pac::gclk;
use hmi_bsp as bsp;

use bsp::entry;
use bsp::hal;
use bsp::pac;

use hal::sercom::Sercom1;
use hal::time::MegaHertz;
use hal::timer::TimerCounter;

#[cfg(not(feature = "use_semihosting"))]
use panic_halt as _;
#[cfg(feature = "use_semihosting")]
use panic_semihosting as _;

use cortex_m::peripheral::NVIC;
use pac::{interrupt, CorePeripherals, Peripherals};
use cortex_m::interrupt as interrupt_helpers;

use hal::clock::GenericClockController;
use hal::delay::Delay;
use hal::ehal::blocking::delay::DelayMs;
use hal::prelude::*;
use hal::sercom::Sercom;
use hal::sercom::i2c;

use hal::gpio::{
    Pin,
    AlternateB,
};

use hal::adc::{
    Adc,
    SampleRate,
    Gain,
    Resolution,
};

use hal::time::*;
use hal::timer::*;

use smart_leds::{hsv::RGB8, SmartLedsWrite};
use ws2812_timer_delay::Ws2812;

use rtt_target::{rtt_init_print, rprintln};

const ADDRESS: u8 = 0x22;

#[entry]
fn main() -> ! {
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

    let mut delay = Delay::new(core.SYST, &mut clocks);

    rtt_init_print!();

    let mut adc = Adc::adc(peripherals.ADC, &mut peripherals.PM, &mut clocks);
    adc.samples(SampleRate::_128);
    adc.resolution(Resolution::_12BIT);

    let pins = bsp::Pins::new(peripherals.PORT);

    //Configure I2C
    let sercom1_clock = &clocks.sercom1_core(&gclk0).unwrap();
    let pads: i2c::Pads<
        Sercom1,
        hal::gpio::Pin<hal::gpio::PA22, _>,
        hal::gpio::Pin<hal::gpio::PA23, _>,
    > = i2c::Pads::new(pins.sda, pins.scl);

    let mut sercom = peripherals.SERCOM1;

    sercom.enable_apb_clock(&peripherals.PM);

    unsafe {
        core.NVIC.set_priority(interrupt::SERCOM0, 1);
        NVIC::unmask(interrupt::SERCOM0);
    }

    let interrupt_pin = Some(pins.int.into_push_pull_output());

    i2c_peripheral::configure_bus_status();

    i2c_peripheral::configure_sercom(sercom, ADDRESS);

    let mut row_a: Pin<_, AlternateB> = pins.row_a.into_mode();
    let mut row_b: Pin<_, AlternateB> = pins.row_b.into_mode();
    let mut row_c: Pin<_, AlternateB> = pins.row_c.into_mode();
    let mut nav_dir: Pin<_, AlternateB> = pins.nav_dir.into_mode();

    let nav_a = pins.nav_a.into_pull_up_input();

    let tc12_led_timer = &clocks.tc1_tc2(&gclk0).unwrap();

    let mut led_timer = TimerCounter::tc1_(tc12_led_timer, peripherals.TC1, &mut peripherals.PM);
    led_timer.start(MegaHertz::MHz(7).into_duration()); //TODO: This was 7 in the other project.  Investigate, docs call for 3

    let mut led_data_pin = pins.rgb_data.into_push_pull_output();
    led_data_pin.set_drive_strength(true);

    let mut led_strand = Ws2812::new(led_timer, led_data_pin);

    let mut led_data: [RGB8; 12] = [RGB8::default() ;12];

    led_strand.write(led_data.iter().cloned())
            .unwrap();

    let mut cycle = 0;

    let mut reading_a : u16;
    let mut reading_b : u16;
    let mut reading_c : u16;
    let mut reading_dir : u16;

    loop {
        //Process protocol commands
        let command = interrupt_helpers::free(|cs| {
            if let Some(comms_status) = i2c_peripheral::BUS_STATUS.borrow(cs).borrow_mut().as_mut()
            {
                comms_status.process()
            } else {
                None
            }
        });

        for pixel in &mut led_data {
            pixel.r = 0;
            pixel.g = 0;
            pixel.b = 0;
        }

        // led_data[cycle].g = 20;

        led_strand.write(led_data.iter().cloned())
            .unwrap();

        cycle = (cycle + 1) % 12;

        reading_a = adc.read(&mut row_a).unwrap();
        reading_b = adc.read(&mut row_b).unwrap();
        reading_c = adc.read(&mut row_c).unwrap();
        reading_dir = adc.read(&mut nav_dir).unwrap();

        rprintln!("{}, {}, {}, {}", reading_a, reading_b, reading_c, reading_dir);

    }
}
