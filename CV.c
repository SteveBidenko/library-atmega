#include <mega16.h>
#include <stdlib.h>  // for abs
#include <stdio.h>
// Comment
#include <delay.h>
#include "lcd_4bit.h"
#include "menu.h"
#include "robowater.h"
// DS1820 ôóíêöèè òåìïåðàòóðíîãî ïðåîáðàçîâàíèÿ (Áèäåíêî+Ïàíàðèí)
#include "spd1820.h"
#include "valcoder.h"
#include "at2404.h"
// Ëîêàëüíûå ìàêðîïîäñòàíîâêè
#define MAJOR_VERSION 1
#define MINOR_VERSION 30
// enum
// Îïðåäåëåíèå ãëàâíûõ ñòðóêòóð
// Îïèñàíèå ãëîáàëüíû ïåðåìåííûõ
// int tw_prs;             // Çàäàííàÿ òåìïåðàòóðà âîäû
// TW_in_Min -Òåìïåðàòóðà Âîäû íà âõîäå( Ïîäà÷å ) ìèí.
// TW_out_Min -Òåìïåðàòóðà Âîäû íà âûõîäå ( Îáðàòêà ) ìèí.
// TW_out_Stop -Òåìïåðàòóðà Âîäû íà âõîäå äëÿ ïîääåðæàíèÿ â ðåæèìå ñòîï.
// TA_in_Min -Òåìïåðàòóðà Âîçäóõà íà âõîäå ìèí -30 Ñ.
// TA_out_Min -Òåìïåðàòóðà Âîçäóõà íà âûõîäå ìèí +15 Ñ.
// TA_out_prs -Òåìïåðàòóðà Âîçäóõà íà âûõîäå óñòàíîâëåííàÿ +20 Ñ.(Çàäàííàÿ)


struct st_datetime s_dt;
struct st_mode mode = {0, 0, 0, 1};           // Òåêóùèé ðåæèì ðàáîòû
struct st_prim_par {
    byte PWM_out1, PWM_out2, ADC1, ADC2;
    int Tw1, Tw2, Ta1, Ta2, TW_in_Min, TW_out_Min, TW_out_Stop, TA_in_Min, TA_out_Min,TA_out_prs; 
} prim_par={
    55, 15, 30, 30,
    99, 1, 2, 3, 75, 15, 30, 27, 15, 20
};
enum en_event event;                          // Òåêóùåå ñîáûòèå â ñèñòåìå
// Îïèñàíèå ôóíêöèé
void printallterms(void); void lcd_primary_screen(void); void check_serial(void);
// Îñíîâíàÿ ïðîãðàììà
void main(void) {
    // register byte i;
    // struct st_prim_par *ppr_par;
    byte size_prim_par;
    init();                  // Èíèöèàëèçàöèÿ âñåé ïåðèôåðèè
    #asm("sei")             // Global enable interrupts
    printf("Ñòàðò ïðîãðàììû RoboWater. %u.%02u. Íàéäåíî %u òåðìîìåòðîâ.\r\n", MAJOR_VERSION, MINOR_VERSION, ds1820_devices);
    // Åñëè åñòü òåðìîìåòðû, òî âûâîäèì èõ çíà÷åíèå
    printallterms();
    // print_all_menu();       // Âûâåäåì íà îòëàäî÷íóþ êîíñîëü âñå ïóíêòû ìåíþ
    // Ñîõðàíÿåì â EEPROM ñòðóêòóðó prim_par
    // ppr_par = &prim_par; 
    size_prim_par = sizeof(prim_par);  
    eeprom_write_struct ((char *)&prim_par, size_prim_par);
    // printf("Äî çàïèñè â EEPROM (%u bytes) çíà÷åíèå Tw2=%u ... ", size_prim_par, prim_par.Tw2); 
    // prim_par.Tw2 = 99;
    // eeprom_write(0, size_prim_par);
    // size_prim_par = eeprom_read(0);
    // printf("Èçìåíÿåì çíà÷åíèå çíà÷åíèå Tw2=%u ...", prim_par.Tw2); 
    // Âîññòàíàâëèâàåì èç EEPROM ñòðóêòóðó prim_par
    eeprom_read_struct ((char *)&prim_par, size_prim_par);
    printf("Ïîñëå ÷òåíèå èç EEPROM çíà÷åíèå Tw2=%u\r\n", prim_par.Tw2); 
    // printf("Áûëî %u, ñòàëî ïîñëå ÷òåíèå èç EEPROM size=%u\r\n", sizeof(prim_par), size_prim_par); 
    /*
    // Íåîáõîäèìà äî ñòàðòà îñíîâíîãî öèêëà ðåàëèçîâàòü ðó÷íóþ êîððåêöèþ äàòû è âðåìåíè ñ àâòîìàòè÷åñêèì âûõîäîì ïî òàéìåðó
    timer1_valcoder = 60;       // Óñòàíîâêà òàéìåðà 30 ñåê
    while(timer1_valcoder) {
         lcd_command(LCD_DISP_ON);       // Óáèðàåì êóðñîð ñ LCD
         lcd_gotoxy(0,0);        // Óñòàíàâëèâàåì êóðñîð â ïîçèöèþ 0 ïåðâîé ñòðîêè
         sprintf(linestr, "Óñò.âðåìåíè");
    }
    */
    lcd_primary_screen();       // âûâîäèì ñòàðòîâóþ êàðòèíêó íà ýêðàí÷èê
    // if (PINC.6) PORTD |= (1<<4); else PORTD &= ~(1<<4);
    // if (PINC.7) PORTD &= ~(1<<5); else PORTD |= (1<<5);
    // if (PINC.5) PORTD &= ~(1<<5); else PORTD |= (1<<5);
    while(1) {
        check_serial();
        // ïðîâåðÿåì è ïðèñâàèâàåì áèòó PORTD.4 ñîñòîÿíèÿ PIND.6
        // if(PIND.6) PORTD &= ~(1<<4); else PORTD |= (1<<4);
        if (event == ev_none) {
            if (!VALCODER_ENTER) {
                // Ïîäàâëåíèå äðåáåçãà Enter
                delay_ms(100);
                if (!VALCODER_ENTER) {  // printf ("Ñãåíåðèðîâàëè íàæàòèå Enter\r\n");
                    event = ev_enter;
                    // Çàïóñêàåì òàéìåð èíàêòèâíîñòè
                    timer1_valcoder = TIMER_INACTIVE;                }
            }
            if (!VALCODER_CANCEL) {
                // Ïîäàâëåíèå äðåáåçãà Cancel
                delay_ms(50);
                if (!VALCODER_CANCEL) { // printf ("Ñãåíåðèðîâàëè íàæàòèå Cancel\r\n");
                    event = ev_cancel;
                    // Çàïóñêàåì òàéìåð èíàêòèâíîñòè
                    timer1_valcoder = TIMER_INACTIVE;                }
            }
            // Îáðàáàòûâàåì ñîáûòèå valcoder'à
            if ((abs(valcoder)-VALCODER_SENSITY) >= 0) {        // Åñëè ñðàáîòàë valcoder
                // printf ("Ñãåíåðèðîâàëè êðó÷åíèå (%i)...\r\n", valcoder);
                if (valcoder < 0) event = ev_left;
                else event = ev_right;
                // Çàïóñêàåì òàéìåð èíàêòèâíîñòè
                timer1_valcoder = TIMER_INACTIVE;
                valcoder = VALCODER_NO_ROTATE;
                // if (valcoder > VALCODER_NO_ROTATE) printf (" -->\r\n"); else printf ("<-- \r\n");
                // printf (" [V = %i, I0 = %u, I1 = %u]\r\n", valcoder, counter0, counter1);
            }
        }
        switch (event) {
            case ev_secunda:                // Îáðàáàòûâàåì åæåñåêóíäíîå ñîáûòèå.
                main_menu[0].val_data = read_term(0);       // Âûâîäèì èíôîðìàöèþ î ãëàâíîì òåðìîìåòðå
                switch (mode.menu) {
                    case 0: lcd_primary_screen(); break;
                    case 1: lcd_menu(0); break;
                    default: ;
                    // case 2: lcd_edit(0); break;
                };
                event = ev_none;            // Î÷èùàåì ñîáûòèå
                break;
            case ev_left:                   // printf ("Îáðàáàòûâàåì ïðîêðóòêó valcoder âëåâî\r\n");
            case ev_right:                  // printf ("Îáðàáàòûâàåì ïðîêðóòêó valcoder âïðàâî\r\n");
                // printf ("Îáðàáàòûâàåì ïðîêðóòêó valcoder (%d), â ðåæèìå %d - ", event-2, mode.menu);
                switch (mode.menu) {
                    case 0: lcd_menu(mode.menu++); break;   // Âûâîäèì ìåíþ áåç èçìåíåíèÿ ïîçèöèè printf ("entering...");
                    case 1: lcd_menu(event-2); break;       // printf ("navigating...");
                    case 2: lcd_edit(event-2); break;       // printf ("editing...");
                    default: ;                              // printf ("defaulting...");
                }
                // printf ("\r\n");
                event = ev_none;            // Î÷èùàåì ñîáûòèå
                break;
            case ev_enter:                  // Åñëè íàæàò Enter
                // LAMP_ECHO_PORT |= (1<<LAMP_ECHO_PIN); timer1_lamp = ENTER_CANCEL_OVERFLOW; if (mode.menu <= 2)
                switch (mode.menu) {
                    // lcd_primary_screen();
                    // Îáðàáàòûâàåì íàæàòèå enter c ó÷åòîì òîãî, ÷òî çíà÷åíèå mode.menu åùå ñòàðîå
                    case 0: lcd_menu(0); ++mode.menu; break;    // Åñëè íàõîäèëèñü â ãëàâíîì ýêðàíå, çàïóñêàåì ïðîðèñîâêó ìåíþ
                    case 1:                                     // Åñëè íàõîäèëèñü â ìåíþ, òî àíàëèçèðóåì
                            lcd_initedit(0); break;
                    case 2: lcd_initedit(1); mode.menu = 1; break;
                };
                event = ev_none;            // Î÷èùàåì ñîáûòèå
                break;
            case ev_timer:
                // Çàïóñêàåì òàéìåð èíàêòèâíîñòè
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
                event = ev_none;            // Î÷èùàåì ñîáûòèå
                break;
            default:
                break;
        };
    }; // while (1)
}
// Ïå÷àòü âñåõ òåðìîìåòðîâ
void printallterms(void) {
    int term;
    register byte i;

    if(!ds1820_devices) return;			// åñëè òåðìîìåòðû íå îáíàðóæåíû - ïðîñòî âûõîäèì èç ôóíêöèè
    printf("\t");						// ïå÷àòàåì çíàê òàáóëÿöèè â òåðìèíàëå
    for(i=0; i<ds1820_devices; i++) {
    	term = read_term(i);
        printf(" t%-u = %-i.%-uC; ", i+1, term/100, abs(term%100));
    }
    printf("\r\n");
}
// ×òåíèå òåìïåðàòóðû (àðãóìåíò - íîìåð òåðìîìåòðà íà÷èíàÿ ñ 0)
int read_term(byte num) {
    float lt;
    lt = termometers[num].scale / 127;
    lt = termometers[num].t * (lt + 1) + termometers[num].offset;
    return (int)lt;
};
// Ïîëíîñòüþ ïðîðèñîâàòü ãëàâíûé ýêðàí
void lcd_primary_screen(void) {
    lcd_command(LCD_DISP_ON);       // Óáèðàåì êóðñîð ñ LCD
    lcd_gotoxy(0,0);        // Óñòàíàâëèâàåì êóðñîð â ïîçèöèþ 0 ïåðâîé ñòðîêè
    sprintf(linestr, "%02u:%02u:%02u %02u.%02u", s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo);
    lcd_puts(linestr);
    lcd_gotoxy(0,1);                // Óñòàíàâëèâàåì êóðñîð â ïîçèöèþ 0 ñòðîêè 2
    // Âûâîäèì èíôîðìàöèþ î òåðìîìåòðàõ
    if(ds1820_devices)
        sprintf(linestr, "t=%-i.%-uC ", main_menu[0].val_data/100, abs(main_menu[0].val_data%100));
    else
        sprintf(linestr, "No termometers");
    lcd_puts(linestr);
}
// Îáðàáîòêà ñîáûòèé îò ñåðèéíîãî ïîðòà
void check_serial(void) {
    byte inbyte;    // Îïèñàíèå ëîêàëüíûõ ïåðåìåííûõ
    register byte i;
    // Îáðàáàòûâàåì ïîñëåäîâàòåëüíûé ïîðò
    if (UCSRA & RX_COMPLETE) { // Ïðèøëî ëè ÷òî-íèáóäü
        inbyte = UDR;
        switch (inbyte) {
            case 0x7A:                // ñèìâîë 'z'
                printf("Âðåìÿ: %02u:%02u:%02u, äàòà:%02u.%02u.%02u, íàéäåíî %u òåðìîìåòðîâ\r\n",
                        s_dt.cHH, s_dt.cMM, s_dt.cSS, s_dt.cdd, s_dt.cmo, s_dt.cyy, ds1820_devices);
                printallterms();
                break;
            case 0x71:                // ñèìâîë 'q'
                print_curr_menu2(1); break;
            case 0x61:                // ñèìâîë 'a'
                print_curr_menu2(-1); break;
            /* case 0x77:                // ñèìâîë 'w'
                // Âõîäèì â ìåíþ (ýìóëÿöèÿ êíîïêè Enter)
                // parameters[12].val_data++; if (parameters[12].val_data > 3) parameters[12].val_data = 0;
                // if (++main_menu[1].val_data > 3) main_menu[1].val_data = 0; // [12] Îñíîâíîé ðåæèì
                // init_curr_menu(&parameters[0], NUM_PARAMETERS);
                break;
            case 0x73:                // ñèìâîë 's'
                // Âûõîäèì èç ìåíþ (ýìóëÿöèÿ êíîïêè Cancel)
                // parameters[12].val_data--; if (parameters[12].val_data < 0) parameters[12].val_data = 3;
                // if (--main_menu[1].val_data < 0) main_menu[1].val_data = 3; // [12] Îñíîâíîé ðåæèì
                // Òåêóùèé óêàçàòåëü óñòàíàâëèâàåì íà ãëàâíîå ìåíþ
                init_curr_menu(&main_menu[0], NUM_MENU);
                break;
            */
            case 0x78:                // ñèìâîë 'x' Ïå÷àòàåì òåêóùåå ìåíþ
                print_curr_menu(); break;
            /* case 0x65:                // ñèìâîë 'e'
                lcd_initedit(0); clatsman.edit_mode = 1;
                break;
            case 0x64:                // ñèìâîë 'd'
                lcd_initedit(1); clatsman.edit_mode = 0;
                break;
            case 0x63:                // ñèìâîë 'c'
                lcd_initedit(-1); clatsman.edit_mode = 0;
                break;*/
            case 0x6D:                // ñèìâîë 'm'
                for(i=0; i<NUM_PARAMETERS; i++) printf("%s\t", param_str(i, parameters));
                printf("\r\n");
                break;
            case 0x6E:                // ñèìâîë 'n'
                for(i=0; i<NUM_MENU; i++) printf("%s\t", param_str(i, main_menu));
                printf("\r\n");
                break;
            default:
                printf("Íàæàò ñèìâîë 0x%x\r\n", inbyte);
        };
    }
}