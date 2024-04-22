// Intel HD Graphics (8086:2a42) driver by NDRAEY (c) 2024
// WARNING: Driver is in WIP STAGE
// For SayoriOS ;)

#pragma once

#define IGFX_GMBUS0 0x5100
#define IGFX_GMBUS1 0x5104
#define IGFX_GMBUS2 0x5108
#define IGFX_GMBUS3 0x510C
#define IGFX_GMBUS4 0x5110
#define IGFX_GMBUS5 0x5120

#define IGFX_CURACNTR 0x70080

void igfx_init();