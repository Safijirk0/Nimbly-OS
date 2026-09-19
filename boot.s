.set ALIGN,    1<<0             # Выравнивать модули на границах страниц
.set MEMINFO,  1<<1             # Предоставить карту памяти
.set GRAPHICS, 1<<2             # Запросить графический режим VBE
.set FLAGS,    ALIGN | MEMINFO | GRAPHICS
.set MAGIC,    0x1BADB002       # Магическое число Multiboot
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Дополнительные поля для настройки графики (Multiboot Header v1)
.long 0, 0, 0, 0, 0             # Неиспользуемые параметры адресации
.long 0                         # 0 = Линейный графический режим
.long 800                       # Ширина экрана (X)
.long 600                       # Высота экрана (Y)
.long 32                        # Глубина цвета (Бит на пиксель)

.section .bss
.align 16
stack_bottom:
.skip 16384                     # 16 КБ стека ядра
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp        # Инициализируем указатель стека

    # Передаем адрес структуры Multiboot Info (из регистра EBX) в наш kernel_main
    push %ebx                   
    
    .global kernel_main
    call kernel_main

_loop:
    cli
    hlt
    jmp _loop