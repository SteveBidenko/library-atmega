#include <mega16.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
#include <delay.h>
#include "lcd_4bit.h"
#include "menu.h"
#include "robowater.h"
// DS1820 ������� �������������� �������������� (�������+�������)
#include "spd1820.h"
#include "valcoder.h"
// ��������� ����������������
#define MAJOR_VERSION 1
#define MINOR_VERSION 28
#define NUM_PARAMETERS 11
// enum
enum en_event {
    e_no_event = 0,
    e_secunda,               // �������� � prn
    e_enter,
    e_cancel,
    e_valcoder,
    e_timer1_valcoder        //
};
// ����������� ������� ��������
// �������� ��������� ����������
// int tw_prs;             // �������� ����������� ����
struct st_datetime s_dt;
struct st_parameter parameters[NUM_PARAMETERS]= {
    {2000, e_temperature, "���.���� ", 1},    // [0] �������� ����������� �������
    {-350, e_temperature, "����� ", 0},       // [1] ����������� ������� �� �����
    {2400, e_temperature, "��������� ", 0},   // [2] ����������� ������� � ���������
    {7200, e_temperature, "���� ��. ", 0},    // [3] ����������� ���� �� �����
    {6500, e_temperature, "���� ���. ", 0},   // [4] ����������� ���� �� ������
    {40, e_percent, "����.���� ", 0},         // [5] ������� ��������� ����� (���)
    {42, e_percent, "���.���� ", 0},          // [6] �������� ��������� ����� (��������� PWM)
    {95, e_percent, "��������. ", 0},         // [7] ������� ��������� ����������� (���)
    {100, e_percent, "���.����. ", 0},        // [8] �������� ��������� ����������� (��������� PWM)
    {1, e_clatsman, "�������� ", 0},          // [9] ��������� ��������� �������� (0 - �������, 1 - �������)
    {1, e_clatsman, "����� ", 0}              // [10] ������ ������ (1 - ���., 0 - ����.)
};
/*struct st_setting parameters[NUM_PARAMETERS]= {
    {2000, e_temperature, "���.���� ", 1},    // [0] �������� ����������� �������
    {-350, e_temperature, "����� ", 0},       // [1] ����������� ������� �� �����
    {2400, e_temperature, "��������� ", 0},   // [2] ����������� ������� � ���������
    {7200, e_temperature, "���� ��. ", 0},    // [3] ����������� ���� �� �����
    {6500, e_temperature, "���� ���. ", 0},   // [4] ����������� ���� �� ������
    {40, e_percent, "����.���� ", 0},         // [5] ������� ��������� ����� (���)
    {42, e_percent, "���.���� ", 0},          // [6] �������� ��������� ����� (��������� PWM)
    {95, e_percent, "��������. ", 0},         // [7] ������� ��������� ����������� (���)
    {100, e_percent, "���.����. ", 0},        // [8] �������� ��������� ����������� (��������� PWM)
    {1, e_clatsman, "�������� ", 0},          // [9] ��������� ��������� �������� (0 - �������, 1 - �������)
    {1, e_clatsman, "����� ", 0}              // [10] ������ ������ (1 - ���., 0 - ����.)
};
*/
struct st_clatsman clatsman = {0, 0, 0, 0};
// �������� �������
void printallterms(void); void lcd_primary_screen(void); void check_serial(void);
// �������� ���������
void main(void) {
    int term;
    register byte i;
    init();                  // ������������� ���� ���������
    #asm("sei")             // Global enable interrupts
    printf("����� ��������� RoboWater. %u.%02u. ������� %u �����������.\r\n", MAJOR_VERSION, MINOR_VERSION, ds1820_devices);
    prn = 1;
    if (ds1820_devices) {       // ���� ���� ����������, �� ������� �� ��������
        for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
        delay_ms (DS1820_ALL_DELAY);
        printallterms();
    }
    // print_all_menu();       // ������� �� ���������� ������� ��� ������ ����
    lcd_primary_screen();       // ������� ��������� �������� �� ��������
    while(1) {
        check_serial();
        // ��������� � ����������� ���� PORTD.4 ��������� PIND.6
        // if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
        if (!VALCODER_ENTER) { delay_ms(30); clatsman.enter = VALCODER_ENTER; }    // ���������� �������� Enter
        if (!VALCODER_CANCEL) { delay_ms(30); clatsman.cancel = VALCODER_CANCEL; } // ���������� �������� Cancel
        // if (PINC.6) PORTD |= (1<<4); else PORTD &= ~(1<<4);
        // if (PINC.7) PORTD &= ~(1<<5); else PORTD |= (1<<5);
        // if (PINC.5) PORTD &= ~(1<<5); else PORTD |= (1<<5);
        // ������������ ������������ �������.
        if (prn) {
            // ������� �������
            prn = 0;
            // ������� ���������� � ������� ����������
            if(ds1820_devices){ term = read_term(0); main_menu[0].val_data = term; }
            // ����� ����������� ����������� ����� ������ ���������
            if (clatsman.valcoder_mode == 0) { // ���� ������ �� ����� valcoder'�, �� ������ ����������� ������
                lcd_primary_screen();          // ������ ����������� ��������
            } else {                           // ��������� ��� ������
                // ������� ��� ������ ���� � �������� ������ � ����������� �� clatsman.lcdline
                if (clatsman.edit_mode) lcd_edit(0); else lcd_menu(0);
            }
            // ��������� ��� ���������� �� ���������
            for(i=0; i<ds1820_devices; i++) ds1820_run_measure(&ds1820_rom_codes[i][0]);
        }
        if (clatsman.enter) { clatsman.enter = 0;           // ����� ������� �������
            clatsman.valcoder_mode = 1;     // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW;
            if (!clatsman.edit_mode) init_curr_menu(&parameters[0], NUM_PARAMETERS);
        }
        if (clatsman.cancel) { clatsman.cancel = 0;
            clatsman.valcoder_mode = 0;   // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW;
            if (!clatsman.edit_mode) { lcd_clrscr(); init_curr_menu(&main_menu[0], MENU_LEVEL1); }
        }
        // ������������ ������� valcoder'�
        if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {
            // ��������� ������ ������������ valcoder
            timer1_valcoder = VALCODER_TIMER_OVERFLOW;
            // ���� ����� ���� �������� �� ���� ������ valcoder
            if (clatsman.valcoder_mode == 0) {
                clatsman.valcoder_mode = 1;     // ������������� ���� ������ valcoder
                valcoder = VALCODER_NO_ROTATE;  // ������� ���� ��� ��������� �������
            } else                              // �����������, ��� ��������� ������ � ���� ������ valcoder
                // ���� valcoder �������� ������ � ������ ��� �� 1 �����, �� ������ ��� ������� �������� �� 2 �����
                if ((valcoder > 0) && (curr_menu.lcd == 0)) { curr_menu.lcd = 1; valcoder = VALCODER_NO_ROTATE; }
                // ���� valcoder �������� ����� � ������ ��� �� 2 �����, �� ������ ��� ������� �������� �� 1 �����
                else if ((valcoder < 0) && (curr_menu.lcd == 1)) { curr_menu.lcd = 0; valcoder = VALCODER_NO_ROTATE; }
            // ��������� ����
            if (clatsman.edit_mode) lcd_edit(valcoder); else lcd_menu(valcoder);
            // if (valcoder > VALCODER_NO_ROTATE) printf (" -->\r\n"); else printf ("<-- \r\n");
            // printf (" [V = %i, I0 = %u, I1 = %u]\r\n", valcoder, counter0, counter1);
            // �������� ��� ����������
            valcoder = VALCODER_NO_ROTATE; // counter0 = 0; counter1 = 0;
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
// ��������� ����������� ������� �����
void lcd_primary_screen(void) {
    lcd_command(LCD_DISP_ON);       // ������� ������ � LCD
    lcd_gotoxy(0,0);        // ������������� ������ � ������� 0 ������ ������
    sprintf(linestr, "%02u:%02u:%02u %02u.%02u", s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo);
    lcd_puts(linestr);
    lcd_gotoxy(0,1);                // ������������� ������ � ������� 0 ������ 2
    // ������� ���������� � �����������
    if(ds1820_devices)
        sprintf(linestr, "t=%-i.%-uC ", main_menu[0].val_data/100, abs(main_menu[0].val_data%100));
    else
        sprintf(linestr, "No termometers");
    lcd_puts(linestr);
}
// ��������� ������� �� ��������� �����
void check_serial(void) {
    byte inbyte;    // �������� ��������� ����������
    register byte i;
    // ������������ ���������������� ����
    if (UCSRA & RX_COMPLETE) { // ������ �� ���-������
        inbyte = UDR;
        switch (inbyte) {
            case 0x7A:                // ������ 'z'
                printf("�����: %02u:%02u:%02u, ����:%02u.%02u.%02u, ������� %u �����������\r\n",
                        s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo, s_dt.cyy, ds1820_devices);
                printallterms();
                break;
            case 0x71:                // ������ 'q'
                print_curr_menu2(1); break;
            case 0x61:                // ������ 'a'
                print_curr_menu2(-1); break;
            case 0x77:                // ������ 'w'
                // ������ � ���� (�������� ������ Enter)
                // parameters[12].val_data++; if (parameters[12].val_data > 3) parameters[12].val_data = 0;
                // if (++main_menu[1].val_data > 3) main_menu[1].val_data = 0; // [12] �������� �����
                init_curr_menu(&parameters[0], NUM_PARAMETERS);
                break;
            case 0x73:                // ������ 's'
                // ������� �� ���� (�������� ������ Cancel)
                // parameters[12].val_data--; if (parameters[12].val_data < 0) parameters[12].val_data = 3;
                // if (--main_menu[1].val_data < 0) main_menu[1].val_data = 3; // [12] �������� �����
                // ������� ��������� ������������� �� ������� ����
                init_curr_menu(&main_menu[0], MENU_LEVEL1);
                break;
            case 0x78:                // ������ 'x' �������� ������� ����
                print_curr_menu(); break;
            case 0x65:                // ������ 'e'
                lcd_initedit(0); clatsman.edit_mode = 1;
                break;
            case 0x64:                // ������ 'd'
                lcd_initedit(1); clatsman.edit_mode = 0;
                break;
            case 0x63:                // ������ 'c'
                lcd_initedit(-1); clatsman.edit_mode = 0;
                break;
            case 0x6D:                // ������ 'm'
                for(i=0; i<NUM_PARAMETERS; i++) printf("%s\t", param_str(i, parameters));
                printf("\r\n");
                break;
            case 0x6E:                // ������ 'n'
                for(i=0; i<MENU_LEVEL1; i++) printf("%s\t", param_str(i, main_menu));
                printf("\r\n");
                break;
            default:
                printf("����� ������ 0x%x\r\n", inbyte);
        };
    }
}