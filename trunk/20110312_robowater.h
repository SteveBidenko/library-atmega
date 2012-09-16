#ifndef __ROBOWATER__
#define __ROBOWATER__
#define F_CPU 8000000UL
// Описание битов регистра "А" статуса и управления
#define RXC     7            // Флаг завершения приема USART
#define TXC     6            // Флаг завершения передачи USART
#define UDRE    5            // Флаг "Регистр данных USART пуст"
#define FE      4            // Флаг ошибки кадрирования
#define OVR     3            // Флаг переполнения
#define UPE     2            // Флаг ошибки контроля четности USART
#define MPCM    0            // Режим мультипроцессорного обмена
// Описание битов регистра "B" статуса и управления
#define RXCIE   7            // Разрешение прерывания по завершению приема
#define TXCIE   6            // Разрешение прерывания по завершению передачи
#define RXEN    4            // Разрешение приема
#define TXEN    3            // Разрешение передачи
#define UCSZ2   2            // Установка 9-ти битного режима
#define RXB8    1            // Разряд номер восемь приемного буфера
#define TXB8    0            // Разряд номер восемь буфера передачи.
// Описание статусов последовательных портов
#define FRAMING_ERROR	(1<<FE)
#define PARITY_ERROR    (1<<UPE)
#define DATA_OVERRUN    (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define TX_COMPLETE 	(1<<UDRE)
#define RX_COMPLETE 	(1<<RXC)
// maximum number of DS1820 devices connected to the 1 Wire bus
#define MAX_DS1820 8
#define VALCODER_TIMER_OVERFLOW 20      // Задается в кол-ве полсекундных интервалов
#define ENTER_CANCEL_OVERFLOW 2      // Задается в кол-ве полсекундных интервалов
#define STRLENGTH 16            // Длина строки параметра
#define LAMP_ECHO_PORT PORTD
#define LAMP_ECHO_PIN 4
// Описание типов переменных
typedef unsigned char 	byte;	// byte = unsigned char
typedef unsigned int 	word;	// word = unsigned int
typedef char str_val[STRLENGTH];
// Описание cтруктур
struct st_datetime {
    byte cHH, cMM, cSS;         // Текущее время
    byte cyy, cmo, cdd;         // Текущая дата
};
// Структура основных переключателей в системе
struct st_clatsman {
    byte alarm:1;           // 0) Тревога (0 - нет тревоги, 1 - тревога)
    byte setting:2;         // 1) Режим задания настроек (00 - установка заданной температура, 01 - установка даты)
                            //    10 - установка времени, 11 - установка всех настроек
    byte valcoder_mode:1;   // 2) Режим valcoder (1 - valcoder активен, 0 - отключен по таймеру)
    byte enter:1;           // 3) Нажат enter на valcoder (0 - нажат, 1 - отжат)
    byte cancel:1;          // 4) Нажата кнопка cancel (0 - нажат, 1 - отжат)
    byte edit_mode:1;       // 5) Режим редактирования
};
/*
// Структура позиций в меню
struct st_menupos {
    word primposmenu:4;     // 1) Позиция основного меню (МАХ 16)
    word run11:2;           // 2) Позиция основного режима (МАХ 4)
    word settingmenu:5;     // 3) Позиция меню настроек (МАХ 32)
    word rulemenu:3;        // 4) Позиция меню установки закона регулирования (МАХ 8)
    word alarmmenu:4;       // 5) Позиция меню аварий (МАХ 16)
};
*/
// Описание функций
extern void init(void);
extern int read_term(byte);
// Описание глобальных переменных
extern byte ds1820_devices;
extern byte ds1820_rom_codes[MAX_DS1820][9];
extern byte prn;
extern struct st_datetime s_dt;
// extern struct st_menupos menupos;
extern struct st_clatsman clatsman;
extern byte timer1_valcoder;
extern byte timer1_lamp;
#endif
