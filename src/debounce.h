#include <Arduino.h>

/********************************** Debounce Variables **************/
#define KEY_PIN GPIOB->IDR & 0x3B

#define KEY0 0
#define KEY1 1
#define KEY2 3
#define ALL_KEYS (1 << KEY0 | 1 << KEY1 | 1 << KEY2)

#define REPEAT_MASK (1 << KEY0 | 1 << KEY1) // repeat: key1, key2
#define REPEAT_START 50                     // after 500ms
#define REPEAT_NEXT 10                      // every 200ms

volatile uint8_t key_state; // debounced and inverted key state:
                            // bit = 1: key pressed
volatile uint8_t key_press; // key press detect

volatile uint8_t key_rpt; // key long press and repeat

#define sw1 0
#define sw2 1
#define sw3 2
#define sw4 3
bool key0_state = false;
bool action = false;

int button_state;
int button_press;   // is set for an instant
int button_depress; // is set for an instant

long new_state = 0;
long new_value = 0;

/********************************************************************/
///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed. Each pressed key is reported
// only once
//
uint8_t get_key_press(uint8_t key_mask)
{
  key_mask &= key_press; // read key(s)
  key_press ^= key_mask; // clear key(s)
  return key_mask;
}
///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed long enough such that the
// key repeat functionality kicks in. After a small setup delay
// the key is reported being pressed in subsequent calls
// to this function. This simulates the user repeatedly
// pressing and releasing the key.
//
uint8_t get_key_rpt(uint8_t key_mask)
{
  key_mask &= key_rpt; // read key(s)
  key_rpt ^= key_mask; // clear key(s)
  return key_mask;
}

///////////////////////////////////////////////////////////////////
//
// check if a key is pressed right now
//
uint8_t get_key_state(uint8_t key_mask)

{
  key_mask &= key_state;
  return key_mask;
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_key_short(uint8_t key_mask)
{
  return get_key_press(~key_state & key_mask);
}

///////////////////////////////////////////////////////////////////
//
uint8_t get_key_long(uint8_t key_mask)
{
  return get_key_press(get_key_rpt(key_mask));
}

void debounce()
{
  static uint8_t ct0, ct1, rpt; // KEY-PIN will becalled to read in key button pressed events
  uint8_t i;

  i = key_state ^ ~KEY_PIN;           // key changed ?
  ct0 = ~(ct0 & i);                   // reset or count ct0
  ct1 = ct0 ^ (ct1 & i);              // reset or count ct1
  i &= ct0 & ct1;                     // count until roll over ?
  key_state ^= i;                     // then toggle debounced state
  key_press |= key_state & i;         // 0->1: key press detect
  if ((key_state & REPEAT_MASK) == 0) // check repeat function
    rpt = REPEAT_START;               // start delay
  if (--rpt == 0)
  {
    rpt = REPEAT_NEXT; // repeat delay
    key_rpt |= key_state & REPEAT_MASK;
  }
}