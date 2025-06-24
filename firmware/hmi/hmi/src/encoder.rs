
use crate::hal::ehal::digital::v2::InputPin;

use crate::pac::{CorePeripherals, Peripherals};
use crate::hal::gpio:: {
    Pin,
    PullUpInput
};

pub struct Encoder<A, B> {
    value: i32,
    relative_reference: i32,
    pin_a: A,
    pin_b: B,
    a_value: bool,
}

pub trait AbsoluteValue {
    fn absolute_value(&self) -> i32;
    fn relative_value(&mut self) -> i32;
}

impl <A, B> Encoder<A, B>
where 
    A: InputPin,
    B: InputPin {

    fn new(a: A, b: B) -> Encoder<A, B> {
        Self {
            value: 0,
            relative_reference: 0,
            pin_a: a,
            pin_b: b,
            a_value: false,
        }
    }

    fn poll(&mut self) {
        let a_val = self.pin_a.is_high().ok().unwrap();

        if a_val != self.a_value {
            let b_val = self.pin_b.is_high().ok().unwrap();

            if b_val {
                self.value += 1;
            } else {
                self.value -= 1;
            }
        }
    }
}

impl<A, B> AbsoluteValue for Encoder<A, B> {
    fn absolute_value(&self) -> i32 {
        self.value
    }

    fn relative_value(&mut self) -> i32 {
        let val = self.value;

        let rel_val = val - self.relative_reference;

        self.relative_reference = val;

        rel_val
    }
}