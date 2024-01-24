.extern     isr_handler
.extern     irq_handler

/* Макрос для обработчика без возврата кода ошибки */
.macro ISR_NOERRCODE isr_num
    .global	isr\isr_num
isr\isr_num:
    cli
    push	$0
    push	$\isr_num
    jmp	isr_common_stub_noerr
.endm

/* Макрос для обработчика прерывания с возвратом кода ошибки */
.macro ISR_ERRCODE isr_num
    .global	isr\isr_num
isr\isr_num:
    cli
    push	$\isr_num
    jmp	isr_common_stub_err
.endm

/* Макрос для враппера обработчика прерывания */
.macro IRQ irq_num, isr_num
    .global irq\irq_num
irq\irq_num:
    cli
    push	$0
    push	$\isr_num
    jmp	irq_common_stub
.endm

/* Обработчики прерываний */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

/* IRQs */
IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

/* Вызов сервиса ОС */
     .global isr80
isr80:
      push  $0
      push  $80
      pusha
      push  %ds
      mov   $0x10, %ax
      mov   %ax, %ds
      call  isr_handler
      pop   %ds
      pop   %edi
      pop   %esi
      pop   %ebp
      add   $4, %esp
      pop   %ebx
      pop   %edx
      pop   %ecx
      add   $12, %esp
      iret

/* Здесь две одинаковые функции (?) */

isr_common_stub_err:
    /* пушим все */
    pusha
    /* сохраняем селектор данных */
    push  %ds
    /* загружаем сегмент данных ядра */
    mov   $0x10, %ax
    mov   %ax, %ds
    /* вызываем наш обработчик */
    call  isr_handler
    /* восстанавливаем селектор данных */
    pop   %ds
    /* восстанавливаем все */
    popa
    /* очищаем стек */
    add   $8, %esp
    /* возврат */
    /* при этом из стека восстанавливаются */
    /* CS, EIP, EFLAGS, SS и ESP */
    iret

isr_common_stub_noerr:
    /* пушим все */
    pusha
    /* сохраняем селектор данных */
    push  %ds
    /* загружаем сегмент данных ядра */
    mov   $0x10, %ax
    mov   %ax, %ds
    /* вызываем наш обработчик */
    call  isr_handler
    /* восстанавливаем селектор данных */
    pop   %ds
    /* восстанавливаем все */
    popa
    /* очищаем стек */
    add   $8, %esp
    /* возврат */
    /* при этом из стека восстанавливаются */
    /* CS, EIP, EFLAGS, SS и ESP */
    iret

/* IRQ common stub */
irq_common_stub:
    pusha
    push  %ds
    mov   $0x10, %ax
    mov   %ax, %ds
    call  irq_handler
    pop   %ds
    popa
    add   $8, %esp
    iret
