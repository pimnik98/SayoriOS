// AC'97 driver by NDRAEY (Drew Pavlenko >_)

// FIXME: Partially works in VirtualBox, fully in QEMU
// FIXME: Per-frame lags! 180 ms!

#include <drv/audio/ac97.h>
#include <lib/math.h>
#include <io/ports.h>
#include <drv/pci.h>
#include <common.h>

uint8_t ac97_busnum, ac97_slot, ac97_func;

bool ac97_initialized = false;
bool ac97_varibale_sample_rate = false;

uint8_t ac97_bar0_type = 0;
uint8_t ac97_bar1_type = 0;

size_t* physpages_ac97 = 0;
size_t  currently_pages_count = 0;

uint32_t current_sample_rate = 44100;

// Volume in dB, not % (max 64)
void ac97_set_master_volume(uint8_t left, uint8_t right, bool mute) {
    uint16_t value = (right & 63)
        | ((left & 63) << 8)
        | ((mute) << 15);
       
    outs(native_audio_mixer + NAM_SET_MASTER_VOLUME, value);
}

// Volume in dB, not % (max 32)
void ac97_set_pcm_volume(uint8_t right, uint8_t left, bool mute) {
    uint16_t value = (right & 31)
        | ((left & 31) << 8)
        | ((mute) << 15);
        
    outs(native_audio_mixer + NAM_SET_PCM_VOLUME, value);
}

void ac97_set_pcm_sample_rate(uint32_t sample_rate) {
    if(!ac97_varibale_sample_rate
       || (sample_rate > AC97_MAX_RATE
       	   || sample_rate < AC97_MIN_RATE)
       ) return;
    
    outs(native_audio_mixer + NAM_SAMPLE_RATE, sample_rate);
}

void ac97_reset_channel() {
    outb(native_audio_bus_master + 0x1b, inb(native_audio_bus_master + 0x1B) | 0x02);
}

void ac97_init() {
    // Find device
    pci_find_device(AC97_VENDOR, AC97_DEVICE,
    				&ac97_busnum, &ac97_slot, &ac97_func);

    uint16_t devnum = pci_get_device(ac97_busnum, ac97_slot, ac97_func);

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

    qemu_log("NAM: %x; NABM: %x", native_audio_mixer, native_audio_bus_master);

    uint16_t extended_id = ins(native_audio_mixer + NAM_EXTENDED_ID);

    uint8_t rev = ((extended_id & (3 << 10)) >> 10) & 0b11;
    qemu_log("Codec revision: (%d) %s", rev, 
    		 rev == 0 ? "r < 21" : (
    		 	rev == 1 ? "r22" : (
    		 		rev == 2 ? "r23"
    		 				 : "Not supported"
    		 		)
    		 	)
    		 );

    uint32_t gc = inl(native_audio_bus_master + NABM_GLOBAL_CONTROL);
    qemu_log("Received global control: (%d) %x", gc, gc);
    gc |= 2;  // Cold reset

    uint32_t word = pci_read_confspc_word(ac97_busnum, ac97_slot, ac97_func, 0x3C);  // All 0xF PCI register
    
    qemu_log("AC'97's IRQ is: %d", word & 0xff);

    outl(native_audio_bus_master + NABM_GLOBAL_CONTROL, gc);
    outs(native_audio_mixer + NAM_RESET, 1);
    qemu_log("Cold reset");

    // /* 
    // ac97_global_status_t status;
    // ac97_global_status_t* statusptr = &status;

    uint32_t status = inl(native_audio_bus_master + NABM_GLOBAL_STATUS);
    qemu_log("Status: %x", status);

    // qemu_log("Status: %d (%x)\n", status, status);
    // qemu_log("Status Reserved: %d\n", status.reserved);
    // qemu_log("Status Channels: %d\n", (status.channel==0?2:(status.channel==1?4:(status.channel==2?6:0))));
    // qemu_log("Status Samples: %s\n", status.sample==1?"16 and 20 bits":"only 16 bits");
    
    // */

    uint16_t extended_audio = ins(native_audio_mixer + NAM_EXTENDED_AUDIO);
    qemu_log("Status: %d", extended_audio);

    if((extended_id & 1) > 0) { // Check for variable sample rate
        qemu_log("AC'97 supports variable sample rate!!!");
        extended_audio |= 1;
        ac97_varibale_sample_rate = true;
    }

    outs(native_audio_mixer + NAM_EXTENDED_AUDIO, extended_audio);

    // ac97_set_pcm_sample_rate(ac97_varibale_sample_rate ? 44100 : 48000);
    ac97_set_pcm_sample_rate(44100);

    ac97_set_master_volume(0, 0, false);
    ac97_set_pcm_volume(0, 0, false);

    qemu_log("Updated capabilities");

    ac97_initialized = true;

    currently_pages_count = BUFFER_SIZE / PAGE_SIZE;
    physpages_ac97 = kcalloc(currently_pages_count, sizeof(size_t));

    for(size_t i = 0; i < currently_pages_count; i++) {
        physpages_ac97[i] = alloc_phys_pages(1);

        map_pages(
            get_kernel_dir(),
            physpages_ac97[i],
            physpages_ac97[i],
            1,
            (PAGE_WRITEABLE | PAGE_PRESENT)
        );

        qemu_log("AC97: V: %x; P: %x", physpages_ac97[i], physpages_ac97[i]);
    }

    qemu_log("OK: Page count for 64 KB buffer is: %d", currently_pages_count);
    qemu_log("    Physical pages position: %x", physpages_ac97);

    qemu_log("sizeof(BDL_ENTRY) = %d", sizeof(AC97_BDL_t));

    qemu_log("AC'97 initialized successfully!");
}

bool ac97_is_initialized() {
    return ac97_initialized;
}

size_t ac97_copy_user_memory_to_dma(char* data, size_t length) {
    if(length > BUFFER_SIZE)
        return currently_pages_count;

    for(size_t i = 0; i < currently_pages_count; i++) {
        memcpy(((char*)physpages_ac97[i]), data + (i * PAGE_SIZE), PAGE_SIZE);
    }

    return currently_pages_count;
}

void ac97_destroy_user_buffer() {
    if(!physpages_ac97)
        return;

    for (register size_t i = 0; i < currently_pages_count; i++) {
        unmap_pages(get_kernel_dir(), physpages_ac97[i], 1);
        vmm_free_page(physpages_ac97[i]);
    }
    
    kfree(physpages_ac97);

    currently_pages_count = 0;
    physpages_ac97 = 0;

    qemu_log("Destroyed buffer");
}

void ac97_single_page_write(ssize_t page_num) {
    size_t pgs = MIN(page_num, 32);

    for (register size_t j = 0; j < pgs; j++) {
        ac97_bdl_buffer[j].memory_pos = (size_t)physpages_ac97[j];
        ac97_bdl_buffer[j].sample_count = PAGE_SIZE / 2;
    }

    ac97_bdl_buffer[pgs].flags = 1 << 14;

    ac97_update_bdl();
    ac97_update_lvi(31);
    
    ac97_set_play_sound(true);
    ac97_clear_status_register();
}

// FIXME: Needs optimizations

void ac97_single_page_write_wait(ssize_t page_num) {
    ac97_single_page_write(page_num);

    qemu_log("Polling...");
    
    // uint16_t status = 0;
    // 
    // while(status == 0) {
    //     fast_ins(native_audio_bus_master + NABM_PCM_OUT + 0x06, status);
    // }

    // This also works, but without `sleep_ms` function it skips a lot of frames.
    while(ins(native_audio_bus_master + NABM_PCM_OUT + 0x08) != 0);
    sleep_ms(140);

    // How to fix that?
    /// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

    qemu_log("Ok!");

    ac97_clear_bdl();
}
