#pragma once
#include <kernel.h>
#define DEVMGR_VERSION_MAJOR   0
#define DEVMGR_VERSION_MINOR   0
#define DEVMGR_VERSION_PATCH   1

#define DEVMGR_KEY_VENDORID     0
#define DEVMGR_KEY_DEVICEID     1
#define DEVMGR_KEY_COMMAND      2
#define DEVMGR_KEY_STATUS       3
#define DEVMGR_KEY_RIVID        4
#define DEVMGR_KEY_PROGIF       5
#define DEVMGR_KEY_SUBCLASS     6
#define DEVMGR_KEY_CLASS        7
#define DEVMGR_KEY_CACHE        8
#define DEVMGR_KEY_TIMER        9
#define DEVMGR_KEY_HEADER       10
#define DEVMGR_KEY_BIST         11
#define DEVMGR_KEY_BAR0         12
#define DEVMGR_KEY_BAR1         13
#define DEVMGR_KEY_BAR2         14
#define DEVMGR_KEY_BAR3         15
#define DEVMGR_KEY_BAR4         16
#define DEVMGR_KEY_BAR5         17
#define DEVMGR_KEY_INTERRUPT    18
#define DEVMGR_KEY_SECBUS       19
#define DEVMGR_KEY_STATE        20
#define DEVMGR_KEY_CATEGORY     21

typedef struct devmgr_categories {
    uint32_t classID;
    uint32_t subClass;
    uint32_t pifID;
    char nameCat[64];
    char nameSubCat[64];
    char namePif[128];
} devmgr_cat_t;

typedef struct devmgr_device {
    uint32_t vendor;
    uint32_t device;
    uint32_t command;
    uint32_t status;
    uint32_t revID;
    uint32_t progIF;
    uint32_t subClass;
    uint32_t classID;
    uint32_t cache;
    uint32_t timer;
    uint32_t header;
    uint32_t bist;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t interrupt;
    uint32_t secBus;
    uint32_t state;
    uint32_t category;
} devmgr_t;
