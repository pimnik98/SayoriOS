/**
 * @defgroup Драйвер PCI (Peripheral Component Interconnect)
 * @file drv/pci.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), Арен Елчинян (SynapseOS)
 * @brief Драйвер PCI (Peripheral Component Interconnect)
 * @version 0.3.4
 * @date 2023-01-14
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */

#pragma once

#include <common.h>

#define CLASS_DEVICE_TOO_OLD 0x00   ///! Устройство не имеет идентификатора класса, возможно, оно было сделано до его определения
#define CLASS_MASS_STORAGE 0x01     /// Контроллер запоминающего устройства
#define CLASS_NETWORK_CTRLR 0x02    /// Сетевой контроллер
#define CLASS_DISPLAY 0x03          /// Контроллер дисплея
#define CLASS_MULTIMEDIA 0x04       /// Мультимедийный контроллер
#define CLASS_MEMORY 0x05           /// Контроллер памяти
#define CLASS_BRIDGE 0x06           /// Мост
#define CLASS_SCC 0x07              /// Простой коммуникационный контроллер
#define CLASS_SYSTEM 0x08           /// Периферийное устройство базовой системы
#define CLASS_INPUT 0x09            /// Контроллер устройства ввода
#define CLASS_DOCK 0x0A             /// Док-станция
#define CLASS_PROCESSOR 0x0B        /// Процессор
#define CLASS_SERIAL_BUS 0x0C       /// Контроллер последовательной шины
#define CLASS_WIRELESS 0x0D         /// Беспроводной контроллер
#define CLASS_INTELLIGENTIO 0x0E    /// Интеллектуальный контроллер
#define CLASS_SATELLITE  0x0F       /// Контроллер спутниковой связи
#define CLASS_ENCRYPT 0x10          /// Контроллер шифрования
#define CLASS_SIGNAL_PROC 0x11      /// Контроллер обработки сигналов
#define CLASS_PROC_ACCEL 0x12       /// Ускоритель обработки
#define CLASS_NO_ESS_INS 0x13       /// Второстепенная аппаратура
#define CLASS_CO_CPU 0x40           /// Со-Процессор

#define PCI_ADDRESS_PORT 0xCF8      /// Точка входа || Адрес конфигурации, который требуется для доступа
#define PCI_DATA_PORT 0xCFC         /// Пароль входа || Генерирует доступ к конфигурации и будет передавать данные в или из регистра

#define PCI_VENDOR_NO_DEVICE 0xFFFF /// Устройство не найдено

/**
 * @brief Структура устройства
 */
typedef struct pci_header_t {
	uint16_t vendor_id;             /// ID-Поставщика
	uint16_t device_id;             /// ID-Устройства
	uint8_t revision;               /// ID-Реверсии
	uint8_t prog_if;                /// Положение дел (???) Prog IF
	uint8_t subclass_id;            /// Подкатегория устройства
	uint8_t class_id;               /// Категория устройства
	uint8_t cache_line_size;        /// Размер строки кэша
	uint8_t latency_timer;          /// Таймер задержки
	uint8_t hdr_type;               /// Тип заголовка (???)
	uint8_t bist;                   /// БИСТ (???) BIST
	uint32_t bar[6];                /// ???
	uint32_t cardbus_cis_ptr;       /// Базовый адрес CardBus Socket/ExCa
	uint16_t subsys_vendor;         /// ???
	uint16_t subsys_id;             /// ???
	uint32_t expansion_rom;         /// ???
	uint8_t capatibilities;         /// ???
	uint8_t reserved[3];            /// Зарезервированный
	uint32_t reserved2;             /// Зарезервированный
	uint8_t int_line;               /// Линия прерывания
	uint8_t int_pin;                /// ПИН-код прерывания
	uint8_t min_grant;              /// ???
	uint8_t max_latency;            /// ???
} pci_header_t;

uint16_t pci_read_confspc_word(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
uint8_t pci_get_class(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pci_get_subclass(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pci_get_hdr_type(uint8_t bus, uint8_t slot, uint8_t function);
uint16_t pci_get_vendor(uint8_t bus, uint8_t slot, uint8_t function);
uint16_t pci_get_device(uint8_t bus, uint8_t slot, uint8_t function);
const char *pci_get_device_type(uint8_t klass, uint8_t subclass);
const char *pci_get_vendor_name(uint16_t vendor);
uint32_t pci_get_bar(uint8_t hdrtype, uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_number, uint8_t *bar_type);
void pci_find_device(uint16_t vendor, uint16_t device, uint8_t *bus_ret, uint8_t *slot_ret, uint8_t *func_ret);
void pci_print_list();
void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint32_t offset, uint32_t value);
void pci_find_device_by_class_and_subclass(uint16_t class, uint16_t subclass, uint16_t *vendor_ret, uint16_t *deviceid_ret,
									  uint8_t *bus_ret, uint8_t *slot_ret, uint8_t *func_ret);
uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);