#include <WiFi.h>
#include <AsyncMqttClient.h>

// Connection timeout;
#define CON_TIMEOUT   10*1000                     // milliseconds

// MQTT Broker configuration and Wi-Fi

const char* ssid = "PTCL-Baj";
const char* password = "lenovo123455";
// MQTT Broker configuration
#define MQTT_HOST     "farmer.cloudmqtt.com"
#define MQTT_PORT     16930
#define USERNAME      "puycbalx"
#define PASSWORD      "So2IRqdJlmCg"

// MQTT callback
AsyncMqttClient mqttClient;
TimerHandle_t   mqttReconnectTimer;
TimerHandle_t   wifiReconnectTimer;

bool published = false;
void sleep()
{
  published = false;
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);
  Serial.println("Going to sleep now");
  delay(2000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

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
  uint16_t packetIdPubTemp = mqttClient.publish( "takepic", 0, false, "Yes");
  if ( !packetIdPubTemp  )
  {
    Serial.println( "Sending Failed! err: " + String( packetIdPubTemp ) );
  }
  else
  {
    Serial.println("MQTT Publish succesful");
    published = true;
  }
}

void setup()
{

  Serial.begin(115200);

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

  // COnfigure MQTT Broker and callback
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectMQTT));
  mqttClient.setCredentials( USERNAME, PASSWORD );
  mqttClient.onConnect (onMqttConnect );
  mqttClient.setServer( MQTT_HOST, MQTT_PORT );


  String topic = "takepic";
}


void loop()
{
  connectMQTT();
  if (published)
    sleep();
}
