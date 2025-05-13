#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace daikin_s21 {

class DaikinS21 : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_tx_uart(uart::UARTComponent *tx_uart) { tx_uart_ = tx_uart; }
  void set_rx_uart(uart::UARTComponent *rx_uart) { rx_uart_ = rx_uart; }
  void set_inside_temperature_sensor(sensor::Sensor *sensor) { inside_temperature_sensor_ = sensor; }
  void set_outside_temperature_sensor(sensor::Sensor *sensor) { outside_temperature_sensor_ = sensor; }
  void set_fan_speed_sensor(sensor::Sensor *sensor) { fan_speed_sensor_ = sensor; }

 protected:
  uart::UARTComponent *tx_uart_{nullptr};
  uart::UARTComponent *rx_uart_{nullptr};
  sensor::Sensor *inside_temperature_sensor_{nullptr};
  sensor::Sensor *outside_temperature_sensor_{nullptr};
  sensor::Sensor *fan_speed_sensor_{nullptr};
};

}  // namespace daikin_s21
}  // namespace esphome