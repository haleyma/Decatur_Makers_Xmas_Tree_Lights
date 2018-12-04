#include <Adafruit_NeoPixel.h>
 
// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

//***************FOR THE BUTTON****************************
int btnPin1 = 8;
int btnPin2 = 7;
int btnPin3 = 6;
int btnPin4 = 5;

int nowState; 
int reading;
int prevState = LOW;
int mode = 0;

long prevTime = 0;
long debounce = 200;

//************************************************************
 
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:
 
    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }
 
    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }
 
    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }
 
    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }
 
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
 
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }
 
    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }
 
    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};
 
void Stick1Complete();
void Stick2Complete();
void Stick3Complete();
 
// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines
NeoPatterns Stick1(25, 10, NEO_GRB + NEO_KHZ800, &Stick1Complete);
NeoPatterns Stick2(25, 11, NEO_GRB + NEO_KHZ800, &Stick2Complete);
NeoPatterns Stick3(50, 12, NEO_GRB + NEO_KHZ800, &Stick3Complete);

 
// Initialize everything and prepare to start
void setup()
{
  Serial.begin(115200);

 // assign all the button pins to INPUT_PULLUP
   pinMode(8, INPUT_PULLUP);
   pinMode(7, INPUT_PULLUP);
   pinMode(6, INPUT_PULLUP);
   pinMode(5, INPUT_PULLUP);

    
    // Initialize all the pixelStrips
    Stick1.begin();
    Stick2.begin();
    Stick3.begin();
    

    Stick1.Fade(Stick1.Color(255,255,0),Stick1.Color(255,255,255), 8, 500);
    Stick2.Fade(Stick2.Color(255,0,0),Stick2.Color(200,50,50), 8, 500);
    Stick3.Fade(Stick3.Color(0,255,0),Stick3.Color(50,200,50), 8, 500); 
   
}
 
// Main loop
void loop()
{
    // Update the rings.
    Stick1.Update();
    Stick2.Update(); 
    Stick3.Update(); 
    
      
    //reading = digitalRead(btnPin1);
    // Switch patterns on a button press:
    int reading1 = digitalRead(btnPin1);
    int reading2 = digitalRead(btnPin2);
    int reading3 = digitalRead(btnPin3);
    int reading4 = digitalRead(btnPin4);
    if (reading1 == 0) // Button #1 pressed
      {
        if((millis() - prevTime) > debounce) 
        {
          prevTime = millis ();
          mode = 0;
        }
      }

     if (reading2 == 0) // Button #2 pressed
      {
        if((millis() - prevTime) > debounce) 
        {
          prevTime = millis ();
          mode = 1;
        }
      }

     if (reading3 == 0) // Button #3 pressed
      {
        if((millis() - prevTime) > debounce) 
        {
          prevTime = millis ();
          mode = 2;
        }
      }

      if (reading4 == 0) // Button #4 pressed
      {
        if((millis() - prevTime) > debounce) 
        {
          prevTime = millis ();
          mode = 3;
        }
      }
      if (mode > 3) mode=0;

      switch(mode)
      {
        case 0: // stick with initialized patterns
                //Stick1.Fade(Stick1.Color(0,255,0),Stick1.Color(255,255,255), 8, 500);
                Stick1.ActivePattern = FADE;
                Stick1.Color1 = (Stick1.Color(255,255,0));
                Stick1.Color2 = (Stick1.Color(255,255,255));
                Stick1.TotalSteps = 8;
                Stick1.Interval = 500;
                
                //Stick2.Fade(Stick2.Color(255,0,0),Stick2.Color(200,50,50), 8, 500);
                Stick2.ActivePattern = FADE;
                Stick2.Color1 = (Stick2.Color(255,0,0));
                Stick2.Color2 = (Stick2.Color(200,50,50));
                Stick2.TotalSteps = 8;
                Stick2.Interval = 500;
                
                //Stick3.Fade(Stick3.Color(0,255,0),Stick3.Color(50,200,50), 8, 500);
                Stick3.ActivePattern = FADE;
                Stick3.Color1 = (Stick3.Color(0,255,0));
                Stick3.Color2 = (Stick3.Color(50,200,50));
                Stick3.TotalSteps = 8;
                Stick3.Interval = 500;
                break;
                
        case 1: // switch to active patterns
                // set Stick one to theater chase
                Stick1.ActivePattern = THEATER_CHASE;
                Stick1.Interval = 500;
            
                //  on Stick2 - change to theaterChase
                Stick2.ActivePattern = THEATER_CHASE;
                Stick2.Interval = 500;
          
                // Set stick3 to  - Scanner
                Stick3.ActivePattern = SCANNER;
                Stick3.Interval = 50;
                Stick3.TotalSteps = 98;
                break;
                
        case 2: // Speed up active patterns
                // set Stick one to theater chase
                Stick1.ActivePattern = THEATER_CHASE;
                Stick1.Interval = 200;
            
                //  on Stick2 - change to theaterChase
                Stick2.ActivePattern = THEATER_CHASE;
                Stick2.Interval = 200;
          
                // Set stick3 to  - Scanner
                Stick3.ActivePattern = SCANNER;
                Stick3.Interval = 10;
                Stick3.TotalSteps = 98;
                break;
                
        case 3: // switch to rainbow cylce
                Stick1.ActivePattern = RAINBOW_CYCLE;
                Stick1.TotalSteps = 255;
                Stick1.Interval = 10;
                
                Stick2.ActivePattern = RAINBOW_CYCLE;
                Stick2.TotalSteps = 255;
                Stick2.Interval= 10;

                Stick3.ActivePattern = RAINBOW_CYCLE;
                Stick3.TotalSteps = 255;
                Stick3.Interval= 10;
                break;
                
        default: //restore to initialized patterns
                //Stick1.Fade(Stick1.Color(0,255,0),Stick1.Color(255,255,255), 8, 500);
                Stick1.ActivePattern = FADE;
                Stick1.Color1 = (Stick1.Color(255,255,0));
                Stick1.Color2 = (Stick1.Color(255,255,255));
                Stick1.TotalSteps = 8;
                Stick1.Interval = 500;
                
                //Stick2.Fade(Stick2.Color(255,0,0),Stick2.Color(200,50,50), 8, 500);
                Stick2.ActivePattern = FADE;
                Stick2.Color1 = (Stick2.Color(255,0,0));
                Stick2.Color2 = (Stick2.Color(200,50,50));
                Stick2.TotalSteps = 8;
                Stick2.Interval = 500;
                
                //Stick3.Fade(Stick3.Color(0,255,0),Stick3.Color(50,200,50), 8, 500);
                Stick3.ActivePattern = FADE;
                Stick3.Color1 = (Stick3.Color(0,255,0));
                Stick3.Color2 = (Stick3.Color(50,200,50));
                Stick3.TotalSteps = 8;
                Stick3.Interval = 500;
                break;
      }
}
//      
        
 
//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------
 
// Stick1 Completion Callback
void Stick1Complete()
{

}
 
// Stick2 Completion Callback
void Stick2Complete()
{

}
 
// Stick3 Completion Callback
void Stick3Complete()
{
    
    
}
