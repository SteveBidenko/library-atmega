#include <mega16.h>
#include <delay.h>
#include "robowater.h"
#include "valcoder.h"
#include "menu.h"

byte valcoder0, valcoder1;      // ���� ����������
word counter0, counter1;        // ������� ���-�� ���������� �� valcoder
int valcoder;                   // ����������� ��������

// ������� ���������� Valcoder'�
interrupt [EXT_INT0] void ext_int0_isr(void) {
    valcoder0 = 1; counter0++;
    // ���������, ���������� �� ��������� valcoder'�
    if (valcoder == VALCODER_NO_ROTATE) {
        // ��������� valcoder2
        if (valcoder1) valcoder = VALCODER_TO_RIGHT;
    }
    delay_ms(VALCODER_INT_DELAY);
    // GIFR |= (1<<6);
}
interrupt [EXT_INT1] void ext_int1_isr(void) {
    valcoder1 = 1; counter1++;
    // ���������, ���������� �� ��������� valcoder'�
    if (valcoder == VALCODER_NO_ROTATE) {
        // ��������� valcoder2
        if (valcoder0) valcoder = VALCODER_TO_LEFT;
    }
    delay_ms(VALCODER_INT_DELAY);
    // GIFR |= (1<<7);
}
