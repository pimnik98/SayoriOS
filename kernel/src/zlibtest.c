#include "../include/compress/zlib/zlib.h"
#include "io/ports.h"

#define MAX_BUFFER_SIZE 1024

void compress_string(const char* input, char* output, unsigned int* output_length) {
    z_stream stream;

    memset(&stream, 0, sizeof(stream));
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        qemu_err("Failed to initialize deflate stream\n");
        return;
    }

    stream.avail_in = strlen(input);
    stream.next_in = (unsigned char*)input;

    unsigned int total_output = 0;

    do {
        stream.avail_out = MAX_BUFFER_SIZE;
        stream.next_out = (unsigned char*)output + total_output;

        if (deflate(&stream, Z_FINISH) == Z_STREAM_ERROR) {
            qemu_err("Compression error\n");
            deflateEnd(&stream);
            return;
        }

        total_output += MAX_BUFFER_SIZE - stream.avail_out;
    } while (stream.avail_out == 0);

    *output_length = total_output;
    deflateEnd(&stream);
}

void decompress_buffer(const char* input, unsigned int input_length, char* output, unsigned int* output_length) {
    z_stream stream;

    memset(&stream, 0, sizeof(stream));
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (inflateInit(&stream) != Z_OK) {
        qemu_err("Failed to initialize inflate stream\n");
        return;
    }

    stream.avail_in = input_length;
    stream.next_in = (unsigned char*)input;

    unsigned int total_output = 0;

    do {
        stream.avail_out = MAX_BUFFER_SIZE;
        stream.next_out = (unsigned char*)output + total_output;

        if (inflate(&stream, Z_NO_FLUSH) == Z_STREAM_ERROR) {
            qemu_err("Decompression error\n");
            inflateEnd(&stream);
            return;
        }

        total_output += MAX_BUFFER_SIZE - stream.avail_out;
    } while (stream.avail_out == 0);

    *output_length = total_output;
    inflateEnd(&stream);
}

int test_zlib() {
    const char* input = "PIKACHU";
    qemu_printf("Original string: %s\n", input);

    char compressed_buffer[MAX_BUFFER_SIZE];
    unsigned int compressed_length = 0;

    compress_string(input, compressed_buffer, &compressed_length);

    qemu_printf("Compressed buffer (length=%d):\n", compressed_length);
    for (unsigned int i = 0; i < compressed_length; i++)
        new_qemu_printf("%02x ", compressed_buffer[i]);
    qemu_printf("\n");

    char decompressed_buffer[MAX_BUFFER_SIZE];
    unsigned int decompressed_length = 0;

    decompress_buffer(compressed_buffer, compressed_length, decompressed_buffer, &decompressed_length);

    qemu_printf("Decompressed buffer (length=%u): %s\n", decompressed_length, decompressed_buffer);

    return 0;
}
