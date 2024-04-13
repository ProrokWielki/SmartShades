#pragma once

#include <atomic>

class StateMachine
{
public:
    enum class State : uint8_t
    {
        WAITING,
        DOWN,
        UP,
        CALIBRATE
    };

    StateMachine(const std::function<void()> & idle, const std::function<void()> & stop, const std::function<void()> & down, const std::function<void()> & up)
    : idle_{idle}, stop_{stop}, down_{down}, up_{up}
    {
    }

    void request_new_state(State new_state)
    {
        current_state = new_state;
    }

    void request_new_position(int position)
    {
        new_position_requested = true;
        requested_steps_value = top_steps_value + (top_to_bottom_steps_value * (position) / 100);
    }

    void update_steps_value(int new_steps_value)
    {
        current_steps_value = new_steps_value;
    }

    int get_current_postion() const
    {
        return (std::abs(top_steps_value - current_steps_value) * 100) / top_to_bottom_steps_value;
    }

    int get_top_position_steps_value() const
    {
        return top_steps_value;
    }

    int get_bottom_position_steps_value() const
    {
        return bottom_steps_value;
    }

    bool safe_to_publish()
    {
        return !new_position_requested and top_to_bottom_steps_value != 0;
    }

    void loop()
    {
        switch (current_state)
        {
            case State::WAITING:
                idle_();
                if (new_position_requested)
                {
                    if (requested_steps_value < current_steps_value)
                    {
                        current_state = State::UP;
                    }
                    else if (requested_steps_value > current_steps_value)
                    {
                        current_state = State::DOWN;
                    }
                }
                break;
            case State::DOWN:
                if (going == false)
                {
                    down_();
                    going = true;
                }
                else if (current_steps_value >= bottom_steps_value or current_steps_value >= requested_steps_value or current_steps_value == old_steps_value)
                {
                    current_state = State::WAITING;
                    stop_();

                    new_position_requested = false;
                    going = false;
                }
                break;
            case State::UP:
                if (going == false)
                {
                    up_();
                    going = true;
                }
                else if (current_steps_value <= top_steps_value or current_steps_value <= requested_steps_value or current_steps_value == old_steps_value)
                {
                    current_state = State::WAITING;
                    stop_();

                    new_position_requested = false;
                    going = false;
                }
                break;
            case State::CALIBRATE:
                if (calibration_state == State::UP)
                {
                    if (going == false)
                    {
                        up_();
                        going = true;
                    }
                    else if (current_steps_value == old_steps_value)
                    {
                        top_steps_value = current_steps_value;
                        calibration_state = State::DOWN;
                        going = false;

                        current_state = State::WAITING;
                        stop_();

                        top_to_bottom_steps_value = std::abs(top_steps_value - bottom_steps_value);
                    }
                }
                else if (calibration_state == State::DOWN)
                {
                    if (going == false)
                    {
                        down_();
                        going = true;
                    }
                    else if (current_steps_value == old_steps_value)
                    {
                        bottom_steps_value = current_steps_value;
                        calibration_state = State::UP;
                        going = false;
                    }
                }
                break;
        }
        old_steps_value = current_steps_value;
    }

private:
    State current_state{State::CALIBRATE};
    State calibration_state{State::DOWN};

    std::atomic<bool> new_position_requested{false};

    std::atomic<int> requested_steps_value{};
    std::atomic<int> current_steps_value{};

    int old_steps_value{};
    int top_steps_value{};
    int bottom_steps_value{};
    int top_to_bottom_steps_value{};

    bool going{false};

    std::function<void()> idle_;
    std::function<void()> stop_;
    std::function<void()> down_;
    std::function<void()> up_;
};
