#include "s21.h"
#include "esphome/core/log.h"

namespace esphome {
namespace daikin_s21 {

static const char *const TAG = "daikin_s21";

void DaikinS21::setup() {
  if (this->rx_uart_ == nullptr || this->tx_uart_ == nullptr) {
    ESP_LOGW(TAG, "No UART component set");
    this->mark_failed();
    return;
  }
}

void DaikinS21::loop() {
  std::vector<uint8_t> buffer;
  while (this->rx_uart_->available()) {
    buffer.push_back(this->rx_uart_->read());
  }

  if (buffer.size() >= 8) {
    // Basic frame validation (simplified, no checksum for brevity)
    std::string data(buffer.begin(), buffer.end());
    // Parse G1 frame (inside temperature)
    if (data[2] == '1' && this->inside_temperature_sensor_) {
      float room_temp = data[6] / 2.0f; // Example byte
      this->inside_temperature_sensor_->publish_state(room_temp);
    }
    // Parse G5 frame (outside temperature, fan speed)
    if (data[2] == '5' && data.size() >= 10) {
      if (this->outside_temperature_sensor_) {
        float outside_temp = data[8] / 2.0f; // Example byte
        this->outside_temperature_sensor_->publish_state(outside_temp);
      }
      if (this->fan_speed_sensor_) {
        uint16_t fan_speed = data[9] * 10; // Example byte
        this->fan_speed_sensor_->publish_state(fan_speed);
      }
    }
  }
}

void DaikinS21::dump_config() {
  ESP_LOGCONFIG(TAG, "Daikin S21 Component");
  if (this->tx_uart_ && this->rx_uart_) {
    ESP_LOGCONFIG(TAG, "  UART: Configured (TX and RX)");
  }
  if (this->inside_temperature_sensor_) {
    ESP_LOGCONFIG(TAG, "  Inside Temperature Sensor: Configured");
  }
  if (this->outside_temperature_sensor_) {
    ESP_LOGCONFIG(TAG, "  Outside Temperature Sensor: Configured");
  }
  if (this->fan_speed_sensor_) {
    ESP_LOGCONFIG(TAG, "  Fan Speed Sensor: Configured");
  }
}

}  // namespace daikin_s21
}  // namespace esphome