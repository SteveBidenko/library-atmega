#include <mega16.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include <delay.h>
#include "robowater.h"
// DS1820 ������� �������������� �������������� (�������+�������)
#include "spd1820.h"
#include "valcoder.h"
// ��������� ����������������
#define MAJOR_VERSION 1
#define MINOR_VERSION 25
// ����������� ������� ��������
// �������� ��������� ����������
// int tw_prs;             // �������� ����������� ����
struct st_time s_time;
struct st_date s_date;
struct st_info info = {2000, -350, 2400, 7200, 6500, 40, 42, 95, 100};
struct st_clatsman clatsman = {1, 1, 0, 1, 0, 0, 0, 0};
// ��������� ��������� ����
// struct st_menupos menupos = {0, 0, 0, 0, 0, 0};
lcd_str linestr;            // ������ ��� LCD
signed char curr_menu_level1 = 0, next_menu_level1 = 1;  // ������� � ��������� ����� ����
lcd_str mainscreen[16] = {     // ���� ������� ������
"��������� ",
"�����     ",
"����.���� ",
"���.����  ",
"���� ��.  ",
"���� ���. ",
"����������",
"���.������",
"��������  ",
"�����     ",
"�����     ",// C��� / ������� / ��������� / ����
"����      ",
"�����     ",
"����      ",
"������ ���",
"��������� "
};
// �������� �������
void printallterms(void);
// �������� � ������ ������ ������� ����� � ����
void lcd_display_time(void) {
    lcd_gotoxy(0,0);        // ������������� ������ � ������� 0 ������ ������
    sprintf(linestr, "%02u:%02u:%02u %02u.%02u", s_time.cHH, s_time.cMM, s_time.cSS, s_date.cdd, s_date.cmo);
    lcd_puts(linestr);
}
// �������, ������������ ������ ����� ����. menu_choice - (-1, 0, 1)
byte next_menu(signed char *menu_level, signed char menu_choice) {
    // ����������� ��� ��������� ������� ���������� ����
    *menu_level += menu_choice;
    // �������� �������� �� �������������� � ���������
    if (*menu_level >= MENU_LEVEL1) *menu_level = 0;
    if (*menu_level < 0) { *menu_level = MENU_LEVEL1 - 1; return 0; }
    // ����������� ��, ��� �� ����� ����������
    if ((*menu_level + 1) == MENU_LEVEL1) return 0; else return ((byte)*menu_level + 1);
}
// ������ ���� �������� ���� � ���������� ���������
void print_all_menu(void) {
    register byte i;
    // ������� �� ���������� ������� ��� ������ ����
    printf ("������� ����: ");
    for (i=0; i<MENU_LEVEL1; i++) printf ("%s ", mainscreen[i]);
    printf ("\n");
}
// ������� ������ �������� � ���������� ������ ����
void lcd_menu(signed char direction) {
    // ���� ����������� ���������� �� ����, �� ������������ ������� ������ ����
    if (direction) next_menu_level1 = next_menu(&curr_menu_level1, direction);
    lcd_gotoxy(0,0);        // ������������� ������ � ������� 0 ������ 1
    sprintf(linestr, "%s", mainscreen[curr_menu_level1]); lcd_puts(linestr);
    lcd_gotoxy(0,1);        // ������������� ������ � ������� 0 ������ 1
    sprintf(linestr, "%s", mainscreen[next_menu_level1]); lcd_puts(linestr);
}
// �������� ���������
void main(void) {
    byte status, inbyte;    // �������� ��������� ����������
    int term;
    register byte i;
    init();                  // ������������� ���� ���������
    #asm("sei")             // Global enable interrupts
    printf("����� ��������� RoboWater. %u.%02u\r\n", MAJOR_VERSION, MINOR_VERSION);
    prn = 1;
    if (ds1820_devices) {       // ���� ���� ����������, �� ������� �� ��������
        for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
        delay_ms (DS1820_ALL_DELAY);
        printallterms();
    }
    // print_all_menu();       // ������� �� ���������� ������� ��� ������ ����
    lcd_display_time();       // ������� ��������� �������� �� ��������
    while(1) {
        // ������������ ���������������� ����
        if ((status = UCSRA) & RX_COMPLETE) { // ������ �� ���-������
            inbyte = UDR;
            switch (inbyte) {
                case 0x7A:                // ������ 'z'
                    printf("�����: %02u:%02u:%02u, ����:%02u.%02u.%02u, ������� %u �����������\r\n",
                            s_time.cHH, s_time.cMM, s_time.cSS, s_date.cdd, s_date.cmo, s_date.cyy, ds1820_devices);
                    printallterms();
                    break;
                case 0x71:                // ������ 'q'
                    next_menu_level1 = next_menu(&curr_menu_level1, 1);
                    printf("����� ���� [%u, %u]", curr_menu_level1, next_menu_level1);
                    printf(" 1) ""%s"",\t2) ""%s""\r\n", mainscreen[curr_menu_level1], mainscreen[next_menu_level1]);
                    break;
                case 0x61:                // ������ 'a'
                    next_menu_level1 = next_menu(&curr_menu_level1, -1);
                    printf("����� ���� [%u, %u]", curr_menu_level1, next_menu_level1);
                    printf(" 1) ""%s"",\t2) ""%s""\r\n", mainscreen[curr_menu_level1], mainscreen[next_menu_level1]);
                    break;
                case 0x78:                // ������ 'x'
                    print_all_menu();
                    break;
                default:
                    printf("����� ������ 0x%x\r\n", inbyte);
            };
        }
        // ��������� � ����������� ���� PORTD.4 ��������� PIND.6
        if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
        // ������������ ������������ �������.
        if (prn) {
            // ������� �������
            prn = 0;
            // ��������� ��� ���������� �� ���������
            for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
            if (clatsman.valcoder_mode == 0) { // ���� ������ �� ����� valcoder'�, �� ������ ����������� ������
                // lcd_clrscr();
                lcd_command(LCD_DISP_ON);       // ������� ������ � LCD
                lcd_display_time();             // ������ ����������� ��������
                lcd_gotoxy(0,1);                // ������������� ������ � ������� 0 ������ 2
                if(ds1820_devices){             // ������� ���������� � �����������
                    // delay_ms (DS1820_ALL_DELAY);
                    term = read_term(0); sprintf(linestr, "t=%-i.%-uC ", term/100, abs(term%100));
                } else sprintf(linestr, "No termometers");
                lcd_puts(linestr);
            }
        }
        // ������������ ������� valcoder'�
        if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {
            // ��������� ������ ������������ valcoder
            timer1_valcoder = VALCODER_TIMER_OVERFLOW;
            // ���� ����� ���� �������� �� ���� ������ valcoder
            if (clatsman.valcoder_mode == 0) {
                lcd_clrscr();                   // ������� �����
                clatsman.valcoder_mode = 1;     // ������������� ���� ������ valcoder
                lcd_menu(0);                    // ������� ���� ��� ��������� �������
            } else                              // �����������, ��� ��������� ������ � ���� ������ valcoder
                // ���� valcoder �������� ������ � ������ ��� �� 1 �����, �� ������ ��� ������� �������� �� 2 �����
                if ((valcoder > 0) && (clatsman.lcdline == 0)) clatsman.lcdline = 1;
                else
                    // ���� valcoder �������� ����� � ������ ��� �� 2 �����, �� ������ ��� ������� �������� �� 1 �����
                    if ((valcoder < 0) && (clatsman.lcdline == 1)) clatsman.lcdline = 0;
                    // � ��������� ������ ��������� ����
                    else lcd_menu(valcoder);
            // ������� ������ �� �������
            if (clatsman.lcdline == 0) lcd_gotoxy(LCD_DISP_LENGTH-1,0); else lcd_gotoxy(LCD_DISP_LENGTH-1,1);
            // ���� ������� �� �������� ��������
            lcd_command(LCD_DISP_ON_CURSOR_BLINK);
            // if (valcoder > VALCODER_NO_ROTATE) printf (" -->\r\n"); else printf ("<-- \r\n");
            // printf (" [V = %i, I0 = %u, I1 = %u]\r\n", valcoder, counter0, counter1);
            // �������� ��� ����������
            valcoder = VALCODER_NO_ROTATE; // counter0 = 0; counter1 = 0;
            // delay_ms (500);
        }
    };
}
// ������ ���� �����������
void printallterms(void) {
    register byte i;
    int term;
    if(!ds1820_devices) return;			// ���� ���������� �� ���������� - ������ ������� �� �������
    printf("\t");						// �������� ���� ��������� � ���������
    for(i=0; i<ds1820_devices; i++) {
    	term = read_term(i);
        printf(" t%-u = %-i.%-uC; ", i+1, term/100, abs(term%100));
    }
    printf("\r\n");
}
// ������ ����������� (�������� - ����� ���������� ������� � 0)
int read_term(byte num) {
    int lterm;
    lterm = ds1820_temperature_10(&ds1820_rom_codes[num][0]);
    return (lterm);
};

