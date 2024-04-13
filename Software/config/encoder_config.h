#pragma once

#include <ESPRotary.h>

#include "config.h"

namespace encoder
{
constexpr auto ENCODER_PHASE_A_PIN = 5;
constexpr auto ENCODER_PHASE_B_PIN = 4;

void init(ESPRotary & encoder_)
{
    encoder_.begin(ENCODER_PHASE_A_PIN, ENCODER_PHASE_B_PIN, 20);
    encoder_.setIncrement(GOING_DOWN_MAKES_COUNTER_INCREASE ? 1 : -1);
}

}  // namespace encoder
