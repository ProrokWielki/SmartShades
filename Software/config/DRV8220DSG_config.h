#pragma once

#include "config.h"

namespace drv8220dsg
{

constexpr auto DRV_SLEEP_PIN = 2;
constexpr auto DRV_MODE_PIN = 14;
constexpr auto DRV_IN1_PIN = 12;
constexpr auto DRV_IN2_PIN = 13;

void down()
{
    digitalWrite(DRV_IN1_PIN, IN1_DOWN_VALUE);
    digitalWrite(DRV_IN2_PIN, !IN1_DOWN_VALUE);
}
void up()
{
    digitalWrite(DRV_IN1_PIN, !IN1_DOWN_VALUE);
    digitalWrite(DRV_IN2_PIN, IN1_DOWN_VALUE);
}
void stop()
{
    digitalWrite(DRV_IN1_PIN, 1);
    digitalWrite(DRV_IN2_PIN, 1);
}

void idle()
{
    digitalWrite(DRV_IN1_PIN, 0);
    digitalWrite(DRV_IN2_PIN, 0);
}

void init()
{
    pinMode(DRV_SLEEP_PIN, OUTPUT);
    pinMode(DRV_MODE_PIN, OUTPUT);
    pinMode(DRV_IN1_PIN, OUTPUT);
    pinMode(DRV_IN2_PIN, OUTPUT);

    digitalWrite(DRV_SLEEP_PIN, HIGH);
    digitalWrite(DRV_MODE_PIN, LOW);

    idle();
}

}  // namespace drv8220dsg
