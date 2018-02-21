/*
  TwoPixel

  This example shows intensity levels on two groups of RGB LED's using the analogWrite()
  function. 

  The analogWrite() function uses PWM pins ~3 (red), ~5 (green), ~6 (blue), ~9 (red), 
  ~10 (green), and ~11 (blue).

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Fade

  On RPUlux the PWM pins each go to an AL8805 that can drive power LED's.
*/

#define NUM_PIXEL 2
#define NUM_COLOR 3
#define INTENSITY_LVL 9

struct Pixel {
  struct pin_t {
    uint8_t pin;
  } color[NUM_COLOR];  
};

static const struct Pixel pixel[NUM_PIXEL] = {3,5,6,9,10,11};

// each level doubles the amount of light output (0 is off).
static const uint8_t intensity2pwm[INTENSITY_LVL] = {
    [0] = 0, 
    [1] = 1,
    [2] = 3,
    [3] = 7,
    [4] = 15,
    [5] = 31,
    [6] = 63,
    [7] = 127,
    [8] = 255,
};

// the setup routine runs once when you press reset:
void setup() 
{
  // declare the pwm pins to be an output:
  pinMode(pixel[0].color[0].pin, OUTPUT);
  pinMode(pixel[0].color[1].pin, OUTPUT);
  pinMode(pixel[0].color[2].pin, OUTPUT);
  pinMode(pixel[1].color[0].pin, OUTPUT);
  pinMode(pixel[1].color[1].pin, OUTPUT);
  pinMode(pixel[1].color[2].pin, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  
  //run each pin one at a time
  for(uint8_t i=0; i <NUM_PIXEL; i++)
  {
    for(uint8_t j=0; j <3; j++) 
    {
      for(int8_t k=0; k<INTENSITY_LVL; k++)
      {
        analogWrite(pixel[i].color[j].pin, intensity2pwm[k]);
        delay(300);
      }
      for(int8_t k= INTENSITY_LVL-1; k>=0; k--)
      {
        analogWrite(pixel[i].color[j].pin, intensity2pwm[k]);
        delay(300);
      }
    }
  }
}
