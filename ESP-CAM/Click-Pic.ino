/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card

  IMPORTANT!!!
   - Select Board "ESP32 Wrover Module"
   - Select the Partion Scheme "Huge APP (3MB No OTA)
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "dl_lib.h"
#include "driver/rtc_io.h"
#include "soc/timer_group_struct.h" // Disable watchdog timer
#include "soc/timer_group_reg.h"    // Disable watchdog timer
#include <AsyncMqttClient.h>
#include <WiFi.h>
#include <C:\Users\AJ_Khalid\Documents\Arduino\libraries\arduino-esp32-master\tools\sdk\include\mbedtls\mbedtls\base64.h>

// Connection timeout;
#define CON_TIMEOUT   10*1000                     // milliseconds

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


// MQTT Broker configuration
#define MQTT_HOST     "farmer.cloudmqtt.com"
#define MQTT_PORT     16930
#define USERNAME      "puycbalx"
#define PASSWORD      "So2IRqdJlmCg"
#define TOPIC_PIC     "ESPCAM"


int pictureNumber = 1;

// MQTT callback
AsyncMqttClient mqttClient;
TimerHandle_t   mqttReconnectTimer;
TimerHandle_t   wifiReconnectTimer;

camera_fb_t * fb = NULL;
bool published = false;

// Update these with values suitable for your network.

const char* ssid = "";
const char* password = "";

bool connectMQTT() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
  int i = 0;
  while ( !mqttClient.connected() && i < 9 )
  {
    delay(250);
    Serial.print(".");
    i++;
  }
  
  if ( !mqttClient.connected() )
  {
    Serial.println("Failed to connect to MQTT Broker");
    return false;
  }
  else
  {
    Serial.println("Connected to MQTT Broker");
    return true;
  }
}

void onMqttConnect(bool sessionPresent)
{
  // Take picture
  capture();

  // Publish picture
  const char* pic_buf = (const char*)(fb->buf);
  size_t length = fb->len;


  uint16_t packetIdPubTemp = mqttClient.publish( "/tasweerName", 0, false, "Weight_Object.jpg" );
  if ( !packetIdPubTemp  )
  {
    Serial.println( "Sending Failed! err: " + String( packetIdPubTemp ) );
  }
  else
  {
    Serial.println("MQTT Publish succesful");
    Serial.println("Picture name published.");
  }

  unsigned char  image[9000];

  size_t olen;

  int err = mbedtls_base64_encode(image, sizeof(image), &olen, fb->buf, length);
  String img((const __FlashStringHelper*) image);
  Serial.println("buffer is " + String(img.length()) + " bytes");
  int packet_size = 5000;
  String packets = "";
  float pack = float(img.length()) / packet_size;
  if ( float(pack / int(pack)) > 1)
  {
    packets = String(int(pack) + 1);
  }
  else
  {
    packets = String(int(pack));
  }
  Serial.println("Total number of packets is: " + packets);
  packetIdPubTemp = mqttClient.publish( "/totalPackets", 0, false, packets.c_str());
  if ( !packetIdPubTemp  )
  {
    Serial.println( "Sending Failed! err: " + String( packetIdPubTemp ) );
  }
  else
  {
    Serial.println("MQTT Publish succesful");
    Serial.println("Number of packets published.");
  }

  String subs;
  int i = 0;
sendimage:
  subs = "";
  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed = 1;
  TIMERG0.wdt_wprotect = 0;
  if (i < packets.toInt() - 1)
  {
    for (int j = 0; j < packet_size; j++)
    {
      subs += img[j + i * packet_size];
    }
  }
  else
  {
    for (int j = 0; j < img.length() - i * packet_size; j++)
    {
      subs += img[j + i * packet_size];
    }
  }
  Serial.println("Packet size: " + String(subs.length()) + " of total size: " + String(img.length()));
  packetIdPubTemp = mqttClient.publish( "/tasweer", 0, false, subs.c_str(), subs.length());


  if ( !packetIdPubTemp  )
  {
    Serial.println( "Sending Failed! err: " + String( packetIdPubTemp ) );
  }
  else
  {
    Serial.println("Packet " + String(i + 1) + " of total " + packets + " published.");
  }
  delay(800);
  i++;
  if (i < packets.toInt())
  {
    goto sendimage;
  }
  if ( packetIdPubTemp  )
  {
    Serial.println("MQTT Publish succesful");
    published = true;
  }

  // No delay result in no message sent.
  delay(200);
  esp_camera_fb_return(fb);
}

void sleep()
{
  published = false;
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);
  Serial.println("Going to sleep now");
  delay(2000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void save2sd()
{
  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) + ".jpg";

  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();
}

void capture()
{
  // Take Picture with Camera
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  //save2sd();

  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);
  delay(2000);
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //Serial.println("Starting SD Card");
  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    return;
  }

  // COnfigure MQTT Broker and callback
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectMQTT));
  mqttClient.setCredentials( USERNAME, PASSWORD );
  mqttClient.onConnect (onMqttConnect );
  mqttClient.setServer( MQTT_HOST, MQTT_PORT );
  connectMQTT();
}

void loop()
{
  if (published)
  {
    sleep();
  }
  else
  {
    connectMQTT();
  }
}
