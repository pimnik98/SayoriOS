#pragma once

typedef enum {
	OK = 0,
	E_NO_MEMORY,
	E_INVALID_BUFFER,
	E_NO_DEVICE,
	E_DEVICE_NOT_ONLINE,
	E_IO_ERROR,
	E_TIMEOUT,
	E_INVALID_SIZE,
	E_INVALID_SIGNATURE,
	E_NOT_SUPPORTED,
	E_PIKACHU,   // Tsssss it's an easteregg
	E_UNKNOWN
} status_t;
