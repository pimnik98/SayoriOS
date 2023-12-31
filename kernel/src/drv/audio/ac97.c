// AC'97 driver by NDRAEY (Drew Pavlenko >_)

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
    qemu_log("Status: %x", status);

    // qemu_log("Status: %d (%x)\n", status, status);
    // qemu_log("Status Reserved: %d\n", status.reserved);
    // qemu_log("Status Channels: %d\n", (status.channel==0?2:(status.channel==1?4:(status.channel==2?6:0))));
    // qemu_log("Status Samples: %s\n", status.sample==1?"16 and 20 bits":"only 16 bits");
    
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

    qemu_log("Updated capabilities\n");

    ac97_initialized = true;

    qemu_log("AC'97 initialized successfully!");
}

bool ac97_is_initialized() {
    return ac97_initialized;
}

size_t ac97_copy_user_memory_to_dma(const char* data, size_t length) {
    currently_pages_count = length / PAGE_SIZE;

    ac97_audio_buffer = kmalloc_common(length, PAGE_SIZE);
	ac97_audio_buffer_phys = virt2phys(get_kernel_page_directory(), (virtual_addr_t) ac97_audio_buffer);

	memcpy(ac97_audio_buffer, data, length);

    qemu_log("Made user buffer with %d pages on a board", currently_pages_count);

    return currently_pages_count;
}

void ac97_destroy_user_buffer() {
//    if(!physpages_ac97)
//        return;
//
//    for (size_t i = 0; i < currently_pages_count; i++) {
//        phys_free_single_page(physpages_ac97[i]);
//    }
    
//    memset(physpages_ac97, 0, sizeof(size_t) * currently_pages_count);
//    kfree(physpages_ac97);
    kfree(ac97_audio_buffer);

    currently_pages_count = 0;
//    physpages_ac97 = 0;

    qemu_log("Destroyed buffer");
}

void ac97_single_page_write_wait(size_t page_num) {
	size_t remaining_pages = currently_pages_count - page_num;
    for (size_t j = 0; j < MIN(remaining_pages, 32); j++) {
//        ac97_buffer[j].memory_pos = physpages_ac97[(remaining_pages) + j];
        ac97_buffer[j].memory_pos = (void *) (ac97_audio_buffer_phys + ((page_num + j) * PAGE_SIZE));
        ac97_buffer[j].sample_count = PAGE_SIZE / 2;
    }

    ac97_buffer[remaining_pages >= 31 ? 31 : remaining_pages].flags = 1 << 14;

    ac97_update_bdl();
    ac97_update_lvi(remaining_pages >= 31 ? 31 : remaining_pages);
    
    ac97_set_play_sound(true);
    ac97_clear_status_register();

    while(inb(native_audio_bus_master + 0x16) == 0) {}

    memset(&ac97_buffer, 0, sizeof(AC97_BDL_t)*31);
}

void ac97_test() {
    FILE* file = fopen("R:\\Sayori\\a.wav", "rb");
    fseek(file, 0, SEEK_END);

    const uint32_t filesize = ftell(file);

    fseek(file, 0xae, SEEK_SET);

    char* data = kmalloc(filesize);
    fread(file, filesize, 1, data);

    size_t page_count = ac97_copy_user_memory_to_dma(data, filesize);

    qemu_log("Allocated %d pages for user memory in DMA", page_count);

    ac97_set_master_volume(2, 2, false);
    ac97_set_pcm_volume(2, 2, false);

    for(ssize_t i = 0; i < page_count; i += 32) {
        ac97_single_page_write_wait(i);
    }

    qemu_log("Exiting");
    ac97_reset_channel();

    kfree(data);
    fclose(file);

    ac97_destroy_user_buffer();
}
