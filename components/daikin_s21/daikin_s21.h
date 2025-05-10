#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace daikin_s21 {

class DaikinS21 : public climate::Climate, public Component, public uart::UARTDevice {
 public:
  DaikinS21(uart::UARTComponent *parent);

  void setup() override;
  void loop() override;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  // Sensor setters
  void set_room_sensor(sensor::Sensor *s) { room_sensor_ = s; }
  void set_outdoor_sensor(sensor::Sensor *s) { outdoor_sensor_ = s; }
  void set_coil_sensor(sensor::Sensor *s) { coil_sensor_ = s; }

  void set_powerful_sensor(binary_sensor::BinarySensor *s) { powerful_sensor_ = s; }
  void set_comfort_sensor(binary_sensor::BinarySensor *s) { comfort_sensor_ = s; }
  void set_quiet_sensor(binary_sensor::BinarySensor *s) { quiet_sensor_ = s; }
  void set_streamer_sensor(binary_sensor::BinarySensor *s) { streamer_sensor_ = s; }
  void set_body_sensor(binary_sensor::BinarySensor *s) { body_sensor_ = s; }
  void set_led_sensor(binary_sensor::BinarySensor *s) { led_sensor_ = s; }

 protected:
  void send_command_();
  void send_swing_command_();
  void update_internal_state_from_response_(const uint8_t *data, size_t len);

  sensor::Sensor *room_sensor_{nullptr};
  sensor::Sensor *outdoor_sensor_{nullptr};
  sensor::Sensor *coil_sensor_{nullptr};

  binary_sensor::BinarySensor *powerful_sensor_{nullptr};
  binary_sensor::BinarySensor *comfort_sensor_{nullptr};
  binary_sensor::BinarySensor *quiet_sensor_{nullptr};
  binary_sensor::BinarySensor *streamer_sensor_{nullptr};
  binary_sensor::BinarySensor *body_sensor_{nullptr};
  binary_sensor::BinarySensor *led_sensor_{nullptr};

  bool swing_vertical_{false};
  bool swing_horizontal_{false};
};

}  // namespace daikin_s21
}  // namespace esphome
