#ifndef SAYORI_JSE_EVENT_H
#define SAYORI_JSE_EVENT_H

typedef struct  JSE_EVENT_KBD_KEY_STATE_t {
    int8_t Status;            ///< Статус
    uint32_t Start;           ///< Когда нажато
    uint32_t End;             ///< Когда отпущенно
    uint32_t Last;            ///< Последний вызов
} JSE_EVENT_KBD_KEY_STATE;

#endif //SAYORI_JSE_EVENT_H
