
use crate::comms::BusCommand;

use smart_leds::SmartLedsWrite;
use smart_leds::RGB8;

pub fn process_command<LedStrand>(command: &BusCommand) 
where LedStrand: SmartLedsWrite<Error = (), Color = RGB8> {
    match command.register {
        0x20 => {
            if command.data_size == 1 && (1..9).contains(&command.data[0]) {

            }
        }

        _ => { }
    }
}

pub fn build_response<LedStrand>(register: u8) -> Option<([u8; 20], usize)>
where LedStrand: SmartLedsWrite<Error = (), Color = RGB8> {
    
    let mut register_data: [u8; 20] = [0; 20];

    match register {
        0x10 => {
            // let (octave, octave_notes) = synth_engine.get_octave_notes();

            // register_data[0] = octave;
            // register_data[1..14].copy_from_slice(&octave_notes);

            Some((register_data, 14))
        }
        0x20 => {
            // let octave = synth_engine.state.octave;

            // register_data[0] = octave;

            Some((register_data, 1))
        }
        _ => { 
            None
        }
    }
}