// AC'97 driver by NDRAEY (Drew Pavlenko >_)

// FIXME: Not working in VirtualBox, only in QEMU

#include <kernel.h>
#include <lib/math.h>
#include <drv/audio/ac97.h>
#include <common.h>

uint8_t ac97_busnum, ac97_slot, ac97_func;

bool ac97_initialized = false;
bool ac97_varibale_sample_rate = false;

uint16_t native_audio_mixer = 0;
uint16_t native_audio_bus_master = 0;

uint8_t ac97_bar0_type = 0;
uint8_t ac97_bar1_type = 0;

size_t* physpages_ac97;
size_t  currently_pages_count = 0;

uint32_t current_sample_rate = 44100;

AC97_BDL_t ac97_bdl_buffer[32];

#define AC97_BUFFER_BASE 0xE0000000

// Volume in dB, not % (max 64)
void ac97_set_master_volume(uint8_t left, uint8_t right, bool mute) {
    uint16_t value = ((right & 63) << 0)
        | ((left & 63) << 8)
        | ((mute ? 1 : 0) << 15);
    outs(native_audio_mixer + NAM_SET_MASTER_VOLUME, value);
}

// Volume in dB, not % (max 32)
void ac97_set_pcm_volume(uint8_t right, uint8_t left, bool mute) {
    uint16_t value = ((right & 31) << 0)
        | ((left & 31) << 8)
        | ((mute?1:0) << 15);
    outs(native_audio_mixer + NAM_SET_PCM_VOLUME, value);
}

void ac97_set_pcm_sample_rate(uint32_t sample_rate) {
    if(!ac97_varibale_sample_rate || (sample_rate > AC97_MAX_RATE || sample_rate < AC97_MIN_RATE)) return;
    
    outs(native_audio_mixer + NAM_SAMPLE_RATE, sample_rate);
}

void ac97_reset_channel() {
    outb(native_audio_bus_master + 0x1b, inb(native_audio_bus_master + 0x1B) | 0x02);

    microseconds_delay(100);
}

void ac97_clear_status_register() {
    outb(native_audio_bus_master + 0x16, 0x1C);
}

void _ac97_update_bdl(uint32_t address) {
    outl(native_audio_bus_master + NABM_PCM_OUT, address);  // BDL Address register
}

void ac97_update_bdl() {
    _ac97_update_bdl((uint32_t)&ac97_bdl_buffer);
}

void ac97_update_lvi(uint8_t index) {
    outb(native_audio_bus_master + NABM_PCM_OUT + 0x05, index);
}

void ac97_set_play_sound(bool play) {
    outb(native_audio_bus_master + 0x1b, inb(native_audio_bus_master + 0x1B) | (play?0x01:0x00));
}

void ac97_init() {
    // Find device
    pci_find_device(AC97_VENDOR, AC97_DEVICE, &ac97_busnum, &ac97_slot, &ac97_func);

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

    // uint8_t hdrtype = pci_get_hdr_type(ac97_busnum, ac97_slot, ac97_func);
    // native_audio_mixer = pci_get_bar(hdrtype, ac97_busnum, ac97_slot, ac97_func, 0, ac97_bar0_type);  // BAR0
    // native_audio_bus_master = pci_get_bar(hdrtype, ac97_busnum, ac97_slot, ac97_func, 1, ac97_bar1_type); // BAR1

    qemu_log("NAM: %x; NABM: %x", native_audio_mixer, native_audio_bus_master);

    uint16_t extended_id = ins(native_audio_mixer + NAM_EXTENDED_ID);

    uint8_t rev = ((extended_id & (3 << 10)) >> 10) & 0b11;
    qemu_log("Codec revision: (%d) %s", rev, rev==0?"r < 21":(rev==1?"r22":(rev==2?"r23":"Not supported")));

    uint32_t gc = inl(native_audio_bus_master + NABM_GLOBAL_CONTROL);
    qemu_log("Received global control: (%d) %x", gc, gc);
    gc |= 1 << 1;  // Cold reset
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
        qemu_log("AC'97 supports variable sample rate!!!\n");
        extended_audio |= 1;
        ac97_varibale_sample_rate = true;
    }

    outs(native_audio_mixer + NAM_EXTENDED_AUDIO, extended_audio);

    // ac97_set_pcm_sample_rate(ac97_varibale_sample_rate ? 44100 : 48000);
    ac97_set_pcm_sample_rate(44100);

    ac97_set_master_volume(0, 0, false);
    ac97_set_pcm_volume(0, 0, false);

    qemu_log("Updated capabilities\n");

    ac97_initialized = true;

    qemu_log("AC'97 initialized successfully!");
}

bool ac97_is_initialized() {
    return ac97_initialized;
}

size_t ac97_copy_user_memory_to_dma(char* data, size_t length) {
    currently_pages_count = length / PAGE_SIZE;

    physpages_ac97 = kcalloc(currently_pages_count, sizeof(size_t));

    for(size_t i = 0; i < currently_pages_count; i++) {
        size_t offset = i * PAGE_SIZE;

        // Why it produces WhItE NoIsE???
        // I checked the data with qemu's qmp and found data the same in both physical and virtual addresses!!!

        // physpages_ac97[i] = virt2phys(get_kernel_dir(), (size_t)(data + offset));
        // qemu_log("AC97: %x => %x", (size_t)(data + offset), physpages_ac97[i]);
        
        physpages_ac97[i] = alloc_phys_pages(1);

        map_pages(
            get_kernel_dir(),
            // AC97_BUFFER_BASE + offset,
            physpages_ac97[i] + offset,
            physpages_ac97[i],
            1,
            (PAGE_WRITEABLE | PAGE_PRESENT)
        );

        // memcpy(AC97_BUFFER_BASE + offset, data + offset, PAGE_SIZE);
        memcpy(physpages_ac97[i] + offset, data + offset, PAGE_SIZE);

        // qemu_log("Virtual: %x Physical: %x", 0xE0000000+(i*PAGE_SIZE), physpages_ac97[i]);
    }

    qemu_log("Made user buffer with %d pages on a board", currently_pages_count);

    return currently_pages_count;
}

void ac97_destroy_user_buffer() {
    if(!physpages_ac97)
        return;

    for (size_t i = 0; i < currently_pages_count; i++) {
        unmap_pages(get_kernel_dir(), physpages_ac97[i], 1);
        vmm_free_page(physpages_ac97[i]);
    }
    
    // memset(physpages_ac97, 0, sizeof(size_t) * currently_pages_count);
    kfree(physpages_ac97);

    currently_pages_count = 0;
    physpages_ac97 = 0;

    qemu_log("Destroyed buffer");
}

void ac97_single_page_write_wait(ssize_t page_num) {
    size_t pgs = MIN(page_num, 32);

    for (size_t j = 0; j < pgs; j++) {
        char* pos = physpages_ac97[(currently_pages_count - page_num) + j];
        
        // qemu_log("Memory: %x", pos);
           
        ac97_bdl_buffer[j].memory_pos = pos;
        ac97_bdl_buffer[j].sample_count = PAGE_SIZE / 2;
        // ac97_bdl_buffer[j].sample_count = PAGE_SIZE;
    }

    ac97_bdl_buffer[(page_num > 32 ? 31 : page_num)].flags = 1 << 14;

    // ac97_reset_channel();
    ac97_update_bdl();
    ac97_update_lvi(31);
    
    ac97_set_play_sound(true);
    ac97_clear_status_register();

    while(inb(native_audio_bus_master + 0x16) == 0) {}

    memset(&ac97_bdl_buffer, 0, sizeof(AC97_BDL_t)*31);
}

void ac97_test() {
    FILE* file = fopen("/sound.wav", "rb");
    fseek(file, 0, SEEK_END);

    uint32_t filesize = ftell(file);

    fseek(file, 64, SEEK_SET);

    char* data = kcalloc(filesize + 1, sizeof(char));
    fread_c(file, filesize, 1, data);

    qemu_log("Data (%x) starts with: %x %x %x %x", data, data[0], data[1], data[2], data[3]);
    qemu_log("Physical address: %x", virt2phys(get_kernel_dir(), data));

    size_t page_count = ac97_copy_user_memory_to_dma(data, filesize);

    qemu_log("Allocated %d pages for user memory in DMA", page_count);


    ac97_set_master_volume(2, 2, false);
    ac97_set_pcm_volume(2, 2, false);

    for(ssize_t i = page_count; i > 0; i-= 32) {
        // qemu_log("%d", MIN(i, 32));

        ac97_single_page_write_wait(i);
    }

    qemu_log("Exiting");
    ac97_reset_channel();

    kfree(data);
    fclose(file);

    ac97_destroy_user_buffer();
}
