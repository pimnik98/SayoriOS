// AC'97 driver by NDRAEY

// FIXME: Not working in VirtualBox, only in QEMU

#include <common.h>
#include <lib/math.h>
#include <drv/audio/ac97.h>
#include "io/ports.h"
#include "drv/pci.h"
#include "mem/pmm.h"
#include "lib/stdio.h"
#include "mem/vmm.h"

uint8_t ac97_busnum, ac97_slot, ac97_func;

bool ac97_initialized = false;
bool ac97_varibale_sample_rate = false;

uint16_t native_audio_mixer = 0;
uint16_t native_audio_bus_master = 0;

uint8_t ac97_bar0_type = 0;
uint8_t ac97_bar1_type = 0;

size_t  currently_pages_count = 0;

uint32_t current_sample_rate = 44100;

__attribute__((aligned(PAGE_SIZE))) AC97_BDL_t ac97_buffer[32];

char* ac97_audio_buffer = 0;
size_t ac97_audio_buffer_phys;

size_t ac97_lvi = 0;

#define AUDIO_BUFFER_SIZE (128 * KB)

// Volume in dB, not % (max 64)
void ac97_set_master_volume(uint8_t left, uint8_t right, bool mute) {
    const uint16_t value = (right & 63) << 0
        | (left & 63) << 8
        | (uint8_t)mute << 15;
    outw(native_audio_mixer + NAM_SET_MASTER_VOLUME, value);
}

// Volume in dB, not % (max 32)
void ac97_set_pcm_volume(uint8_t right, uint8_t left, bool mute) {
    uint16_t value = (right & 31) << 0
        | (left & 31) << 8
        | (mute?1:0) << 15;
    outw(native_audio_mixer + NAM_SET_PCM_VOLUME, value);
}

void ac97_set_pcm_sample_rate(uint16_t sample_rate) {
    if(!ac97_varibale_sample_rate
	|| sample_rate > AC97_MAX_RATE
	|| sample_rate < AC97_MIN_RATE)
		return;
    
    outw(native_audio_mixer + NAM_SAMPLE_RATE, sample_rate);
//    outs(native_audio_mixer + 0x2E, sample_rate);
//    outs(native_audio_mixer + 0x30, sample_rate);
}

//void ac97_load_data(char* data, uint32_t length) {
//    size_t samples = 0xfffe;
//    size_t i;
//    size_t times = MIN(31, length/samples);
//
//    for (i = 0; i < times; i++) {
//        ac97_buffer[i].memory_pos = data + (i * samples);
//        ac97_buffer[i].sample_count = samples;
//    }
//    ac97_buffer[i-1].flags = 1 << 14;  // 14 bit - last entry of buffer, stop playing
//}

void ac97_reset_channel() {
    outb(native_audio_bus_master + 0x1b, inb(native_audio_bus_master + 0x1B) | 0x02);
}

void ac97_clear_status_register() {
    outb(native_audio_bus_master + 0x16, 0x1C);
}

void _ac97_update_bdl(uint32_t address) {
    outl(native_audio_bus_master + NABM_PCM_OUT, address);  // BDL Address register
}

void ac97_update_bdl() {
    _ac97_update_bdl((uint32_t) ac97_buffer);
}

void ac97_update_lvi(uint8_t index) {
    outb(native_audio_bus_master + NABM_PCM_OUT + 0x05, index);
}

void ac97_set_play_sound(bool play) {
//    outb(native_audio_bus_master + 0x1b, inb(native_audio_bus_master + 0x1B) | (uint8_t)play);
    outb(native_audio_bus_master + 0x1b, (uint8_t)play);
}

void ac97_init() {
    // Find device
    pci_find_device(AC97_VENDOR, AC97_DEVICE, &ac97_busnum, &ac97_slot, &ac97_func);

    const uint16_t devnum = pci_get_device(ac97_busnum, ac97_slot, ac97_func);

    qemu_log("AC'97 ID: %d (%x)", devnum, devnum);

    if(devnum == PCI_VENDOR_NO_DEVICE) {
        qemu_log("AC'97 not connected!");
        return;
    }else{
        qemu_log("Detected AC'97");
    }

    // Enable IO Busmastering

    uint16_t command_register = pci_read_confspc_word(ac97_busnum, ac97_slot, ac97_func, 4);
    qemu_log("Command register is: %x", command_register);

    command_register |= 0x05;

    qemu_log("Command register now is: %x", command_register);
    pci_write(ac97_busnum, ac97_slot, ac97_func, 4, command_register);

    // Get NAM and NABM adresses for port i/o.

    native_audio_mixer = pci_read_confspc_word(ac97_busnum, ac97_slot, ac97_func, 0x10);  // BAR0
    native_audio_bus_master = pci_read_confspc_word(ac97_busnum, ac97_slot, ac97_func, 0x14); // BAR1

    // That's QEMU BUG?
    native_audio_mixer--;
    native_audio_bus_master--;

    // uint8_t hdrtype = pci_get_hdr_type(ac97_busnum, ac97_slot, ac97_func);
    // native_audio_mixer = pci_get_bar(hdrtype, ac97_busnum, ac97_slot, ac97_func, 0, ac97_bar0_type);  // BAR0
    // native_audio_bus_master = pci_get_bar(hdrtype, ac97_busnum, ac97_slot, ac97_func, 1, ac97_bar1_type); // BAR1

    qemu_log("NAM: %x; NABM: %x", native_audio_mixer, native_audio_bus_master);

    const uint16_t extended_id = inw(native_audio_mixer + NAM_EXTENDED_ID);

    const size_t rev = (extended_id >> 10) & 0b11;
    const char* rev_strs[] = {"r < 21", "r22", "r23"};
    qemu_log("Codec revision: (%d) %s", rev, rev_strs[rev]);

    uint32_t gc = inl(native_audio_bus_master + NABM_GLOBAL_CONTROL);
    qemu_log("Received global control: (%d) %x", gc, gc);
    gc |= 1 << 1;  // Cold reset
    outl(native_audio_bus_master + NABM_GLOBAL_CONTROL, gc);
    outw(native_audio_mixer + NAM_RESET, 1);
    qemu_log("Cold reset");

    // /* 
    // ac97_global_status_t status;
    // ac97_global_status_t* statusptr = &status;

    const uint32_t status = inl(native_audio_bus_master + NABM_GLOBAL_STATUS);

    qemu_log("Status: %d (%x)\n", status, status);
//    qemu_log("Status Reserved: %d\n", status.reserved);
//    qemu_log("Status Channels: %d\n", (status.channel==0?2:(status.channel==1?4:(status.channel==2?6:0))));
//    qemu_log("Status Samples: %s\n", status.sample==1?"16 and 20 bits":"only 16 bits");
    
    // */

    uint16_t extended_audio = inw(native_audio_mixer + NAM_EXTENDED_AUDIO);
    qemu_log("Status: %d", extended_audio);

    if((extended_id & 1) != 0) { // Check for variable sample rate
        qemu_log("AC'97 supports variable sample rate!!!\n");
        extended_audio |= 1;
        ac97_varibale_sample_rate = true;
    }

    outw(native_audio_mixer + NAM_EXTENDED_AUDIO, extended_audio);

    ac97_set_pcm_sample_rate(ac97_varibale_sample_rate ? 44100 : 48000);

    ac97_set_master_volume(0, 0, false);
    ac97_set_pcm_volume(0, 0, false);

    ac97_audio_buffer = kmalloc_common(AUDIO_BUFFER_SIZE, PAGE_SIZE);
    ac97_audio_buffer_phys = virt2phys(get_kernel_page_directory(),
                                       (virtual_addr_t)ac97_audio_buffer);

    qemu_log("Updated capabilities\n");

    ac97_FillBDLs();

    ac97_initialized = true;

    qemu_log("AC'97 initialized successfully!");
}

bool ac97_is_initialized() {
    return ac97_initialized;
}

void ac97_FillBDLs() {
    size_t sample_divisor = 2;
    // We need to fill ALL BDL entries.
    // If we don't do that, we can encounter lags and freezes, because DMA doesn't stop
    // even when its read pointer reached the end marker. (It will scroll to the end)
    // So we need to spread buffer on BDL array

    size_t bdl_span = AUDIO_BUFFER_SIZE / 32; // It's the size of each transfer (32 is the count of BDLs)

    size_t filled = 0;
    for (size_t j = 0; j < AUDIO_BUFFER_SIZE; j += bdl_span) {
        ac97_buffer[filled].memory_pos = (void*)(ac97_audio_buffer_phys + j);
        ac97_buffer[filled].sample_count = (bdl_span / sample_divisor) + 2;

//            LOG("[%d] %x; %x", filled, ac97_data_buffer.second[filled].memory_pos, ac97_data_buffer.second[filled].sample_count);
        filled++;
    }

    qemu_log("Fills: %d", filled);

    filled--;

    ac97_buffer[filled].flags = (1 << 14) | (1 << 15);

    ac97_update_bdl();
    ac97_update_lvi(filled);

    ac97_lvi = filled;
}

void ac97_WriteAll(void* buffer, size_t size) {
    qemu_log("Start");

    size_t loaded = 0;

    for(; loaded < size; loaded += AUDIO_BUFFER_SIZE) {
        size_t block_size = MIN(size - loaded, AUDIO_BUFFER_SIZE);

        memcpy(ac97_audio_buffer,
                     (char*)buffer + loaded,
                     block_size);

        if (block_size < AUDIO_BUFFER_SIZE) {
            memset((char *) ac97_audio_buffer + block_size,
                     0,
                     AUDIO_BUFFER_SIZE - block_size);
        }

        ac97_update_lvi(ac97_lvi);

        ac97_set_play_sound(true);
        ac97_clear_status_register();

        while ((inb(native_audio_bus_master + 0x16) & (1 << 1)) == 0) {
            __asm__ volatile("nop");
        }
    }

    qemu_log("Finish");
}


void ac97_test() {
    FILE* file = fopen("R:\\Sayori\\a.wav", "rb");
    fseek(file, 0, SEEK_END);

    const uint32_t filesize = ftell(file);

    fseek(file, 0xae, SEEK_SET);

    char* data = kmalloc(filesize);
    fread(file, filesize, 1, data);

//    size_t page_count = ac97_copy_user_memory_to_dma(data, filesize);

//    qemu_log("Allocated %d pages for user memory in DMA", page_count);

    ac97_set_master_volume(2, 2, false);
    ac97_set_pcm_volume(2, 2, false);

    ac97_WriteAll(data, filesize);

    qemu_log("Exiting");
    ac97_reset_channel();

    kfree(data);
    fclose(file);

//    ac97_destroy_user_buffer();
}
