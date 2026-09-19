#define VIDEO_ADDRESS 0xB8000
#define MAX_COLS 80
#define MAX_ROWS 25

// Базовые цвета VGA
#define COLOR_BLACK   0x00
#define COLOR_GREEN   0x02
#define COLOR_BLUE    0x01
#define COLOR_CYAN    0x03
#define COLOR_RED     0x04
#define COLOR_MAGENTA 0x05
#define COLOR_WHITE   0x0F

int cursor_x = 0;
int cursor_y = 0;
unsigned char current_color = COLOR_GREEN;

char command_buffer[256];
int command_index = 0;

// Псевдо-рандомное число для игры (генератор на основе системного таймера опроса)
unsigned int pseudo_rand = 12345;

// --- СТРУКТУРЫ ДЛЯ ВТОРОГО ПУНКТА (IDT - ТЕОРИЯ С OSDEV) ---
// Каждая запись в IDT весит ровно 8 байт и описывает, куда прыгать процессору
struct idt_entry_struct {
    unsigned short base_low;  // Младшие 16 бит адреса функции-обработчика
    unsigned short sel;       // Селектор сегмента ядра (обычно 0x08)
    unsigned char  always0;   // Этот байт всегда должен быть равен 0
    unsigned char  flags;     // Флаги доступа (присутствие, уровень прав)
    unsigned short base_high; // Старшие 16 бит адреса функции-обработчика
} __attribute__((packed)); // packed говорит компилятору не делать отступы в памяти

typedef struct idt_entry_struct idt_entry_t;


// --- НИЗКОУРОВНЕВЫЙ ВВОД-ВЫВОД ---
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void outw(unsigned short port, unsigned short data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

void get_cpu_vendor(char* vendor) {
    unsigned int eax, ebx, ecx, edx;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
    for (int i = 0; i < 4; i++) {
        vendor[i]     = (ebx >> (i * 8)) & 0xFF;
        vendor[i + 4] = (edx >> (i * 8)) & 0xFF;
        vendor[i + 8] = (ecx >> (i * 8)) & 0xFF;
    }
    vendor[12] = '\0';
}

// --- УМНЫЙ ДРАЙВЕР ЭКРАНА ---
void scroll() {
    volatile char* video_memory = (volatile char*) VIDEO_ADDRESS;
    for (int y = 1; y < MAX_ROWS; y++) {
        for (int x = 0; x < MAX_COLS * 2; x++) {
            video_memory[((y - 1) * MAX_COLS * 2) + x] = video_memory[(y * MAX_COLS * 2) + x];
        }
    }
    int last_row_offset = (MAX_ROWS - 1) * MAX_COLS * 2;
    for (int x = 0; x < MAX_COLS * 2; x += 2) {
        video_memory[last_row_offset + x] = ' ';
        video_memory[last_row_offset + x + 1] = current_color;
    }
    cursor_y = MAX_ROWS - 1; cursor_x = 0;
}

void clear_screen() {
    volatile char* video_memory = (volatile char*) VIDEO_ADDRESS;
    for (int i = 0; i < MAX_COLS * MAX_ROWS * 2; i += 2) {
        video_memory[i] = ' '; video_memory[i+1] = current_color;
    }
    cursor_x = 0; cursor_y = 0;
}

void print_char(char c) {
    volatile char* video_memory = (volatile char*) VIDEO_ADDRESS;
    if (c == '\n') { cursor_x = 0; cursor_y++; } 
    else {
        int offset = (cursor_y * MAX_COLS + cursor_x) * 2;
        video_memory[offset] = c; video_memory[offset + 1] = current_color;
        cursor_x++;
    }
    if (cursor_x >= MAX_COLS) { cursor_x = 0; cursor_y++; }
    if (cursor_y >= MAX_ROWS) scroll();
}

void print_string(const char* str) {
    int i = 0; while (str[i] != '\0') { print_char(str[i]); i++; }
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Проверяет, начинается ли строка s1 со строки s2 (нужно для команды echo)
int strncmp(const char* s1, const char* s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return 1;
        if (s1[i] == '\0') return 0;
    }
    return 0;
}


// --- ФИЧА: МИНИ-ИГРА "УГАДАЙ ЧИСЛО" ---
void play_guess_game() {
    // Получаем "случайное" число от 1 до 9 на основе нашего счетчика pseudo_rand
    int target = (pseudo_rand % 9) + 1; 
    print_string("Welcome to CoreGuess mini-game!\n");
    print_string("I am thinking of a number from 1 to 9. Try to guess it.\n");
    print_string("Enter your guess (1-9): ");

    // Включаем локальный опрос клавиатуры для игры
    while (1) {
        if (inb(0x64) & 1) {
            unsigned char scancode = inb(0x60);
            if (!(scancode & 0x80)) { // Нажатие
                if (scancode >= 0x02 && scancode <= 0x0A) { // Сканкоды цифр 1-9
                    char guess_char = "123456789"[scancode - 0x02];
                    print_char(guess_char);
                    print_string("\n");

                    int guess = guess_char - '0';
                    if (guess == target) {
                        current_color = COLOR_CYAN;
                        print_string("YOU WIN! Ferdo Studios approves your luck!\n");
                    } else {
                        current_color = COLOR_RED;
                        print_string("WRONG! The number was: ");
                        print_char(target + '0');
                        print_string("\nBetter luck next time!\n");
                    }
                    break; // Выходим из игры
                }
            }
        }
    }
}


// --- КОМАНДНЫЙ ИНТЕРПРЕТАТОР ---
void execute_command() {
    print_string("\n");
    command_buffer[command_index] = '\0';

    if (strcmp(command_buffer, "help") == 0) {
        print_string("Available commands:\n");
        print_string("  help       - Show this message\n");
        print_string("  version    - Show OS version info\n");
        print_string("  clear      - Clear the screen\n");
        print_string("  echo <txt> - Print your text back\n");
        print_string("  guess      - Play 'Guess the number' game\n");
        print_string("  fastfetch  - Show system info and logo\n");
        print_string("  shutdown   - Turn off the system\n");
        print_string("  color green/red/cyan/white - Change text color\n");
    } 
    else if (strcmp(command_buffer, "version") == 0) {
        print_string("Nimbly OS v0.1 [Development Build]\n");
        print_string("Created by Ferdo Studios (c) 2026\n");
    } 
    else if (strcmp(command_buffer, "clear") == 0) {
        clear_screen();
    }
    // ФИЧА: Команда echo
    else if (strncmp(command_buffer, "echo ", 5) == 0) {
        // Выводим строку, пропуская первые 5 символов ("echo ")
        print_string(command_buffer + 5);
        print_string("\n");
    }
    // ФИЧА: Запуск игры
    else if (strcmp(command_buffer, "guess") == 0) {
        play_guess_game();
    }
    else if (strcmp(command_buffer, "fastfetch") == 0) {
        char cpu_vendor[13]; get_cpu_vendor(cpu_vendor);
        unsigned char old_color = current_color;
        print_string("\n");
        current_color = COLOR_RED;     print_string("      ____      "); current_color = old_color; print_string("     safijirk"); current_color = COLOR_WHITE; print_string("@"); current_color = old_color; print_string("NimblyOS-PC\n");
        current_color = COLOR_RED;     print_string("     /\\   \\     "); current_color = old_color; print_string("     -------------------------\n");
        current_color = COLOR_RED;     print_string("    /  \\   \\    "); current_color = COLOR_MAGENTA; print_string("  _|_   "); current_color = old_color; print_string("OS: "); current_color = COLOR_WHITE; print_string("Nimbly OS v0.1 Build 2026\n"); current_color = old_color;
        current_color = COLOR_RED;     print_string("   /____\\___\\  "); current_color = COLOR_MAGENTA; print_string(" / | \\  "); current_color = old_color; print_string("Kernel: "); current_color = COLOR_WHITE; print_string("Nimbly-Core (x86_32)\n"); current_color = old_color;
        current_color = COLOR_MAGENTA; print_string("               |  | |  "); current_color = old_color; print_string("Studio: "); current_color = COLOR_WHITE; print_string("Ferdo Studios\n"); current_color = old_color;
        current_color = COLOR_MAGENTA; print_string("               |__|_|  "); current_color = old_color; print_string("CPU: "); current_color = COLOR_WHITE; print_string(cpu_vendor); print_string(" x86 Processor\n"); current_color = old_color;
        current_color = COLOR_MAGENTA; print_string("               /  /    "); current_color = old_color; print_string("Display: "); current_color = COLOR_WHITE; print_string("VGA Text Mode (80x25)\n"); current_color = old_color;
        current_color = COLOR_MAGENTA; print_string("              /__/     "); current_color = old_color; print_string("Memory: "); current_color = COLOR_WHITE; print_string("16 KB (Kernel Stack Allocated)\n"); current_color = old_color;
        current_color = old_color;
    }
    else if (strcmp(command_buffer, "color green") == 0) { current_color = COLOR_GREEN; print_string("Color changed to Green.\n"); }
    else if (strcmp(command_buffer, "color red") == 0) { current_color = COLOR_RED; print_string("Color changed to Red.\n"); }
    else if (strcmp(command_buffer, "color cyan") == 0) { current_color = COLOR_CYAN; print_string("Color changed to Cyan.\n"); }
    else if (strcmp(command_buffer, "color white") == 0) { current_color = COLOR_WHITE; print_string("Color changed to White.\n"); }
    else if (command_index == 0) {} 
    else {
        print_string("Unknown command: "); print_string(command_buffer); print_string("\nType 'help' for commands list.\n");
    }

    command_index = 0;
    print_string("\nNimblyOS> ");
}

// --- ДРАЙВЕР КЛАВИАТУРЫ ---
const char keyboard_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	
  '9', '0', '-', '=', '\b', '\t',			
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*', 0, ' '	
};

void check_keyboard() {
    if (inb(0x64) & 1) {
        unsigned char scancode = inb(0x60);
        if (!(scancode & 0x80)) {
            char key = keyboard_map[scancode];
            if (key == '\n') { execute_command(); } 
            else if (scancode == 0x0E) { 
                if (command_index > 0) {
                    command_index--; cursor_x--; print_char(' '); cursor_x--;      
                }
            }
            else if (key > 0 && command_index < 255) {
                command_buffer[command_index++] = key; print_char(key);
            }
        }
    }
}

// --- СТАРТ СИСТЕМЫ ---
void kernel_main(void) {
    clear_screen();
    print_string("Welcome to Nimbly OS v0.1!\n");
    print_string("-----------------------------------------\n");
    print_string("IDT Base Structures... LOADED IN MEMORY\n");
    print_string("Type 'guess' to launch game or 'echo hello'.\n");
    print_string("\nNimblyOS> ");

    while (1) {
        check_keyboard();
        
        // Переменная pseudo_rand постоянно мотается в фоне.
        // Так как человек нажимает клавиши в случайные моменты времени,
        // значение этой переменной при нажатии всегда будет случайным! Настоящий рандом без библиотек.
        pseudo_rand++; 
        
        for (volatile int i = 0; i < 50000; i++);
    }
}