#include <mega16.h>
#include <delay.h>
#include "robowater.h"
#include "valcoder.h"

byte valcoder0, valcoder1;      // Биты прерываний
word counter0, counter1;        // счетчик кол-ва прерываний от valcoder
int valcoder;                   // Направление вращения

// Внешние прерывания Valcoder'а
interrupt [EXT_INT0] void ext_int0_isr(void) {
    valcoder0 = 1; counter0++;
    // Проверяем, обработали ли прокрутку valcoder'а
    if (valcoder == VALCODER_NO_ROTATE) {
        // Проверяем valcoder2
        if (valcoder1) valcoder = VALCODER_TO_RIGHT;
    }
    delay_ms(VALCODER_INT_DELAY);
    // GIFR |= (1<<6);
}
interrupt [EXT_INT1] void ext_int1_isr(void) {
    valcoder1 = 1; counter1++;
    // Проверяем, обработали ли прокрутку valcoder'а
    if (valcoder == VALCODER_NO_ROTATE) {
        // Проверяем valcoder2
        if (valcoder0) valcoder = VALCODER_TO_LEFT;
    }
    delay_ms(VALCODER_INT_DELAY);
    // GIFR |= (1<<7);
}
