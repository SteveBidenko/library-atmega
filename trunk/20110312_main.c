#include <mega16.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include <delay.h>
#include "lcd_4bit.h"
#include "menu.h"
#include "robowater.h"
// DS1820 функции температурного преобразования (Биденко+Панарин)
#include "spd1820.h"
#include "valcoder.h"
// Локальные макроподстановки
#define MAJOR_VERSION 1
#define MINOR_VERSION 28
#define NUM_PARAMETERS 11
// enum
enum en_event {
    e_no_event = 0,
    e_secunda,               // Привязан к prn
    e_enter,
    e_cancel,
    e_valcoder,
    e_timer1_valcoder        //
};
// Определение главных структур
// Описание глобальны переменных
// int tw_prs;             // Заданная температура воды
struct st_datetime s_dt;
struct st_parameter parameters[NUM_PARAMETERS]= {
    {2000, e_temperature, "ЗАД.ТЕМП ", 1},    // [0] Заданная температура воздуха
    {-350, e_temperature, "УЛИЦА ", 0},       // [1] Температура воздуха на улице
    {2400, e_temperature, "ПОМЕЩЕНИЕ ", 0},   // [2] Температура воздуха в помещении
    {7200, e_temperature, "ВОДА ВХ. ", 0},    // [3] Температура воды на входе
    {6500, e_temperature, "ВОДА ВЫХ. ", 0},   // [4] Температура воды на выходе
    {40, e_percent, "ОТКР.КРАН ", 0},         // [5] Текущее состояние крана (АЦП)
    {42, e_percent, "УСТ.КРАН ", 0},          // [6] Заданное состояние крана (расчетное PWM)
    {95, e_percent, "ВЕНТИЛЯТ. ", 0},         // [7] Текущее состояние вентилятора (АЦП)
    {100, e_percent, "УСТ.ВЕНТ. ", 0},        // [8] Заданное состояние вентилятора (расчетное PWM)
    {1, e_clatsman, "ЗАСЛОНКА ", 0},          // [9] Положение воздушной заслонки (0 - открыта, 1 - закрыта)
    {1, e_clatsman, "НАСОС ", 0}              // [10] Работа насоса (1 - вкл., 0 - выкл.)
};
/*struct st_setting parameters[NUM_PARAMETERS]= {
    {2000, e_temperature, "ЗАД.ТЕМП ", 1},    // [0] Заданная температура воздуха
    {-350, e_temperature, "УЛИЦА ", 0},       // [1] Температура воздуха на улице
    {2400, e_temperature, "ПОМЕЩЕНИЕ ", 0},   // [2] Температура воздуха в помещении
    {7200, e_temperature, "ВОДА ВХ. ", 0},    // [3] Температура воды на входе
    {6500, e_temperature, "ВОДА ВЫХ. ", 0},   // [4] Температура воды на выходе
    {40, e_percent, "ОТКР.КРАН ", 0},         // [5] Текущее состояние крана (АЦП)
    {42, e_percent, "УСТ.КРАН ", 0},          // [6] Заданное состояние крана (расчетное PWM)
    {95, e_percent, "ВЕНТИЛЯТ. ", 0},         // [7] Текущее состояние вентилятора (АЦП)
    {100, e_percent, "УСТ.ВЕНТ. ", 0},        // [8] Заданное состояние вентилятора (расчетное PWM)
    {1, e_clatsman, "ЗАСЛОНКА ", 0},          // [9] Положение воздушной заслонки (0 - открыта, 1 - закрыта)
    {1, e_clatsman, "НАСОС ", 0}              // [10] Работа насоса (1 - вкл., 0 - выкл.)
};
*/
struct st_clatsman clatsman = {0, 0, 0, 0};
// Описание функций
void printallterms(void); void lcd_primary_screen(void); void check_serial(void);
// Основная программа
void main(void) {
    int term;
    register byte i;
    init();                  // Инициализация всей периферии
    #asm("sei")             // Global enable interrupts
    printf("Старт программы RoboWater. %u.%02u. Найдено %u термометров.\r\n", MAJOR_VERSION, MINOR_VERSION, ds1820_devices);
    prn = 1;
    if (ds1820_devices) {       // Если есть термометры, то выводим их значение
        for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
        delay_ms (DS1820_ALL_DELAY);
        printallterms();
    }
    // print_all_menu();       // Выведем на отладочную консоль все пункты меню
    lcd_primary_screen();       // выводим стартовую картинку на экранчик
    while(1) {
        check_serial();
        // проверяем и присваиваем биту PORTD.4 состояния PIND.6
        // if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
        if (!VALCODER_ENTER) { delay_ms(30); clatsman.enter = VALCODER_ENTER; }    // Подавление дребезга Enter
        if (!VALCODER_CANCEL) { delay_ms(30); clatsman.cancel = VALCODER_CANCEL; } // Подавление дребезга Cancel
        // if (PINC.6) PORTD |= (1<<4); else PORTD &= ~(1<<4);
        // if (PINC.7) PORTD &= ~(1<<5); else PORTD |= (1<<5);
        // if (PINC.5) PORTD &= ~(1<<5); else PORTD |= (1<<5);
        // Обрабатываем ежесекундное событие.
        if (prn) {
            // Очищаем событие
            prn = 0;
            // Выводим информацию о главном термометре
            if(ds1820_devices){ term = read_term(0); main_menu[0].val_data = term; }
            // Здесь неправильно реализована общая логика программы
            if (clatsman.valcoder_mode == 0) { // Если сейчас не режим valcoder'а, то рисуем стандартные строки
                lcd_primary_screen();          // Рисуем стандартный экранчик
            } else {                           // Обновляем две строки
                // Выводим две строки меню и включаем курсор в зависимости от clatsman.lcdline
                if (clatsman.edit_mode) lcd_edit(0); else lcd_menu(0);
            }
            // Запускаем все термометры на измерение
            for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
        }
        if (clatsman.enter) { clatsman.enter = 0;           // Сразу очищаем событие
            clatsman.valcoder_mode = 1;     // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW;
            if (!clatsman.edit_mode) init_curr_menu(&parameters[0], NUM_PARAMETERS);
        }
        if (clatsman.cancel) { clatsman.cancel = 0;
            clatsman.valcoder_mode = 0;   // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW;
            if (!clatsman.edit_mode) { lcd_clrscr(); init_curr_menu(&main_menu[0], MENU_LEVEL1); }
        }
        // Обрабатываем событие valcoder'а
        if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {
            // Запускаем таймер инактивности valcoder
            timer1_valcoder = VALCODER_TIMER_OVERFLOW;
            // Если перед этим событием не было режима valcoder
            if (clatsman.valcoder_mode == 0) {
                clatsman.valcoder_mode = 1;     // Устанавливаем флаг режима valcoder
                valcoder = VALCODER_NO_ROTATE;  // Выводим меню без изменения позиции
            } else                              // Анализируем, где находится курсор и куда крутим valcoder
                // Если valcoder крутится вправо и курсор был на 1 линии, то ставим ему признак перевода на 2 линию
                if ((valcoder > 0) && (curr_menu.lcd == 0)) { curr_menu.lcd = 1; valcoder = VALCODER_NO_ROTATE; }
                // Если valcoder крутится влево и курсор был на 2 линии, то ставим ему признак перевода на 1 линию
                else if ((valcoder < 0) && (curr_menu.lcd == 1)) { curr_menu.lcd = 0; valcoder = VALCODER_NO_ROTATE; }
            // Обновляем меню
            if (clatsman.edit_mode) lcd_edit(valcoder); else lcd_menu(valcoder);
            // if (valcoder > VALCODER_NO_ROTATE) printf (" -->\r\n"); else printf ("<-- \r\n");
            // printf (" [V = %i, I0 = %u, I1 = %u]\r\n", valcoder, counter0, counter1);
            // Обнуляем все переменные
            valcoder = VALCODER_NO_ROTATE; // counter0 = 0; counter1 = 0;
        }
    };
}
// Печать всех термометров
void printallterms(void) {
    register byte i;
    int term;
    if(!ds1820_devices) return;			// если термометры не обнаружены - просто выходим из функции
    printf("\t");						// печатаем знак табуляции в терминале
    for(i=0; i<ds1820_devices; i++) {
    	term = read_term(i);
        printf(" t%-u = %-i.%-uC; ", i+1, term/100, abs(term%100));
    }
    printf("\r\n");
}
// Чтение температуры (аргумент - номер термометра начиная с 0)
int read_term(byte num) {
    int lterm;
    lterm = ds1820_temperature_10(&ds1820_rom_codes[num][0]);
    return (lterm);
};
// Полностью прорисовать главный экран
void lcd_primary_screen(void) {
    lcd_command(LCD_DISP_ON);       // Убираем курсор с LCD
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 второй строки
    sprintf(linestr, "%02u:%02u:%02u %02u.%02u", s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo);
    lcd_puts(linestr);
    lcd_gotoxy(0,1);                // Устанавливаем курсор в позицию 0 строки 2
    // Выводим информацию о термометрах
    if(ds1820_devices)
        sprintf(linestr, "t=%-i.%-uC ", main_menu[0].val_data/100, abs(main_menu[0].val_data%100));
    else
        sprintf(linestr, "No termometers");
    lcd_puts(linestr);
}
// Обработка событий от серийного порта
void check_serial(void) {
    byte inbyte;    // Описание локальных переменных
    register byte i;
    // Обрабатываем последовательный порт
    if (UCSRA & RX_COMPLETE) { // Пришло ли что-нибудь
        inbyte = UDR;
        switch (inbyte) {
            case 0x7A:                // символ 'z'
                printf("Время: %02u:%02u:%02u, дата:%02u.%02u.%02u, найдено %u термометров\r\n",
                        s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo, s_dt.cyy, ds1820_devices);
                printallterms();
                break;
            case 0x71:                // символ 'q'
                print_curr_menu2(1); break;
            case 0x61:                // символ 'a'
                print_curr_menu2(-1); break;
            case 0x77:                // символ 'w'
                // Входим в меню (эмуляция кнопки Enter)
                // parameters[12].val_data++; if (parameters[12].val_data > 3) parameters[12].val_data = 0;
                // if (++main_menu[1].val_data > 3) main_menu[1].val_data = 0; // [12] Основной режим
                init_curr_menu(&parameters[0], NUM_PARAMETERS);
                break;
            case 0x73:                // символ 's'
                // Выходим из меню (эмуляция кнопки Cancel)
                // parameters[12].val_data--; if (parameters[12].val_data < 0) parameters[12].val_data = 3;
                // if (--main_menu[1].val_data < 0) main_menu[1].val_data = 3; // [12] Основной режим
                // Текущий указатель устанавливаем на главное меню
                init_curr_menu(&main_menu[0], MENU_LEVEL1);
                break;
            case 0x78:                // символ 'x' Печатаем текущее меню
                print_curr_menu(); break;
            case 0x65:                // символ 'e'
                lcd_initedit(0); clatsman.edit_mode = 1;
                break;
            case 0x64:                // символ 'd'
                lcd_initedit(1); clatsman.edit_mode = 0;
                break;
            case 0x63:                // символ 'c'
                lcd_initedit(-1); clatsman.edit_mode = 0;
                break;
            case 0x6D:                // символ 'm'
                for(i=0; i<NUM_PARAMETERS; i++) printf("%s\t", param_str(i, parameters));
                printf("\r\n");
                break;
            case 0x6E:                // символ 'n'
                for(i=0; i<MENU_LEVEL1; i++) printf("%s\t", param_str(i, main_menu));
                printf("\r\n");
                break;
            default:
                printf("Нажат символ 0x%x\r\n", inbyte);
        };
    }
}