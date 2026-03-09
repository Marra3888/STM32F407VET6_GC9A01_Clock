//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Это программное обеспечение распространяется свободно. Вы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал
// "Электроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Автор: Надыршин Руслан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "random_move.h"

#define StateChanging	0
#define StateWait		1
#define WaitTime 		10

static uint8_t State = StateChanging;
static int16_t newPosition = 0;
static int16_t position = 0;
static uint16_t StateCounter;

int16_t GetCurrentPos(int16_t min, int16_t max) {
	switch (State) {
	case StateChanging:
		if (position == newPosition) {
			State = StateWait;
			StateCounter = WaitTime;
		} else {
			int16_t dPosition = position - newPosition;
			if (dPosition > 40)
				position -= 6;
			else if (dPosition > 10)
				position -= 4;
			else if (dPosition > 0)
				position--;
			else if (dPosition < -40)
				position += 6;
			else if (dPosition < -10)
				position += 4;
			else if (dPosition < 0)
				position++;
		}
		break;
	case StateWait:
		if (!--StateCounter) {
			State = StateChanging;
			newPosition = (rand() + min) % max;
		}
		break;
	}

	return position;
}
