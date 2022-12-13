#include <Arduino.h>
//#include <SoftwareSerial.h>

#if !(defined(STM32F0) || defined(STM32F1) || defined(STM32F2) || defined(STM32F3) || defined(STM32F4) || defined(STM32F7) || \
      defined(STM32L0) || defined(STM32L1) || defined(STM32L4) || defined(STM32H7) || defined(STM32G0) || defined(STM32G4) || \
      defined(STM32WB) || defined(STM32MP1) || defined(STM32L5))
#error This code is designed to run on STM32F/L/H/G/WB/MP1 platform! Please check your Tools->Board setting.
#endif

// These define's must be placed at the beginning before #include "STM32TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
// Don't define TIMER_INTERRUPT_DEBUG > 2. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0

#include "STM32TimerInterrupt.h"
#include "debounce.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN PC13 // Pin 33/PB0 control on-board LED_GREEN on F767ZI
#endif

#include "STM32_ISR_Timer.h"

#define TIMER_INTERVAL_MS 100
#define HW_TIMER_INTERVAL_MS 50

HardwareSerial Serial2(PA3, PA2);
HardwareSerial Serial3(PB11, PB10);
//SoftwareSerial myserial(PB5, PB6); // RX, TX

#define DEBUG 0
#define DEBUG1 0

#define patch_back midi_cc(0xb0, 24, 0)
#define patch_forward midi_cc(0xb0, 25, 0)
#define loop_record_start midi_cc(0xb0, 63, 0)
#define loop_off midi_cc(0xb0, 62, 0)
#define loop_on midi_cc(0xb0, 62, 64)
#define loop_play midi_cc(0xb0, 64, 64)
#define loop_stop midi_cc(0xb0, 64, 0)
#define fx1_on midi_cc(0xb0, 48, 64)
#define fx1_off midi_cc(0xb0, 48, 0)
#define delay_on midi_cc(0xb0, 55, 64)
#define delay_off midi_cc(0xb0, 55, 0)
#define ctrl_off midi_cc(0xb0, 72, 0)
#define ctrl_on midi_cc(0xb0, 72, 64)

int i = 0;
byte buffer[254];
byte maxlength = 3;
byte SerIn;
byte SOF = 0xF0;
bool cmd;

enum fbv_led
{
  //! LED A.
  FBV_LED_A = 0x20,
  //! LED B.
  FBV_LED_B = 0x30,
  //! LED C.
  FBV_LED_C = 0x40,
  //! LED D.
  FBV_LED_D = 0x50,
  //! LED Volume.
  FBV_LED_VOL = 0x03,
  //! LED Wah-Wah.
  FBV_LED_WAH = 0x13,
  //! LED_TAP
  FBV_LED_TAP = 0x61
};

struct schalter
{
  bool a = false;
  bool b = false;
  bool c = false;
  bool d = false;
  bool wah = false;
  bool tap = false;
};
schalter sw;

// Init STM32 timer TIM1
STM32Timer ITimer(TIM1);

// Init STM32_ISR_Timer
// Each STM32_ISR_Timer can service 16 different ISR-based timers
STM32_ISR_Timer ISR_Timer;

#define TIMER_INTERVAL_1_MS 1L
#define TIMER_INTERVAL_0_5S 500L
#define TIMER_INTERVAL_1S 1000L
#define TIMER_INTERVAL_1_5S 1500L

void dbg(String str, int i)
{
  Serial2.print(str);
  Serial2.print(" ");
  Serial2.println(i);
}

void TimerHandler()
{
  ISR_Timer.run();
}
void blink_led()
{
  digitalWrite(PC13, !digitalRead(PC13));
}

#include "sub_routines.h"

void check_keys(int key)
{
}

/*****************************
Setup
Put your startup code here
******************************/

void setup()
{
  // put your setup code here, to run once:
  pinMode(PB0, INPUT_PULLUP);
  pinMode(PB1, INPUT_PULLUP);
  pinMode(PB3, INPUT_PULLUP);
  pinMode(PB4, INPUT_PULLUP);
  pinMode(PB5, INPUT_PULLUP);
  pinMode(PB12, OUTPUT);
  pinMode(PC13, OUTPUT);
  //myserial.begin(31250);
  Serial1.begin(31250);
  Serial2.begin(9600);
  Serial3.begin(31250);
  Serial2.println(BOARD_NAME);
  Serial2.println(STM32_TIMER_INTERRUPT_VERSION);
  Serial2.print(F("CPU Frequency = "));
  Serial2.print(F_CPU / 1000000);
  Serial2.println(F(" MHz"));
  fbv_set_led(FBV_LED_A, 0);
  fbv_set_led(FBV_LED_B, 0);
  fbv_set_led(FBV_LED_C, 0);
  fbv_set_led(FBV_LED_D, 0);
  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    Serial2.print(F("Starting ITimer OK, millis() = "));
    Serial2.println(millis());
  }
  else
    Serial2.println(F("Can't set ITimer. Select another freq. or timer"));
  // Just to demonstrate, don't use too many ISR Timers if not absolutely neces
  // You can use up to 16 timer for each ISR_Timer
  ISR_Timer.setInterval(10, debounce);
  ISR_Timer.setInterval(TIMER_INTERVAL_1S, blink_led);
  // ISR_Timer.setInterval(TIMER_INTERVAL_1_5S,  doingSomething3);
}

void loop()
{

  // put your main code here, to run repeatedly:

  if (Serial1.available() > 0)
  {
    while (Serial1.available() > 0)
    {
      SerIn = Serial1.read();
      if ((SerIn == SOF)) //&& (i == 0))
      {
        buffer[i] = SerIn;
      }
      else if (i == 1)
      {
        {
          buffer[i] = SerIn; // Length
          maxlength = i + SerIn;
        }
      }

      if (i == maxlength)
      {
        buffer[i] = SerIn;
        check_buf(buffer, maxlength + 1);
        i = 0;
      }
      else
      {
        buffer[i] = SerIn;
        i++;
      }
      if (DEBUG1)
      {
        Serial2.print("L: ");
        Serial2.print(maxlength);
        Serial2.print(" i: ");
        Serial2.println(i);
        p(SerIn);
      }
    }
  }
  new_state = get_key_state(1 << KEY0);
  if ((new_state == 0) && (button_state != new_state))
  {
    dbg("Button_press", 0);
    button_press = 1;
  }
  if ((new_state == 1) && (button_state != new_state))
  {
    dbg("Button_press", 1);
    button_depress = 1;
  }
  button_state = new_state;
  digitalWrite(PB12, new_state);
}
