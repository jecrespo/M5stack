#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <M5Stack.h>
#include "secrets.h"

#define DISPOSITIVO "enrique"      //Dispositivo que identifica al publicar en MQTT
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
String publish_A_string = topic_root + "/A";
const char *publish_A = publish_A_string.c_str();
String publish_B_string = topic_root + "/B";
const char *publish_B = publish_B_string.c_str();
String publish_C_string = topic_root + "/C";
const char *publish_C = publish_C_string.c_str();
String subs_led_string = topic_root + "/led";
const char *subs_led = subs_led_string.c_str();
String subs_text_string = topic_root + "/text";
const char *subs_text = subs_text_string.c_str();

void callback(char *topic, byte *payload, unsigned int length)
{
  String mensaje = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    mensaje+=(char)payload[i];
  }
  Serial.println();

  if (String(topic) == String(subs_text))
  {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(3, 90);
    M5.Lcd.fillRect(0, 90, 320, 50, BLACK);
    M5.Lcd.setCursor(3, 90);
    M5.Lcd.println(mensaje);
  }

  if (String(topic) == String(subs_led))
  {
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1')
    {
      M5.Lcd.drawCircle(270, 170, 20, RED);
      M5.Lcd.fillCircle(270, 170, 20, RED);
    }
    else
    {
      M5.Lcd.drawCircle(270, 170, 20, GREEN);
      M5.Lcd.fillCircle(270, 170, 20, GREEN);
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

void setup()
{
  M5.begin(true, false, true);
  M5.Power.begin();

  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(3, 10);
  M5.Lcd.println("MQTT Button Control");
  M5.Lcd.setCursor(3, 35);
  M5.Lcd.println("Press button A, B or C to send message");
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(60, 210);
  M5.Lcd.print("A");
  M5.Lcd.setCursor(150, 210);
  M5.Lcd.print("B");
  M5.Lcd.setCursor(245, 210);
  M5.Lcd.print("C");

  Serial.begin(9600);
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
}

void loop()
{
  M5.update();

  // if you want to use Releasefor("was released for"), use .wasReleasefor(int time) below
  if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 200))
  {
    //M5.Lcd.print('A');
    client.publish(publish_A, "press");
  }
  else if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 200))
  {
    //M5.Lcd.print('B');
    client.publish(publish_B, "press");
  }
  else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 200))
  {
    //M5.Lcd.print('C');
    client.publish(publish_C, "press");
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