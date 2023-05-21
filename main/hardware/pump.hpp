#pragma once

#include <data/watering_schedule.hpp>

#include <driver/gpio.h>
#include <driver/ledc.h>

namespace plant {

class WaterPump {
public:
    explicit WaterPump(gpio_num_t output) {
        // gpio_reset_pin(ouput_);

        // zero-initialize the config structure.
        gpio_config_t io_conf = {};
        // disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        // set as output mode
        io_conf.mode = GPIO_MODE_OUTPUT;
        // bit mask of the pins that you want to set,e.g.GPIO18/19
        io_conf.pin_bit_mask = 1 << output;
        // disable pull-down mode
        io_conf.pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE;
        // disable pull-up mode
        io_conf.pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE;
        // configure GPIO with the given settings
        gpio_config(&io_conf);

        // Prepare and then apply the LEDC PWM timer configuration
        ledc_timer_config_t ledc_timer_conf = {
            .speed_mode = ledc_mode,
            .duty_resolution = LEDC_TIMER_13_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 1000, // Set output frequency at 1 kHz
            .clk_cfg = LEDC_AUTO_CLK};
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_conf));

        // Prepare and then apply the LEDC PWM channel configuration
        ledc_channel_config_t ledc_channel_conf = {
            .gpio_num = output,
            .speed_mode = ledc_timer_conf.speed_mode,
            .channel = ledc_channel,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0, // Set duty to 0% (pump off)
            .hpoint = 0,
            .flags = {.output_invert = 1}};
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_conf));
    }

    void set_flow_speed(FlowSpeed speed) {
        ESP_ERROR_CHECK(
            ledc_set_duty(ledc_mode, ledc_channel, duty_cycle_for(speed)));
        ESP_ERROR_CHECK(ledc_update_duty(ledc_mode, ledc_channel));
    }

    void turn_off() {
        set_flow_speed(FlowSpeed{0.f});
    }

    [[nodiscard]] bool is_enabled() const {
        const auto duty_cycle = ledc_get_duty(ledc_mode, ledc_channel);
        return duty_cycle > 0;
    }

private:
    std::uint32_t duty_cycle_for(FlowSpeed speed) {
        if (speed.value() == 0) {
            return 0;
        } else {
            return static_cast<uint32_t>(min_duty_cycle +
                                         duty_cycle_range * speed.value());
        }
    }

    static constexpr ledc_mode_t ledc_mode = ledc_mode_t::LEDC_LOW_SPEED_MODE;
    static constexpr ledc_channel_t ledc_channel =
        ledc_channel_t::LEDC_CHANNEL_0;
    static constexpr std::uint32_t max_duty_cycle = (1 << 13) - 1;
    static constexpr std::uint32_t min_duty_cycle = (15 * max_duty_cycle) / 100;
    static constexpr std::uint32_t duty_cycle_range =
        max_duty_cycle - min_duty_cycle;
};

} // namespace plant