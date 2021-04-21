#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <PubSubClient.h>

const int LEFT_CAPTOR = 0;
const int RIGHT_CAPTOR = 1;
const int LED_PIN = 8;

String mainTopic = "BedCaptor";
String ledPowerTopic = mainTopic + "/LedPower";
String ledStateTopic = mainTopic + "/LedState";

Ethernet WSA; // WSAStartup
EthernetClient ethClient;
PubSubClient mqttClient;

String payloadToString(byte *payload, unsigned int length)
{
  char buffer[length];
  sprintf(buffer, "%.*s", length, payload);
  return String(buffer);
}



void switchLed(bool on)
{
  if(on)
  {
    digitalWrite(LED_PIN, 1);
  }
  else
  {
    digitalWrite(LED_PIN, 0);
  }
  if(!mqttClient.publish(ledStateTopic.c_str(), String(on).c_str()))
  {
    Serial.println("Unable to publish led state value..");
  }
}

void actionCallback(char * topicChar, byte* payloadByte, unsigned int length)
{
  Serial.println("New message received");
  String topic = String(topicChar);
  String payload = payloadToString(payloadByte, length);

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  Serial.println(payload);

  if(!topic.equals(ledPowerTopic))
  {
    return;
  }

  if(payload.equalsIgnoreCase("on"))
  {
    switchLed(true);
  }
  else if(payload.equalsIgnoreCase("off"))
  {
    switchLed(false);
  }
}

void setupMqttClient() {
  mqttClient.setClient(ethClient);
  mqttClient.setServer("mqtt.flespi.io",1883);
  mqttClient.setCallback(actionCallback);
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  setupMqttClient();

  analogWrite(LEFT_CAPTOR, 1.17 / 5.0 * 1024.0);
}

void subscribeActionTopic()
{
  if(!mqttClient.subscribe(ledPowerTopic.c_str()))
  {
    Serial.println("Topic led action subscribe error");
    return;
  }
  Serial.println("Topic led action subscribe success");
}

void connectMqtt() {
  if(!mqttClient.connected())
  {
    Serial.println("Trying to connect to mqtt broker");
    if(!mqttClient.connect("CapteurLit", "xdbT3mFXNfdzziF4LQXG9fyvl9DrRW4cX6TQgoCTrFgPLMJmquQpAeILcb2RWUTJ", ""))
    {
      Serial.println("Mqtt broker connection failed");
      return;
    }
    Serial.println("Mqtt broker connection success");
    subscribeActionTopic();
  }
}

float readBedCaptor(int pin)
{
  float analogValue = analogRead(pin);
  return analogValue * 5.0 / 1024.0 * 1000.0;
}

void publishBedCaptor(int pin)
{
  if(!mqttClient.connected())
  {
    Serial.println("Unable to publish bed captor value since mqtt broker insn't connect");
    return;
  }
  float captorValue = readBedCaptor(pin);
  String finalTopic = pin == LEFT_CAPTOR ? mainTopic + "/Left" : mainTopic + "/Right";
  if(!mqttClient.publish(finalTopic.c_str(), String(captorValue).c_str()))
  {
    Serial.println("Unable to publish bed captor value..");
  }
}

void loop() {
  connectMqtt();
  publishBedCaptor(LEFT_CAPTOR);
  publishBedCaptor(RIGHT_CAPTOR);
  mqttClient.loop();
}