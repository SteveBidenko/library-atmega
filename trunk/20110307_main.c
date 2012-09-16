#include <mega16.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include <delay.h>
#include "robowater.h"
// DS1820 функции температурного преобразования (Биденко+Панарин)
#include "spd1820.h"
#include "valcoder.h"
// Локальные макроподстановки
#define MAJOR_VERSION 1
#define MINOR_VERSION 25
// Определение главных структур
// Описание глобальны переменных
// int tw_prs;             // Заданная температура воды
struct st_time s_time;
struct st_date s_date;
struct st_info info = {2000, -350, 2400, 7200, 6500, 40, 42, 95, 100};
struct st_clatsman clatsman = {1, 1, 0, 1, 0, 0, 0, 0};
// Начальные положения меню
// struct st_menupos menupos = {0, 0, 0, 0, 0, 0};
lcd_str linestr;            // Строка для LCD
signed char curr_menu_level1 = 0, next_menu_level1 = 1;  // Текущий и следующий пункт меню
lcd_str mainscreen[16] = {     // Меню первого уровня
"ПОМЕЩЕНИЕ ",
"УЛИЦА     ",
"ОТКР.КРАН ",
"УСТ.КРАН  ",
"ВОДА ВХ.  ",
"ВОДА ВЫХ. ",
"ВЕНТИЛЯТОР",
"УСТ.ВЕНТИЛ",
"ЗАСЛОНКА  ",
"НАСОС     ",
"РЕЖИМ     ",// Cтоп / Прогрев / Остановка / Пуск
"ДАТА      ",
"ВРЕМЯ     ",
"ЗИМА      ",
"АВАРИЙ НЕТ",
"НАСТРОЙКА "
};
// Описание функций
void printallterms(void);
// Показать в первой строке дисплея время и дату
void lcd_display_time(void) {
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 второй строки
    sprintf(linestr, "%02u:%02u:%02u %02u.%02u", s_time.cHH, s_time.cMM, s_time.cSS, s_date.cdd, s_date.cmo);
    lcd_puts(linestr);
}
// Функция, возвращающая нужный пункт меню. menu_choice - (-1, 0, 1)
byte next_menu(signed char *menu_level, signed char menu_choice) {
    // Увеличиваем или уменьшаем текущий показатель меню
    *menu_level += menu_choice;
    // Начинаем проверки на укладываемость в диапазоне
    if (*menu_level >= MENU_LEVEL1) *menu_level = 0;
    if (*menu_level < 0) { *menu_level = MENU_LEVEL1 - 1; return 0; }
    // Анализируем то, что мы будем возвращать
    if ((*menu_level + 1) == MENU_LEVEL1) return 0; else return ((byte)*menu_level + 1);
}
// Печать всех значений меню в отладочном терминале
void print_all_menu(void) {
    register byte i;
    // Выведем на отладочную консоль все пункты меню
    printf ("Главное меню: ");
    for (i=0; i<MENU_LEVEL1; i++) printf ("%s ", mainscreen[i]);
    printf ("\r\n");
}
// Функция вывода текущего и следующего пункта меню
void lcd_menu(signed char direction) {
    // Если направление отличается от нуля, то модифицируем текущие пункты меню
    if (direction) next_menu_level1 = next_menu(&curr_menu_level1, direction);
    lcd_gotoxy(0,0);        // Устанавливаем курсор в позицию 0 строки 1
    sprintf(linestr, "%s", mainscreen[curr_menu_level1]); lcd_puts(linestr);
    lcd_gotoxy(0,1);        // Устанавливаем курсор в позицию 0 строки 1
    sprintf(linestr, "%s", mainscreen[next_menu_level1]); lcd_puts(linestr);
}
// Основная программа
void main(void) {
    byte status, inbyte;    // Описание локальных переменных
    int term;
    register byte i;
    init();                  // Инициализация всей периферии
    #asm("sei")             // Global enable interrupts
    printf("Старт программы RoboWater. %u.%02u\r\n", MAJOR_VERSION, MINOR_VERSION);
    prn = 1;
    if (ds1820_devices) {       // Если есть термометры, то выводим их значение
        for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
        delay_ms (DS1820_ALL_DELAY);
        printallterms();
    }
    // print_all_menu();       // Выведем на отладочную консоль все пункты меню
    lcd_display_time();       // выводим стартовую картинку на экранчик
    while(1) {
        // Обрабатываем последовательный порт
        if ((status = UCSRA) & RX_COMPLETE) { // Пришло ли что-нибудь
            inbyte = UDR;
            switch (inbyte) {
                case 0x7A:                // символ 'z'
                    printf("Время: %02u:%02u:%02u, дата:%02u.%02u.%02u, найдено %u термометров\r\n",
                            s_time.cHH, s_time.cMM, s_time.cSS, s_date.cdd, s_date.cmo, s_date.cyy, ds1820_devices);
                    printallterms();
                    break;
                case 0x71:                // символ 'q'
                    next_menu_level1 = next_menu(&curr_menu_level1, 1);
                    printf("Выбор меню [%u, %u]", curr_menu_level1, next_menu_level1);
                    printf(" 1) ""%s"",\t2) ""%s""\r\n", mainscreen[curr_menu_level1], mainscreen[next_menu_level1]);
                    break;
                case 0x61:                // символ 'a'
                    next_menu_level1 = next_menu(&curr_menu_level1, -1);
                    printf("Выбор меню [%u, %u]", curr_menu_level1, next_menu_level1);
                    printf(" 1) ""%s"",\t2) ""%s""\r\n", mainscreen[curr_menu_level1], mainscreen[next_menu_level1]);
                    break;
                case 0x78:                // символ 'x'
                    print_all_menu();
                    break;
                default:
                    printf("Нажат символ 0x%x\r\n", inbyte);
            };
        }
        // проверяем и присваиваем биту PORTD.4 состояния PIND.6
        if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
        // Обрабатываем ежесекундное событие.
        if (prn) {
            // Очищаем событие
            prn = 0;
            // Запускаем все термометры на измерение
            for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
            if (clatsman.valcoder_mode == 0) { // Если сейчас не режим valcoder'а, то рисуем стандартные строки
                // lcd_clrscr();
                lcd_command(LCD_DISP_ON);       // Убираем курсор с LCD
                lcd_display_time();             // Рисуем стандартный экранчик
                lcd_gotoxy(0,1);                // Устанавливаем курсор в позицию 0 строки 2
                if(ds1820_devices){             // Выводим информацию о термометрах
                    // delay_ms (DS1820_ALL_DELAY);
                    term = read_term(0); sprintf(linestr, "t=%-i.%-uC ", term/100, abs(term%100));
                } else sprintf(linestr, "No termometers");
                lcd_puts(linestr);
            }
        }
        // Обрабатываем событие valcoder'а
        if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {
            // Запускаем таймер инактивности valcoder
            timer1_valcoder = VALCODER_TIMER_OVERFLOW;
            // Если перед этим событием не было режима valcoder
            if (clatsman.valcoder_mode == 0) {
                lcd_clrscr();                   // Очищаем экран
                clatsman.valcoder_mode = 1;     // Устанавливаем флаг режима valcoder
                lcd_menu(0);                    // Выводим меню без изменения позиции
            } else                              // Анализируем, где находится курсор и куда крутим valcoder
                // Если valcoder крутится вправо и курсор был на 1 линии, то ставим ему признак перевода на 2 линию
                if ((valcoder > 0) && (clatsman.lcdline == 0)) clatsman.lcdline = 1;
                else
                    // Если valcoder крутится влево и курсор был на 2 линии, то ставим ему признак перевода на 1 линию
                    if ((valcoder < 0) && (clatsman.lcdline == 1)) clatsman.lcdline = 0;
                    // В противном случае обновляем меню
                    else lcd_menu(valcoder);
            // Выводим курсор на позицию
            if (clatsman.lcdline == 0) lcd_gotoxy(LCD_DISP_LENGTH-1,0); else lcd_gotoxy(LCD_DISP_LENGTH-1,1);
            // Даем команду на моргание курсором
            lcd_command(LCD_DISP_ON_CURSOR_BLINK);
            // if (valcoder > VALCODER_NO_ROTATE) printf (" -->\r\n"); else printf ("<-- \r\n");
            // printf (" [V = %i, I0 = %u, I1 = %u]\r\n", valcoder, counter0, counter1);
            // Обнуляем все переменные
            valcoder = VALCODER_NO_ROTATE; // counter0 = 0; counter1 = 0;
            // delay_ms (500);
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

