#ifndef SAYORI_VIRTIO_NETWORK_H

#pragma once

#include <common.h>

#define VIO_NET_VENDOR 0x1AF4
#define VIO_NET_DEVICE 0x1000

enum VIO_NAT_IOREGS {
    VIO_NAT_IDR_DF   = 0x00, // Особенности устройства
    VIO_NAT_IDR_GF   = 0x04, // Возможности для гостевой ОС
    VIO_NAT_IDR_QA   = 0x08, // Адрес очереди
    VIO_NAT_IDR_QL   = 0x0C, // Размер очереди
    VIO_NAT_IDR_QS   = 0x0E, // Выбор очереди
    VIO_NAT_IDR_QN   = 0x10, // Уведомление об очереди
    VIO_NAT_IDR_DS   = 0x12, // Состояние устройства
    VIO_NAT_IDR_IS   = 0x13, // Статус ISR
};

enum VIO_NAT_NDREGS {
    VIO_NAT_NDR_MAC1P   = 0x14, // MAC-адрес (0-4) 1я часть
    VIO_NAT_NDR_MAC2P   = 0x18, // MAC-адрес (5-6) 2я часть
    VIO_NAT_NDR_STATUS  = 0x1A  // Статус
};

enum VIO_NAT_BDREGS {
    VIO_NAT_BDR_TSC   = 0x14, // Общее количество секторов
    VIO_NAT_BDR_MSS   = 0x1C, // Максимальный размер сегмента
    VIO_NAT_BDR_MSC   = 0x20, // Максимальное количество сегментов
    VIO_NAT_BDR_CC    = 0x24, // Число цилиндров
    VIO_NAT_BDR_HC    = 0x26, // Подсчет голов
    VIO_NAT_BDR_SC    = 0x27, // Количество секторов
    VIO_NAT_BDR_BL    = 0x28  // Длина блока
};

#define SAYORI_VIRTIO_NETWORK_H

#endif //SAYORI_VIRTIO_NETWORK_H
