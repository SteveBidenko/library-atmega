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
#include "at2404.h"
// Локальные макроподстановки
#define MAJOR_VERSION 1
#define MINOR_VERSION 30
// enum
// Определение главных структур
// Описание глобальны переменных
// int tw_prs;             // Заданная температура воды
// TW_in_Min -Температура Воды на входе( Подаче ) мин.
// TW_out_Min -Температура Воды на выходе ( Обратка ) мин.
// TW_out_Stop -Температура Воды на входе для поддержания в режиме стоп.
// TA_in_Min -Температура Воздуха на входе мин -30 С.
// TA_out_Min -Температура Воздуха на выходе мин +15 С.
// TA_out_prs -Температура Воздуха на выходе установленная +20 С.(Заданная)


struct st_datetime s_dt;
struct st_mode mode = {0, 0, 0, 1};           // Текущий режим работы
struct st_prim_par {
    byte PWM_out1, PWM_out2, ADC1, ADC2;
    int Tw1, Tw2, Ta1, Ta2, TW_in_Min, TW_out_Min, TW_out_Stop, TA_in_Min, TA_out_Min,TA_out_prs; 
} prim_par={
    55, 15, 30, 30,
    99, 1, 2, 3, 75, 15, 30, 27, 15, 20
};
enum en_event event;                          // Текущее событие в системе
// Описание функций
void printallterms(void); void lcd_primary_screen(void); void check_serial(void);
// Основная программа
void main(void) {
    // register byte i;
    // struct st_prim_par *ppr_par;
    byte size_prim_par;
    init();                  // Инициализация всей периферии
    #asm("sei")             // Global enable interrupts
    printf("Старт программы RoboWater. %u.%02u. Найдено %u термометров.\r\n", MAJOR_VERSION, MINOR_VERSION, ds1820_devices);
    // Если есть термометры, то выводим их значение
    printallterms();
    // print_all_menu();       // Выведем на отладочную консоль все пункты меню
    // Сохраняем в EEPROM структуру prim_par
    // ppr_par = &prim_par; 
    size_prim_par = sizeof(prim_par);  
    eeprom_write_struct ((char *)&prim_par, size_prim_par);
    // printf("До записи в EEPROM (%u bytes) значение Tw2=%u ... ", size_prim_par, prim_par.Tw2); 
    // prim_par.Tw2 = 99;
    // eeprom_write(0, size_prim_par);
    // size_prim_par = eeprom_read(0);
    // printf("Изменяем значение значение Tw2=%u ...", prim_par.Tw2); 
    // Восстанавливаем из EEPROM структуру prim_par
    eeprom_read_struct ((char *)&prim_par, size_prim_par);
    printf("После чтение из EEPROM значение Tw2=%u\r\n", prim_par.Tw2); 
    // printf("Было %u, стало после чтение из EEPROM size=%u\r\n", sizeof(prim_par), size_prim_par); 
    /*
    // Необходима до старта основного цикла реализовать ручную коррекцию даты и времени с автоматическим выходом по таймеру
    timer1_valcoder = 60;       // Установка таймера 30 сек
    while(timer1_valcoder) {
         lcd_command(LCD_DISP_ON);       // Убираем курсор с LCD
         lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 первой строки
         sprintf(linestr, "Уст.времени");
    }
    */
    lcd_primary_screen();       // выводим стартовую картинку на экранчик
    // if (PINC.6) PORTD |= (1<<4); else PORTD &= ~(1<<4);
    // if (PINC.7) PORTD &= ~(1<<5); else PORTD |= (1<<5);
    // if (PINC.5) PORTD &= ~(1<<5); else PORTD |= (1<<5);
    while(1) {
        check_serial();
        // проверяем и присваиваем биту PORTD.4 состояния PIND.6
        // if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
        if (event == ev_none) {
            if (!VALCODER_ENTER) {
                // Подавление дребезга Enter
                delay_ms(100);
                if (!VALCODER_ENTER) {  // printf ("Сгенерировали нажатие Enter\r\n");
                    event = ev_enter;
                    // Запускаем таймер инактивности
                    timer1_valcoder = TIMER_INACTIVE;                }
            }
            if (!VALCODER_CANCEL) {
                // Подавление дребезга Cancel
                delay_ms(50);
                if (!VALCODER_CANCEL) { // printf ("Сгенерировали нажатие Cancel\r\n");
                    event = ev_cancel;
                    // Запускаем таймер инактивности
                    timer1_valcoder = TIMER_INACTIVE;                }
            }
            // Обрабатываем событие valcoder'а
            if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {        // Если сработал valcoder
                // printf ("Сгенерировали кручение (%i)...\r\n", valcoder);
                if (valcoder < 0) event = ev_left;
                else event = ev_right;
                // Запускаем таймер инактивности
                timer1_valcoder = TIMER_INACTIVE;
                valcoder = VALCODER_NO_ROTATE;
                // if (valcoder > VALCODER_NO_ROTATE) printf (" -->\r\n"); else printf ("<-- \r\n");
                // printf (" [V = %i, I0 = %u, I1 = %u]\r\n", valcoder, counter0, counter1);
            }
        }
        switch (event) {
            case ev_secunda:                // Обрабатываем ежесекундное событие.
                main_menu[0].val_data = read_term(0);       // Выводим информацию о главном термометре
                switch (mode.menu) {
                    case 0: lcd_primary_screen(); break;
                    case 1: lcd_menu(0); break;
                    default: ;
                    // case 2: lcd_edit(0); break;
                };
                event = ev_none;            // Очищаем событие
                break;
            case ev_left:                   // printf ("Обрабатываем прокрутку valcoder влево\r\n");
            case ev_right:                  // printf ("Обрабатываем прокрутку valcoder вправо\r\n");
                // printf ("Обрабатываем прокрутку valcoder (%d), в режиме %d - ", event-2, mode.menu);
                switch (mode.menu) {
                    case 0: lcd_menu(mode.menu++); break;   // Выводим меню без изменения позиции printf ("entering...");
                    case 1: lcd_menu(event-2); break;       // printf ("navigating...");
                    case 2: lcd_edit(event-2); break;       // printf ("editing...");
                    default: ;                              // printf ("defaulting...");
                }
                // printf ("\r\n");
                event = ev_none;            // Очищаем событие
                break;
            case ev_enter:                  // Если нажат Enter
                // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW; if (mode.menu <= 2)
                switch (mode.menu) {
                    // lcd_primary_screen();
                    // Обрабатываем нажатие enter c учетом того, что значение mode.menu еще старое
                    case 0: lcd_menu(0); ++mode.menu; break;    // Если находились в главном экране, запускаем прорисовку меню
                    case 1:                                     // Если находились в меню, то анализируем
                            lcd_initedit(0); break;
                    case 2: lcd_initedit(1); mode.menu = 1; break;
                };
                event = ev_none;            // Очищаем событие
                break;
            case ev_timer:
                // Запускаем таймер инактивности
                if (mode.menu) timer1_valcoder = TIMER_INACTIVE;
            case ev_cancel:
                lcd_clrscr();
                // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW;
                switch (mode.menu) {
                    case 0: lcd_primary_screen(); break;
                    case 1: --mode.menu; lcd_initedit(-1); break;
                    case 2: --mode.menu; lcd_initedit(-1); break;
                    case 3: mode.menu = 1; break;
                };
                event = ev_none;            // Очищаем событие
                break;
            default:
                break;
        };
    }; // while (1)
}
// Печать всех термометров
void printallterms(void) {
    int term;
    register byte i;

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
    float lt;
    lt = termometers[num].scale / 127;
    lt = termometers[num].t * (lt + 1) + termometers[num].offset;
    return (int)lt;
};
// Полностью прорисовать главный экран
void lcd_primary_screen(void) {
    lcd_command(LCD_DISP_ON);       // Убираем курсор с LCD
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 первой строки
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
            /* case 0x77:                // символ 'w'
                // Входим в меню (эмуляция кнопки Enter)
                // parameters[12].val_data++; if (parameters[12].val_data > 3) parameters[12].val_data = 0;
                // if (++main_menu[1].val_data > 3) main_menu[1].val_data = 0; // [12] Основной режим
                // init_curr_menu(&parameters[0], NUM_PARAMETERS);
                break;
            case 0x73:                // символ 's'
                // Выходим из меню (эмуляция кнопки Cancel)
                // parameters[12].val_data--; if (parameters[12].val_data < 0) parameters[12].val_data = 3;
                // if (--main_menu[1].val_data < 0) main_menu[1].val_data = 3; // [12] Основной режим
                // Текущий указатель устанавливаем на главное меню
                init_curr_menu(&main_menu[0], NUM_MENU);
                break;
            */
            case 0x78:                // символ 'x' Печатаем текущее меню
                print_curr_menu(); break;
            /* case 0x65:                // символ 'e'
                lcd_initedit(0); clatsman.edit_mode = 1;
                break;
            case 0x64:                // символ 'd'
                lcd_initedit(1); clatsman.edit_mode = 0;
                break;
            case 0x63:                // символ 'c'
                lcd_initedit(-1); clatsman.edit_mode = 0;
                break;*/
            case 0x6D:                // символ 'm'
                for(i=0; i<NUM_PARAMETERS; i++) printf("%s\t", param_str(i, parameters));
                printf("\r\n");
                break;
            case 0x6E:                // символ 'n'
                for(i=0; i<NUM_MENU; i++) printf("%s\t", param_str(i, main_menu));
                printf("\r\n");
                break;
            default:
                printf("Нажат символ 0x%x\r\n", inbyte);
        };
    }
}