//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
void midi_cc(int cmd, int pitch, int velocity)
{
  Serial3.write(cmd);
  Serial3.write(pitch);
  Serial3.write(velocity);
}

void fbv_set_led(uint8_t led, bool state)
{
  int i;
  //  dbg("fbv: set led %02X to %d\r\n", led, state);

  Serial1.write(0xf0);
  Serial1.write(3); // length
  Serial1.write(4); // cmd
  Serial1.write(led);
  Serial1.write(state);
}

void p(byte X)
{
  char buff[2];

  sprintf(buff, "%02x", X);

  Serial2.print("0x");
  Serial2.print(buff);
  Serial2.print(" ");
}

void check_buf(byte buffer[64], int length)
{
  int i;
  if (DEBUG)
  {
    // Serial2.println("Check");
    for (i = 0; i < length; i++)
    {
      p(buffer[i]);
    }
  }
  if (buffer[0] != 0xf0)
  {
    if (DEBUG)
    {
      Serial2.println("No Sync..");
    }
    return;
  }

  switch (buffer[2])
  {
  case 0x90:
    Serial2.println("KeepAlive");
    break;
  case 0x81:
    Serial2.print("Schalter-");
    switch (buffer[3])
    {
    case 0x20:
      Serial2.println("A");
      if (buffer[4])
      {
        if (!sw.a) // same as: (buttonState == false
        {
          sw.a = true;
          patch_forward;

          if (true)
          {
            Serial2.println("Patch forward");
          }
        }
        else
        {
          sw.a = false;
          patch_back;
          if (true)
          {
            Serial2.println("Patch Backward");
          }
        }
        Serial2.print("LED_A: ");
        Serial2.println(sw.a);
        fbv_set_led(FBV_LED_A, sw.a);
      }
      break;
    case 0x30:
      Serial2.println("B");

      if (buffer[4])
      {
        if (!sw.b) // same as: (buttonState == false
        {
          sw.b = true;
          loop_record_start;
        }
        else
        {
          sw.b = false;
          loop_stop;
        }
        Serial2.print("LED_B ");
        Serial2.println(sw.b);
        fbv_set_led(FBV_LED_B, sw.b);
      }
      break;
    case 0x40:
      Serial2.println("C");
      if (buffer[4])
      {
        if (!sw.c) // same as: (buttonState == false
        {
          sw.c = true;
          fx1_on;
        }
        else
        {
          sw.c = false;
          fx1_off;
        }
        Serial2.print("LED_C: ");
        Serial2.println(sw.c);
        fbv_set_led(FBV_LED_C, sw.c);
      }
      break;
    case 0x43:
      if (DEBUG)
      {
        Serial2.println("Wah/Volume");
      }
      if (buffer[4])
      {
        if (!sw.wah) // same as: (buttonState == false
        {
          sw.wah = true;
        }
        else
        {
          sw.wah = false;
        }
        Serial2.print("LED_WAH: ");
        Serial2.println(sw.wah);
        fbv_set_led(FBV_LED_WAH, sw.wah);
      }
      break;
    case 0x50:
      if (DEBUG)
      {
        Serial2.println("D");
      }
      if (buffer[4])
      {
        if (!sw.d) // same as: (buttonState == false
        {
          sw.d = true;
          ctrl_on;
        }
        else
        {
          sw.d = false;
          ctrl_off;
        }
        Serial2.print("LED_D: ");
        Serial2.println(sw.d);
        fbv_set_led(FBV_LED_D, sw.d);
      }
      break;
    case 0x61:
      if (DEBUG)
      {
        Serial2.println("Tap");
      }
      if (buffer[4])
      {
        if (!sw.tap) // same as: (buttonState == false
        {
          sw.tap = true;
        }
        else
        {
          sw.tap = false;
        }
        Serial2.print("LED_TAP: ");
        Serial2.println(sw.tap);
        fbv_set_led(FBV_LED_TAP, sw.tap);
        sw.tap = false;
        fbv_set_led(FBV_LED_TAP, sw.tap);
      }
      break;
    default:
      Serial2.println("Unknow");
    }
    break;
  case 0x82:
    dbg("Wah/Volume", buffer[4]);
    midi_cc(0xb0, 11, buffer[4]);
    break;
  default:
    Serial2.println("Unknown");
  }
}
