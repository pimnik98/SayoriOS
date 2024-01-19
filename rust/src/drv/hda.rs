use crate::{qemu_log, qemu_ok};
use crate::qemu_err;
use crate::std::mm::*;
use crate::system::pci;
use crate::system::pci::PCIDevice;

const REG_GCAP: u32 = 0x00;  // Global Capabilities 	(includes number of DMA engines for input and output streams)
const REG_VMIN: u32 = 0x02;  // Minor Version
const REG_VMAJ: u32 = 0x03;  // Major Version
const REG_OUTPAY: u32 = 0x04;  // Output Payload Capacity 	(packet size limit for the/each output line)
const REG_INPAY: u32 = 0x06;  // Input Payload Capacity 	(packet size limit for each input line)
const REG_GCTL: u32 = 0x08;  // Global Control 	(used to reset the link and codec)
const REG_WAKEEN: u32 = 0x0C;  // Wake Enable
const REG_STATESTS: u32 = 0x0E;  // State Change Status
const REG_GSTS: u32 = 0x10;  // Global Status
const REG_OUTSTRMPAY: u32 = 0x18;  // Output Stream Payload Capability
const REG_INSTRMPAY: u32 = 0x1A;  // Input Stream Payload Capability
const REG_INTCTL: u32 = 0x20;  // Interrupt Control
const REG_INTSTS: u32 = 0x24;  // Interrupt Status
const REG_COUNTER: u32 = 0x30;  // Wall Clock Counter
const REG_SSYNC: u32 = 0x34;  // Stream Synchronization 	(set bits 0-29 to pause DMA streams 1-30)
const REG_CORBLBASE: u32 = 0x40;  // CORB Lower Base Address 	(command output ring buffer address)
const REG_CORBUBASE: u32 = 0x44;  // CORB Upper Base Address
const REG_CORBWP: u32 = 0x48;  // CORB Write Pointer
const REG_CORBRP: u32 = 0x4a;  // CORB Read Pointer
const REG_CORBCTL: u32 = 0x4c;  // CORB Control
const REG_CORBSTS: u32 = 0x4d;  // CORB Status
const REG_CORBSIZE: u32 = 0x4e;  // CORB Size
const REG_RIRBLBASE: u32 = 0x50;  // RIRB Lower Base Address 	(response input ring buffer address)
const REG_RIRBUBASE: u32 = 0x54;  // RIRB Upper Base Address
const REG_RIRBWP: u32 = 0x58;  // RIRB Write Pointer
const REG_RINTCNT: u32 = 0x5a;  // Response Interrupt Count
const REG_RIRBCTL: u32 = 0x5c;  // RIRB Control
const REG_RIRBSTS: u32 = 0x5d;  // RIRB Status
const REG_RIRBSIZE: u32 = 0x5e;  // RIRB Size
const REG_ICOI: u32 = 0x60; // Immediate Command Output Interface
const REG_IRII: u32 = 0x64; // Immediate Response Input Interface
const REG_ICS: u32 = 0x68; // Immediate Command Status
const REG_DPLBASE: u32 = 0x70; // DMA Position Lower Base Address
const REG_DPUBASE: u32 = 0x74; // DMA Position Upper Base Address
const OFFSET_SD: u32 = 0x80; // Stream Descriptors

struct HDADevice {
    pcidev: PCIDevice,
    mmio_base: u32
}

static mut main_hda_device: Option<HDADevice> = None;

#[no_mangle]
pub extern "C" fn intel_hda_init() {
    let result = pci::find_device_by_class_and_subclass(4, 3);

    match result {
        Some(ref dev) => {
            qemu_ok!("Found device: {0:x}:{1:x} ({2}.{3}.{4})", dev.vendor, dev.device_id, dev.bus, dev.slot, dev.function);
        }
        None => {
            qemu_err!("Device not found!");
            return;
        }
    };

    let device = result.unwrap();

    let base_addr = device.read_bar(0);

    qemu_log!("Base address: {:x}", base_addr);

    unsafe {
        map_pages(
            get_kernel_page_directory(),
            base_addr,
            base_addr,
            PAGE_SIZE,
            PAGE_WRITEABLE | PAGE_CACHE_DISABLE // PAGE_PRESENT is set automatically
        );
    }

    qemu_ok!("Mapped register page!");

    unsafe {
        main_hda_device = Some(HDADevice {
            pcidev: device,
            mmio_base: base_addr
        });
    }

    unsafe { intel_hda_reset_card(); }

    qemu_ok!("Init ok");
}

unsafe fn intel_hda_read(reg: u32) -> u32 {
    return *(main_hda_device.as_ref().unwrap().mmio_base as *const u32).offset(reg as isize);
}

unsafe fn intel_hda_write(reg: u32, value: u32) {
    *(main_hda_device.as_ref().unwrap().mmio_base as *mut u32).offset(reg as isize) = value;
}

unsafe fn intel_hda_reset_card() {
    intel_hda_write(0x8, 0);

    while (intel_hda_read(0x08) & 1) != 0 {};

    intel_hda_write(0x8, 1);

    while (intel_hda_read(0x08) & 1) != 1 {};

    qemu_ok!("OK?");
}

fn intel_hda_generate_command(codec_address: u32, node_index: u32, command: u32, data: u32) -> u32 {
    return (data & 0xff) | (command << 8) | ((node_index & 0xff) << 20) | ((codec_address & 0b1111) << 28)
}
