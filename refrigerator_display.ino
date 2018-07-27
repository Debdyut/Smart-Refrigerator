//#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library
MCUFRIEND_kbv tft;

#include <TouchScreen.h>
#include <EEPROM.h>


// Neopixel led codes
#include <Adafruit_NeoPixel.h>
#define PIN 13
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 5
int toggle_color;
int r, g, b;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 0; // delay for half a second

//Time storing variable
int last_t;
int cel;

// Temperature sensor
int val;
int tempPin = 5;

#define MINPRESSURE 100
#define MAXPRESSURE 1000

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341
const int TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// EEPROM address
int addr1 = 0;
int addr2 = addr1 + 1;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}


// Include some fonts for the display
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>

// Define colors
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410
#define LIGHT_BLUE 0x87FF

int set_temp;

void setup()
{  
    pinMode(0, OUTPUT);  
    pinMode(12, OUTPUT);

    digitalWrite(0, HIGH);
    digitalWrite(12, HIGH);
    
    // Load last set_temp
    set_temp = EEPROM.read(addr1);
    
    
    if(set_temp <= 0 || set_temp > 20) {
      set_temp = 10;
      EEPROM.write(addr1, set_temp);
            
    }    

    // Load last LED color
    toggle_color = EEPROM.read(addr2);    
    
    if(toggle_color < 0 || toggle_color > 8) {
      toggle_color = 0;
      EEPROM.write(addr2, toggle_color);
            
    }  

    pixels.begin(); // This initializes the NeoPixel library.
    for(int i=0;i<NUMPIXELS;i++){

      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(255,255,255)); // Moderately bright green color.
  
      pixels.show(); // This sends the updated pixel color to the hardware.
  
      delay(delayval); // Delay for a period of time (in milliseconds).
  
    }

    //Serial.begin(9600);
    uint16_t ID = tft.readID();
    if (ID == 0xD3) ID = 0x9481;
    tft.begin(ID);
    tft.setRotation(0);

    // Background color screen
    tft.fillScreen(BLACK);  

    // Title bar
    tft.fillRect(0,0, 239, 40, LIGHT_BLUE);
    showmsgXY(10, 25, 1, &FreeSerifBold12pt7b, "Smart Fridge", 0x001F);          

    // On/Off switch
    //tft.fillRoundRect(155, 5, 80, 30, 10, 0x0400);
    //showmsgXY(180, 27, 1, &FreeSerif12pt7b, "ON", WHITE); 
    tft.fillRoundRect(155, 5, 80, 30, 10, RED);
    showmsgXY(170, 27, 1, &FreeSerif12pt7b, "OFF", WHITE);

    // Set temperature 
    tft.fillTriangle(190, 50, 150, 90, 230, 90, GREEN); // Increase temperature to be set  
    tft.fillTriangle(190, 140, 150, 100, 230, 100, GREEN); // Decrease temperature to be set  
    showmsgXY(30, 180, 1, &FreeSerif12pt7b, "Set Temperature", WHITE); // Label     

    // Separator
    tft.drawLine(10, 200, 230, 200, LIGHT_BLUE);

    // Current temperature label
    showmsgXY(30, 290, 1, &FreeSans9pt7b, "Current", WHITE);  
    showmsgXY(10, 310, 1, &FreeSans9pt7b, "Temperature", WHITE);  

    // Power button section
    tft.fillCircle(180, 250, 40, GREEN);
    showmsgXY(150, 260, 1, &FreeSans9pt7b, "Toggle", BLACK);  
    showmsgXY(145, 310, 1, &FreeSans9pt7b, "LED Color", WHITE);  

    char cstr[16];
    sprintf(cstr, "%02d", set_temp);
    showmsgXY(10, 130, 6, &FreeSans9pt7b, cstr, RED);    
    showmsgXY(70, 230, 2, &FreeSans9pt7b, "o", RED);  
    showmsgXY(80, 260, 2, &FreeSerif12pt7b, "C", RED);     

    int sqsum = 0;
      int sum = 0;
  
      for(int i = 0; i < 10; i++) {
          val = analogRead(tempPin);
          float mv = ( val/1024.0)*5000; 
          cel = floor(mv/10);    

          sqsum += pow(cel, 2);
          sum += cel;

          delay(10);
       }

    float qout = sqsum / 10;
    float avg_temp = sqrt(qout);
    int temp = (int) avg_temp;

    temp = temp - 13;
    
    sprintf(cstr, "%02d", temp);
    showmsgXY(5, 259, 3, &FreeSans9pt7b, cstr, RED);
    last_t = millis();    

}

bool machine_state = false;
int last_temp = 14;

int count = 0;

void loop() {

  int current_t = millis();
  if(current_t - last_t > 1000 * 10) {
      int sqsum = 0;
      int sum = 0;
  
      for(int i = 0; i < 10; i++) {
          val = analogRead(tempPin);
          float mv = ( val/1024.0)*5000; 
          cel = floor(mv/10);    

          sqsum += pow(cel, 2);
          sum += cel;

          delay(10);
       }

    float qout = sqsum / 10;
    float avg_temp = sqrt(qout);
    int temp = (int) avg_temp;
    
    temp = temp - 13;

    char cstr[16];
    sprintf(cstr, "%02d", temp);
    tft.fillRect(5, 220, 65, 50, BLACK);
    showmsgXY(5, 259, 3, &FreeSans9pt7b, cstr, RED);    
    count += 1; 
    last_t = current_t;
    
    if(set_temp > temp && machine_state) {
      digitalWrite(0, HIGH);
    } 
    else if (set_temp + 2 < temp && machine_state)
      digitalWrite(0, LOW);

    if(set_temp - 1 > temp && machine_state) {
      digitalWrite(12, HIGH);
    } 
    else if (set_temp < temp && machine_state)
      digitalWrite(12, LOW);  

  }
  else {
    delay(30); 
  }
  
  // put your main code here, to run repeatedly:  
  if( Touch_getXY() ) 
  {
    //Serial.println("x: " + String(pixel_x) + "; y: " + String(pixel_y)); 

    if( (pixel_x > -5 && pixel_x < 20) && (pixel_y > 200 && pixel_y < 300) )
    {
      if(machine_state) {
        digitalWrite(0, HIGH);
        digitalWrite(12, HIGH);
        tft.fillRoundRect(155, 5, 80, 30, 10, RED);
        showmsgXY(170, 27, 1, &FreeSerif12pt7b, "OFF", WHITE);           
      }
      else {
        tft.fillRoundRect(155, 5, 80, 30, 10, 0x0400);
        showmsgXY(180, 27, 1, &FreeSerif12pt7b, "ON", WHITE);             
      }
      machine_state = !machine_state;
    }
    else if( (pixel_x > 20 && pixel_x < 55) && (pixel_y > 200 && pixel_y < 300) )
    { 
      //Serial.println("+");
      tft.fillRect(10,52, 123, 96, BLACK);
      set_temp += 1;      
      char cstr[16];
      sprintf(cstr, "%02d", set_temp);
      showmsgXY(10, 130, 6, &FreeSans9pt7b, cstr, RED);
      
      EEPROM.update(addr1, set_temp);
    }
    else if( (pixel_x > 65 && pixel_x < 95) && (pixel_y > 200 && pixel_y < 300) )
    {
      //Serial.println("-");
      tft.fillRect(10,52, 123, 96, BLACK);
      set_temp -= 1;      
      char cstr[16];
      sprintf(cstr, "%02d", set_temp);
      showmsgXY(10, 130, 6, &FreeSans9pt7b, cstr, RED);
      
      EEPROM.update(addr1, set_temp);
    }
    else if( (pixel_x > 155 && pixel_x < 225) && (pixel_y > 180 && pixel_y < 300) )
    {
      toggle_color += 1;
      toggle_color %= 8;

      EEPROM.update(addr2, toggle_color);

      change_led_color(toggle_color);
      
    }
     
    
  }  
  
}


// Function to change led color
void change_led_color(int toggle_color) {

  switch (toggle_color) {
        case 0:
          //do something when var equals 1
          r = 148;
          g = 0;
          b = 211;
          break;
        case 1:
          r = 75;
          g = 0;
          b = 130;
          break;
        case 2:
          r = 0;
          g = 0;
          b = 255;
          break;
        case 3:
          r = 0;
          g = 255;
          b = 0;
          break;
        case 4:
          r = 255;
          g = 255;
          b = 0;
          break;
        case 5:
          r = 255;
          g = 127;
          b = 0;
          break;
        case 6:
          r = 255;
          g = 0;
          b = 0;
          break;
        case 7:
        default:
          r = 255;
          g = 255;
          b = 255;
          break;         
      }


      // Change the color of the LED to selected color
      for(int i=0;i<NUMPIXELS;i++) {

        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(r, g, b)); // Moderately bright green color.
    
        pixels.show(); // This sends the updated pixel color to the hardware.
    
        delay(delayval); // Delay for a period of time (in milliseconds).
    
      }

}

// Function to display text to screen 
void showmsgXY(int x, int y, int sz, const GFXfont *f, const char *msg, unsigned long color)
{
    int16_t x1, y1;
    uint16_t wid, ht;    
    tft.setFont(f);
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(sz);
    tft.print(msg);
    delay(1000);
}
