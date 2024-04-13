#pragma once

#include <string>

const std::string MQTT_TOPIC_ROOT = "shades";
const std::string MQTT_SHADE_TOPIC = "bedroom/right";

constexpr uint8_t IN1_DOWN_VALUE = 1;
constexpr bool GOING_DOWN_MAKES_COUNTER_INCREASE = true;

const auto MQTT_TOPIC = MQTT_TOPIC_ROOT + "/" + MQTT_SHADE_TOPIC;