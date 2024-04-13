
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPRotary.h>
#include <Ticker.h>
#include <iterator>
#include <string>

#include "esp_utils/config/mqtt_config.h"
#include "esp_utils/config/wifi_config.h"
#include "esp_utils/helpers/mqtt.h"
#include "esp_utils/helpers/ota.h"
#include "esp_utils/helpers/utils.h"

#include "config/DRV8220DSG_config.h"
#include "config/config.h"
#include "config/encoder_config.h"
#include "state_machine.h"

WiFiClient wifi_;
PubSubClient mqtt_(mqtt::server_ip, mqtt::server_port, wifi_);
ESPRotary encoder_;
StateMachine state_machine(drv8220dsg::idle, drv8220dsg::stop, drv8220dsg::down, drv8220dsg::up);
Ticker state_machine_loop, publisher_loop, reconnect_loop;

void check_connection();
void reconnect_if_needed();

std::map<std::string, std::function<void(byte * payload, unsigned int length)>> handle_topic{
{MQTT_TOPIC + "/set",
 [](byte * string, unsigned int length) {
     std::string parsed(reinterpret_cast<char *>(string), length);
     state_machine.request_new_position(atoi(parsed.c_str()));
 }},
{MQTT_TOPIC + "/calibrate", [](byte * string, unsigned int length) { state_machine.request_new_state(StateMachine::State::CALIBRATE); }}};
char numberArray[20];

void setup()
{
    ota::initialize(wifi::hostname);
    WiFi.hostname(wifi::hostname);
    WiFi.begin(wifi::ssid, wifi::password);

    wait_for_initialization([]() { return not(WiFi.status() == WL_CONNECTED); });
    wait_for_initialization([]() { return not(mqtt_.connect(mqtt::client_id)); });

    mqtt_.setCallback([](char * topic, byte * payload, unsigned int length) { handle_topic[topic](payload, length); });
    mqtt_.setKeepAlive(2);
    mqtt_.setSocketTimeout(2);

    mqtt::subscribe(mqtt_, handle_topic);

    drv8220dsg::init();
    encoder::init(encoder_);

    encoder_.loop();

    state_machine_loop.attach_ms(100, []() { state_machine.loop(); });
    publisher_loop.attach_ms(1000, []() {
        if (state_machine.safe_to_publish() and mqtt_.connected())
        {
            itoa(state_machine.get_current_postion(), numberArray, 10);
            mqtt_.publish((MQTT_TOPIC + "/current_position").c_str(), numberArray);
            itoa(state_machine.get_top_position_steps_value(), numberArray, 10);
            mqtt_.publish((MQTT_TOPIC + "/calibration/top_steps_value").c_str(), numberArray);
            itoa(state_machine.get_bottom_position_steps_value(), numberArray, 10);
            mqtt_.publish((MQTT_TOPIC + "/calibration/bottom_steps_value").c_str(), numberArray);
        }
    });
    reconnect_loop.attach_ms(5000, check_connection);
}

void loop()
{

    encoder_.loop();
    state_machine.update_steps_value(encoder_.getPosition());
    ota::handle();
    mqtt_.loop();
    reconnect_if_needed();
}

constexpr auto MAX_RECONNECT_ATTEMPTS = 36;
std::atomic<bool> reconnect_needed = false;
uint8_t reconnect_attempts = 0;

void check_connection()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        reconnect_needed = true;
        return;
    }

    if (not mqtt_.connected())
    {
        reconnect_needed = true;
        return;
    }
}

void reconnect_if_needed()
{
    if (not reconnect_needed)
    {
        return;
    }

    reconnect_needed = false;
    reconnect_attempts++;

    if (reconnect_attempts > MAX_RECONNECT_ATTEMPTS)
    {
        ESP.restart();
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.reconnect();
        return;
    }

    if (not mqtt_.connected())
    {
        if (not mqtt_.connect(mqtt::client_id))
        {
            return;
        }
        mqtt::subscribe(mqtt_, handle_topic);
    }
    reconnect_attempts = 0;
}