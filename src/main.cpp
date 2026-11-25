#include <WiFi.h>
#include "time.h"
#include "Adafruit_NeoPixel.h"
#include "LittleFS.h"
#include "ESPAsyncWebServer.h"
// If you are having trouble with ESPAsyncWebServer, search it from Library Manager install it from ESP32Async

#define PIN 6
#define NUMPIXELS 90

#define SPRING 0
#define SUMMER 1
#define FALL 2
#define WINTER 3

char *seasons;
char *holidays;

uint8_t current_minute;
uint8_t current_hour;
int8_t current_dst;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// The values go by a GRB sequence. Some colors are much brighter than others so if you want to change any of the colors, be mindful of that.
// The more values being used causes the LED to need to use more power-> more hotter. Generally try not to use a lot of values.
const uint32_t red = pixels.Color(0, 100, 0);
const uint32_t redorange = pixels.Color(94, 237, 17);
const uint32_t orange = pixels.Color(50, 100, 0);
const uint32_t yellow = pixels.Color(170, 170, 30);
const uint32_t green = pixels.Color(50, 0, 0);
const uint32_t lightgreen = pixels.Color(70, 20, 20);
const uint32_t blue = pixels.Color(0, 0, 150);
const uint32_t darkblue = pixels.Color(10, 60, 175);
const uint32_t teal = pixels.Color(190, 6, 90);
const uint32_t purple = pixels.Color(0, 120, 100);
const uint32_t lightpurple = pixels.Color(50, 100, 100);
const uint32_t pink = pixels.Color(0, 255, 127);
const uint32_t lightpink = pixels.Color(120, 175, 125);
const uint32_t white = pixels.Color(150, 150, 150);
const uint32_t black = pixels.Color(0, 0, 0);

const uint32_t season_color[4][2]{
    {red, green},   // SPRING
    {red, orange},  // FALL
    {blue, white},  // WINTER
    {purple, blue}, // SUMMER

};

uint32_t colors[2] = {black, black};

// REPLACE WITH ABOVE
uint32_t color_A = black;
uint32_t color_B = black;

const char *ssid = "EngineeringSubNet";
const char *wifipw = "password";

// IMPORTANT: Do not edit or delete anything from these matrices. These are the LEDs that light up the corresponding words.
int hourWords[25][2] = {
    {0, 0},   // placeholder
    {29, 32}, // one
    {37, 40}, // two
    {40, 45}, // three
    {49, 53}, // four
    {45, 49}, // five
    {53, 56}, // six
    {56, 61}, // seven
    {65, 70}, // eight
    {61, 65}, // nine
    {81, 84}, // ten
    {75, 81}, // eleven
    {70, 76}, // twelve
    // Start of 13-24
    {29, 32}, // one
    {37, 40}, // two
    {40, 45}, // three
    {49, 53}, // four
    {45, 49}, // five
    {53, 56}, // six
    {56, 61}, // seven
    {65, 70}, // eight
    {61, 65}, // nine
    {81, 84}, // ten
    {75, 81}, // eleven
    {70, 76}  // twelve
};

// IMPORTANT: Do not edit or delete anything from these matrices. These are the LEDs that light up the corresponding words.
int minuteWords[12][2] = {
    {0, 0},   // zero
    {25, 29}, // five
    {16, 19}, // ten (in minutes)
    {8, 16},  // a quarter
    {19, 25}, // twenty
    {19, 29}, // twenty five
    {4, 8},   // half
    {19, 29}, // twenty five
    {19, 25}, // twenty
    {8, 16},  // a quarter
    {16, 19}, // ten (in minutes)
    {25, 29}  // five
};

int helpers[4][2] = {
    {0, 4},   // IT IS
    {84, 90}, // OCLOCK
    {32, 34}, // TO
    {33, 37}  // PAST
};

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String processor(const String &var)
{
  Serial.println(var);

  if (var == "SEASONS")
  {
    return seasons;
  }
  if (var == "HOLIDAY")
  {
    return holidays;
  }
  return String();
}

void setTimezone(String timezone)
{
  Serial.printf("  Setting Timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

void initTime(String timezone)
{
  struct tm timeinfo;

  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org"); // First connect to NTP server, with 0 TZ offset
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}

void getCurrentTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time 1");
    return;
  }
  current_hour = timeinfo.tm_hour;
  current_minute = timeinfo.tm_min;
  current_dst = timeinfo.tm_isdst;

  Serial.printf("Time: %u:%u DST = %d\n", current_hour, current_minute, current_dst);
}

void set_time()
{
  int i;
  uint8_t fractional_minute = current_minute / 5;
  uint8_t clock_hour = current_hour;

  pixels.clear(); // Set all pixel colors to 'off' which is not 0xFF

  // print "it is"
  for (i = helpers[0][0]; i < helpers[0][1]; i++)
  {
    pixels.setPixelColor(i, 0xFF); // Sets all pixels that need to be turned on to black
  }
  // print "past"
  if ((fractional_minute <= 6) && (fractional_minute != 0))
  {

    for (i = helpers[3][0]; i < helpers[3][1]; i++)
    {
      pixels.setPixelColor(i, 0xFF);
    }
  }
  // print "to"
  if ((fractional_minute > 6) && (fractional_minute != 0))
  {
    clock_hour = current_hour + 1;
    for (i = helpers[2][0]; i < helpers[2][1]; i++)
    {
      pixels.setPixelColor(i, 0xFF);
    }
  }
  // print "oclock"
  if (fractional_minute == 0)
  {
    for (i = helpers[1][0]; i < helpers[1][1]; i++)
    {
      pixels.setPixelColor(i, 0xFF);
    }
  }
  // print "five"
  if ((fractional_minute == 5) || (fractional_minute == 7))
  {
    for (i = helpers[4][0]; i < helpers[4][1]; i++)
    {
      pixels.setPixelColor(i, 0xFF);
    }
  }
  // print "hour"
  for (int i = hourWords[clock_hour][0]; i < hourWords[clock_hour][1]; i++)
  {
    pixels.setPixelColor(i, 0xFF);
  }
  // print "minute"
  for (int i = minuteWords[fractional_minute][0]; i < minuteWords[fractional_minute][1]; i++)
  {
    pixels.setPixelColor(i, 0xFF);
  }
}

void print_clock()
{
  int blue;
  for (int i = 0; i <= 90; i++)
  {
    if (pixels.getPixelColor(i) == 0xFF)
    {
      if (i % 2)
        pixels.setPixelColor(i, colors[0]);
      else
        pixels.setPixelColor(i, colors[1]);
    }
    else
    {
      pixels.setPixelColor(i, black);
    }
  }
  pixels.show();
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  // startWifi();
  WiFi.begin(ssid, wifipw);
  Serial.println("Connecting Wifi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Wifi RSSI=");
  Serial.println(WiFi.RSSI());

  // Print ESP32 Local IP Address
  Serial.print("Local IP =");
  Serial.println(WiFi.localIP());

  initTime("EST5EDT,M3.2.0,M11.1.0"); // Set for NY
  getCurrentTime();

  colors[0] = red;
  colors[1] = orange;
  // REMOVE
  color_A = red;
  color_B = orange;

  if (!LittleFS.begin())
  {
    Serial.println("Failed to initialize LittleFS");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
              request->send(LittleFS, "/index.html", "text/html");
            });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/style.css", "text/css"); });

  // THIS IS REALLY BAD CODE BUT I DONT HAVE TIME TO FIX
  server.on("/Spring", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  colors[0] = season_color[SPRING][0];
  colors[1] = season_color[SPRING][1];
  request->send(LittleFS, "/index.html", String(), false, processor); });

  server.on("/Fall", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  colors[0] = season_color[FALL][0];
  colors[1] = season_color[FALL][1];
  request->send(LittleFS, "/index.html", String(), false, processor); });

  server.on("/Winter", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  colors[0] = season_color[WINTER][0];
  colors[1] = season_color[WINTER][1];
  request->send(LittleFS, "/index.html", String(), false, processor); });

  
    server.on("/Summer", HTTP_GET, [](AsyncWebServerRequest *request) {
   
  colors[0] = season_color[SUMMER][0];
  colors[1] = season_color[SUMMER][1];
  request->send(LittleFS, "/index.html", String(), false, processor);
  });
  
    

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  getCurrentTime();
  // current_hour = 8;
  // current_minute = 46;

  // int counter = current_minute;
  // if ((counter % 5) == 0)
  //{
  // fractional_minute = current_minute / 5;
  // current_minute = counter / 5;
  set_time();
  print_clock();
  delay(1000);
  //}
}
