#ifndef __ROBOWATER__
#define __ROBOWATER__
#define F_CPU 8000000UL
// �������� ����� �������� "�" ������� � ����������
#define RXC     7            // ���� ���������� ������ USART
#define TXC     6            // ���� ���������� �������� USART
#define UDRE    5            // ���� "������� ������ USART ����"
#define FE      4            // ���� ������ ������������
#define OVR     3            // ���� ������������
#define UPE     2            // ���� ������ �������� �������� USART
#define MPCM    0            // ����� ������������������� ������
// �������� ����� �������� "B" ������� � ����������
#define RXCIE   7            // ���������� ���������� �� ���������� ������
#define TXCIE   6            // ���������� ���������� �� ���������� ��������
#define RXEN    4            // ���������� ������
#define TXEN    3            // ���������� ��������
#define UCSZ2   2            // ��������� 9-�� ������� ������
#define RXB8    1            // ������ ����� ������ ��������� ������
#define TXB8    0            // ������ ����� ������ ������ ��������.
// �������� �������� ���������������� ������
#define FRAMING_ERROR	(1<<FE)
#define PARITY_ERROR    (1<<UPE)
#define DATA_OVERRUN    (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define TX_COMPLETE 	(1<<UDRE)
#define RX_COMPLETE 	(1<<RXC)
// maximum number of DS1820 devices connected to the 1 Wire bus
#define MAX_DS1820 8
#define VALCODER_TIMER_OVERFLOW 20      // �������� � ���-�� ������������ ����������
#define ENTER_CANCEL_OVERFLOW 2      // �������� � ���-�� ������������ ����������
#define STRLENGTH 16            // ����� ������ ���������
#define LAMP_ECHO_PORT PORTD
#define LAMP_ECHO_PIN 4
// �������� ����� ����������
typedef unsigned char 	byte;	// byte = unsigned char
typedef unsigned int 	word;	// word = unsigned int
typedef char str_val[STRLENGTH];
// �������� c�������
struct st_datetime {
    byte cHH, cMM, cSS;         // ������� �����
    byte cyy, cmo, cdd;         // ������� ����
};
// ��������� �������� �������������� � �������
struct st_clatsman {
    byte alarm:1;           // 0) ������� (0 - ��� �������, 1 - �������)
    byte setting:2;         // 1) ����� ������� �������� (00 - ��������� �������� �����������, 01 - ��������� ����)
                            //    10 - ��������� �������, 11 - ��������� ���� ��������
    byte valcoder_mode:1;   // 2) ����� valcoder (1 - valcoder �������, 0 - �������� �� �������)
    byte enter:1;           // 3) ����� enter �� valcoder (0 - �����, 1 - �����)
    byte cancel:1;          // 4) ������ ������ cancel (0 - �����, 1 - �����)
    byte edit_mode:1;       // 5) ����� ��������������
};
/*
// ��������� ������� � ����
struct st_menupos {
    word primposmenu:4;     // 1) ������� ��������� ���� (��� 16)
    word run11:2;           // 2) ������� ��������� ������ (��� 4)
    word settingmenu:5;     // 3) ������� ���� �������� (��� 32)
    word rulemenu:3;        // 4) ������� ���� ��������� ������ ������������� (��� 8)
    word alarmmenu:4;       // 5) ������� ���� ������ (��� 16)
};
*/
extern void init(void);
extern int read_term(byte);
// �������� ���������� ����������
extern byte ds1820_devices;
extern byte ds1820_rom_codes[MAX_DS1820][9];
extern byte prn;
extern struct st_datetime s_dt;
// extern struct st_menupos menupos;
extern struct st_clatsman clatsman;
extern byte timer1_valcoder;
extern byte timer1_lamp;
#endif
