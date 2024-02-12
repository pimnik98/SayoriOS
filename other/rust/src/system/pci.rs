

extern "C" {
    fn pci_read_confspc_word(bus: u8, slot: u8, func: u8, offset: u8) -> u16;
    fn pci_read32(bus: u8, slot: u8, func: u8, offset: u8) -> u32;
    fn pci_find_device(vendor: u16, device: u16, bus_ret: *mut u8, slot_ret: *mut u8, function_ret: *mut u8);
    fn pci_find_device_by_class_and_subclass(class: u16, subclass: u16, vendor_ret: *mut u16, devid_ret: *mut u16, bus_ret: *mut u8, slot_ret: *mut u8, function: *mut u8);
    fn pci_get_device(bus: u8, slot: u8, function: u8) -> u16;
    fn pci_write(bus: u8, slot: u8, func: u8, offset: u32, value: u32);
}

pub struct PCIDevice {
    pub vendor: u16,
    pub device_id: u16,
    pub bus: u8,
    pub slot: u8,
    pub function: u8,
}

impl PCIDevice {
    pub fn read(&self, offset: u8) -> u32 {
        unsafe { pci_read32(self.bus, self.slot, self.function, offset) }
    }

    pub fn read_bar(&self, bar: u8) -> u32 {
        self.read(0x10 + (bar * 4))
    }

    pub fn write(&self, offset: u32, value: u32) {
        unsafe {
            pci_write(self.bus, self.slot, self.function, offset, value);
        }
    }

    pub fn enable_bus_mastering(&self) {
        let mut command_register = self.read(4);
        command_register |= 0x05;
        self.write(4, command_register);

    }
}

pub fn find_device(vendor: u16, device: u16) -> Option<PCIDevice> {
    let mut dev: PCIDevice = PCIDevice {
        vendor,
        device_id: device,
        bus: 0,
        slot: 0,
        function: 0,
    };

    unsafe {
        pci_find_device(vendor, device, &mut dev.bus, &mut dev.slot, &mut dev.function);

        let devid: u16 = pci_get_device(dev.bus, dev.slot, dev.function);

        if devid == 0xffff {
            return None;
        }
    }

    Some(dev)
}

pub fn find_device_by_class_and_subclass(class: u16, subclass: u16) -> Option<PCIDevice> {
    let mut dev: PCIDevice = PCIDevice {
        vendor: 0,
        device_id: 0,
        bus: 0,
        slot: 0,
        function: 0,
    };

    unsafe {
        pci_find_device_by_class_and_subclass(class, subclass, &mut dev.vendor, &mut dev.device_id, &mut dev.bus, &mut dev.slot, &mut dev.function);

        if dev.vendor == 0 || dev.device_id == 0 {
            return None;
        }
    }

    Some(dev)
}