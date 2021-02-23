#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <M5Stack.h>
#include "secrets.h"


#define DISPOSITIVO "enrique"    //Dispositivo que identifica al publicar en MQTT
#define RAIZ "cursocefire/m5stack" //raiz de la ruta donde va a publicar

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
long lastMsgM = 0;
char msg[50];
int value = 0;
int valueM = 0;

//Topics
String topic_root = String(RAIZ) + "/" + String(DISPOSITIVO);
String publish_10sec_string = topic_root + "/dato10s";
const char *publish_10sec = publish_10sec_string.c_str();
String publish_60sec_string = topic_root + "/dato60s";
const char *publish_60sec = publish_60sec_string.c_str();
String publish_reset_string = topic_root + "/reset";
const char *publish_reset = publish_reset_string.c_str();
String subs_led_string = topic_root + "/led";
const char *subs_led = subs_led_string.c_str();
String subs_text_string = topic_root + "/text";
const char *subs_text = subs_text_string.c_str();

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (String(topic) == String(subs_led))
  {
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1')
    {
      //digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
      // but actually the LED is on; this is because
      // it is active low on the ESP-01)
    }
    else
    {
      //digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
    }
  }
}

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266-" + String(DISPOSITIVO) + "-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(publish_reset, "reset");
      // ... and resubscribe
      client.subscribe(subs_led);
      client.subscribe(subs_text);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  M5.begin(true, false, true);
  M5.Power.begin();

  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(65, 10);
  M5.Lcd.println("Button example");
  M5.Lcd.setCursor(3, 35);
  M5.Lcd.println("Press button B for 700ms");
  M5.Lcd.println("to clear screen.");
  M5.Lcd.setTextColor(RED);

  Serial.begin(9600);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void loop() {
  M5.update();

  // if you want to use Releasefor("was released for"), use .wasReleasefor(int time) below
  if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 200))
  {
    M5.Lcd.print('A');
  }
  else if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 200))
  {
    M5.Lcd.print('B');
  }
  else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 200))
  {
    M5.Lcd.print('C');
  }
  else if (M5.BtnB.wasReleasefor(700))
  {
    M5.Lcd.clear(BLACK);
    M5.Lcd.setCursor(0, 0);
  }

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 10000)
  {
    lastMsg = now;
    ++value;
    snprintf(msg, 50, "hello world 10s #%d", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(publish_10sec, msg);
  }

  if (now - lastMsgM > 60000)
  {
    lastMsgM = now;
    ++valueM;
    snprintf(msg, 50, "hello world 60s #%d", valueM);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(publish_60sec, msg);
  }
}