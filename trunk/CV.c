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
#include "at2404.h"
// ��������� ����������������
#define MAJOR_VERSION 1
#define MINOR_VERSION 30
// enum
// ����������� ������� ��������
// �������� ��������� ����������
// int tw_prs;             // �������� ����������� ����
// TW_in_Min -����������� ���� �� �����( ������ ) ���.
// TW_out_Min -����������� ���� �� ������ ( ������� ) ���.
// TW_out_Stop -����������� ���� �� ����� ��� ����������� � ������ ����.
// TA_in_Min -����������� ������� �� ����� ��� -30 �.
// TA_out_Min -����������� ������� �� ������ ��� +15 �.
// TA_out_prs -����������� ������� �� ������ ������������� +20 �.(��������)


struct st_datetime s_dt;
struct st_mode mode = {0, 0, 0, 1};           // ������� ����� ������
struct st_prim_par {
    byte PWM_out1, PWM_out2, ADC1, ADC2;
    int Tw1, Tw2, Ta1, Ta2, TW_in_Min, TW_out_Min, TW_out_Stop, TA_in_Min, TA_out_Min,TA_out_prs; 
} prim_par={
    55, 15, 30, 30,
    99, 1, 2, 3, 75, 15, 30, 27, 15, 20
};
enum en_event event;                          // ������� ������� � �������
// �������� �������
void printallterms(void); void lcd_primary_screen(void); void check_serial(void);
// �������� ���������
void main(void) {
    // register byte i;
    // struct st_prim_par *ppr_par;
    byte size_prim_par;
    init();                  // ������������� ���� ���������
    #asm("sei")             // Global enable interrupts
    printf("����� ��������� RoboWater. %u.%02u. ������� %u �����������.\r\n", MAJOR_VERSION, MINOR_VERSION, ds1820_devices);
    // ���� ���� ����������, �� ������� �� ��������
    printallterms();
    // print_all_menu();       // ������� �� ���������� ������� ��� ������ ����
    // ��������� � EEPROM ��������� prim_par
    // ppr_par = &prim_par; 
    size_prim_par = sizeof(prim_par);  
    eeprom_write_struct ((char *)&prim_par, size_prim_par);
    // printf("�� ������ � EEPROM (%u bytes) �������� Tw2=%u ... ", size_prim_par, prim_par.Tw2); 
    // prim_par.Tw2 = 99;
    // eeprom_write(0, size_prim_par);
    // size_prim_par = eeprom_read(0);
    // printf("�������� �������� �������� Tw2=%u ...", prim_par.Tw2); 
    // ��������������� �� EEPROM ��������� prim_par
    eeprom_read_struct ((char *)&prim_par, size_prim_par);
    printf("����� ������ �� EEPROM �������� Tw2=%u\r\n", prim_par.Tw2); 
    // printf("���� %u, ����� ����� ������ �� EEPROM size=%u\r\n", sizeof(prim_par), size_prim_par); 
    /*
    // ���������� �� ������ ��������� ����� ����������� ������ ��������� ���� � ������� � �������������� ������� �� �������
    timer1_valcoder = 60;       // ��������� ������� 30 ���
    while(timer1_valcoder) {
         lcd_command(LCD_DISP_ON);       // ������� ������ � LCD
         lcd_gotoxy(0,0);        // ������������� ������ � ������� 0 ������ ������
         sprintf(linestr, "���.�������");
    }
    */
    lcd_primary_screen();       // ������� ��������� �������� �� ��������
    // if (PINC.6) PORTD |= (1<<4); else PORTD &= ~(1<<4);
    // if (PINC.7) PORTD &= ~(1<<5); else PORTD |= (1<<5);
    // if (PINC.5) PORTD &= ~(1<<5); else PORTD |= (1<<5);
    while(1) {
        check_serial();
        // ��������� � ����������� ���� PORTD.4 ��������� PIND.6
        // if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
        if (event == ev_none) {
            if (!VALCODER_ENTER) {
                // ���������� �������� Enter
                delay_ms(100);
                if (!VALCODER_ENTER) {  // printf ("������������� ������� Enter\r\n");
                    event = ev_enter;
                    // ��������� ������ ������������
                    timer1_valcoder = TIMER_INACTIVE;                }
            }
            if (!VALCODER_CANCEL) {
                // ���������� �������� Cancel
                delay_ms(50);
                if (!VALCODER_CANCEL) { // printf ("������������� ������� Cancel\r\n");
                    event = ev_cancel;
                    // ��������� ������ ������������
                    timer1_valcoder = TIMER_INACTIVE;                }
            }
            // ������������ ������� valcoder'�
            if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {        // ���� �������� valcoder
                // printf ("������������� �������� (%i)...\r\n", valcoder);
                if (valcoder < 0) event = ev_left;
                else event = ev_right;
                // ��������� ������ ������������
                timer1_valcoder = TIMER_INACTIVE;
                valcoder = VALCODER_NO_ROTATE;
                // if (valcoder > VALCODER_NO_ROTATE) printf (" -->\r\n"); else printf ("<-- \r\n");
                // printf (" [V = %i, I0 = %u, I1 = %u]\r\n", valcoder, counter0, counter1);
            }
        }
        switch (event) {
            case ev_secunda:                // ������������ ������������ �������.
                main_menu[0].val_data = read_term(0);       // ������� ���������� � ������� ����������
                switch (mode.menu) {
                    case 0: lcd_primary_screen(); break;
                    case 1: lcd_menu(0); break;
                    default: ;
                    // case 2: lcd_edit(0); break;
                };
                event = ev_none;            // ������� �������
                break;
            case ev_left:                   // printf ("������������ ��������� valcoder �����\r\n");
            case ev_right:                  // printf ("������������ ��������� valcoder ������\r\n");
                // printf ("������������ ��������� valcoder (%d), � ������ %d - ", event-2, mode.menu);
                switch (mode.menu) {
                    case 0: lcd_menu(mode.menu++); break;   // ������� ���� ��� ��������� ������� printf ("entering...");
                    case 1: lcd_menu(event-2); break;       // printf ("navigating...");
                    case 2: lcd_edit(event-2); break;       // printf ("editing...");
                    default: ;                              // printf ("defaulting...");
                }
                // printf ("\r\n");
                event = ev_none;            // ������� �������
                break;
            case ev_enter:                  // ���� ����� Enter
                // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW; if (mode.menu <= 2)
                switch (mode.menu) {
                    // lcd_primary_screen();
                    // ������������ ������� enter c ������ ����, ��� �������� mode.menu ��� ������
                    case 0: lcd_menu(0); ++mode.menu; break;    // ���� ���������� � ������� ������, ��������� ���������� ����
                    case 1:                                     // ���� ���������� � ����, �� �����������
                            lcd_initedit(0); break;
                    case 2: lcd_initedit(1); mode.menu = 1; break;
                };
                event = ev_none;            // ������� �������
                break;
            case ev_timer:
                // ��������� ������ ������������
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
                event = ev_none;            // ������� �������
                break;
            default:
                break;
        };
    }; // while (1)
}
// ������ ���� �����������
void printallterms(void) {
    int term;
    register byte i;

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
    float lt;
    lt = termometers[num].scale / 127;
    lt = termometers[num].t * (lt + 1) + termometers[num].offset;
    return (int)lt;
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
            /* case 0x77:                // ������ 'w'
                // ������ � ���� (�������� ������ Enter)
                // parameters[12].val_data++; if (parameters[12].val_data > 3) parameters[12].val_data = 0;
                // if (++main_menu[1].val_data > 3) main_menu[1].val_data = 0; // [12] �������� �����
                // init_curr_menu(&parameters[0], NUM_PARAMETERS);
                break;
            case 0x73:                // ������ 's'
                // ������� �� ���� (�������� ������ Cancel)
                // parameters[12].val_data--; if (parameters[12].val_data < 0) parameters[12].val_data = 3;
                // if (--main_menu[1].val_data < 0) main_menu[1].val_data = 3; // [12] �������� �����
                // ������� ��������� ������������� �� ������� ����
                init_curr_menu(&main_menu[0], NUM_MENU);
                break;
            */
            case 0x78:                // ������ 'x' �������� ������� ����
                print_curr_menu(); break;
            /* case 0x65:                // ������ 'e'
                lcd_initedit(0); clatsman.edit_mode = 1;
                break;
            case 0x64:                // ������ 'd'
                lcd_initedit(1); clatsman.edit_mode = 0;
                break;
            case 0x63:                // ������ 'c'
                lcd_initedit(-1); clatsman.edit_mode = 0;
                break;*/
            case 0x6D:                // ������ 'm'
                for(i=0; i<NUM_PARAMETERS; i++) printf("%s\t", param_str(i, parameters));
                printf("\r\n");
                break;
            case 0x6E:                // ������ 'n'
                for(i=0; i<NUM_MENU; i++) printf("%s\t", param_str(i, main_menu));
                printf("\r\n");
                break;
            default:
                printf("����� ������ 0x%x\r\n", inbyte);
        };
    }
}