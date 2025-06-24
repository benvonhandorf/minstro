#![no_std]

pub struct BusCommand {
    pub register: u8,
    pub data: [u8; 20],
    pub data_size: usize,
    pub read_direction: bool,
}

pub struct BusStatus {
    last_register: Option<u8>,
    read_direction: bool,
    data: [u8; 20],
    data_index: usize,
    data_size: usize,
    command: Option<BusCommand>,
    stopped: bool,
}

impl BusStatus {
    pub fn new() -> Self {
        Self {
            last_register: None,
            read_direction: false,
            data: [0u8; 20],
            data_index: 0,
            data_size: 0,
            command: None,
            stopped: true,
        }
    }

    pub fn addr(&mut self, read_direction: bool) {
        if !self.stopped {
            //Build a command for the previous operation
            self.build_command();
        }

        self.stopped = false;
        self.read_direction = read_direction;
        self.data_index = 0;

        if !read_direction {
            self.last_register = None;
            self.data_size = 0;
        }
    }

    pub fn is_reading(&self) -> bool {
        self.read_direction
    }

    pub fn write_data(&mut self, data: u8) -> bool {
        if self.last_register.is_none() {
            self.last_register = Some(data);
            true
        } else {
            self.data[self.data_index] = data;
            self.data_index += 1;
            self.data_size += 1;

            true
        }
    }

    pub fn read_data(&mut self) -> u8 {
        if self.data_index < self.data_size {
            let result = self.data[self.data_index];
            self.data_index += 1;

            result
        } else {
            0xFF
        }
    }

    pub fn stop(&mut self) {
        self.build_command();
        self.stopped = true;
    }

    fn build_command(&mut self) {
        if let Some(last_register) = self.last_register {
            let result = BusCommand {
                register: last_register,
                data: self.data,
                data_size: self.data_index,
                read_direction: self.read_direction,
            };

            self.command = Some(result);
        }
    }

    pub fn process(&mut self) -> Option<BusCommand> {
        let result = self.command.take();
        self.command = None;

        result
    }

    pub fn can_provide_data(&self) -> bool {
        self.stopped
    }

    pub fn provide_data(&mut self, register: u8, data: &[u8; 20], data_size: usize) {
        if Some(register) == self.last_register {
            self.data[..20].copy_from_slice(data);
            self.data_size = data_size;
            self.data_index = 0;
        }
    }
}
