#include <Arduino.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EthernetClient.h>

Ethernet WSA; // WSAStartup
EthernetClient ethClient;
IPAddress server(192, 168, 1, 25);

#define LED_PIN 3

String payloadToString(byte *payload, unsigned int length)
{
  char buffer[length];
  sprintf(buffer, "%.*s", length, payload);
  return String(buffer);
}

void callback(char *topic, byte *payload, unsigned int length)
{

  String message = payloadToString(payload, length);
  if(message.equals("Coucou"))
  {
    Serial.println(message);
    digitalWrite(LED_PIN, HIGH);
  }
}

PubSubClient mqtt(server, 1883, callback, ethClient);

void setup () {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);


  pinMode(10, INPUT);

  //Pre
  digitalWrite(10, 80);
  analogWrite(0, 80);
}

void reconnect()
{
  while (!mqtt.connected())
  {
    Serial.println("[MQTT] Connecting to MQTT...");
    if (mqtt.connect("MonObjectConnecte")) // Or mqtt.connect("MonObjectConnecte", "user", "pass") if you have protect the mqtt broker by credentials
    {
      Serial.println("[MQTT] Connected");
      mqtt.publish("EssaieIOT", "Hello world from WizIO");
      mqtt.subscribe("EssaieIOT");
    }
    else
    {
      Serial.print("[ERROR] MQTT Connect: ");
      Serial.println(mqtt.state());
      delay(60 * 1000);
    }
  }
}

void loop() {
  if (!mqtt.connected())
  {
    reconnect();
  }
  mqtt.loop();

  int test = digitalRead(10);
  Serial.println(test);
}
