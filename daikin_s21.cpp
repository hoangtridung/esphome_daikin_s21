// daikin_s21.cpp
#include "daikin_s21.h"
#include "esphome/core/log.h"

namespace esphome {
namespace daikin_s21 {

static const char *const TAG = "daikin_s21";

DaikinS21::DaikinS21(uart::UARTComponent *parent) : UARTDevice(parent) {}

void DaikinS21::setup() {
  ESP_LOGI(TAG, "Daikin S21 climate component initialized");
}

void DaikinS21::loop() {
  static std::vector<uint8_t> buffer;
  while (available()) {
    uint8_t c = read();
    buffer.push_back(c);
    if (c == 0x03 && buffer.size() >= 6) {
      update_internal_state_from_response_(buffer.data(), buffer.size());
      buffer.clear();
    } else if (buffer.size() > 64) {
      buffer.clear();
    }
  }
}

void DaikinS21::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();
  if (call.get_fan_mode().has_value())
    this->fan_mode = *call.get_fan_mode();

  send_command_();
  publish_state();
}

climate::ClimateTraits DaikinS21::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_auto_mode(true);
  traits.set_supports_cool_mode(true);
  traits.set_supports_heat_mode(true);
  traits.set_supports_dry_mode(true);
  traits.set_supports_fan_mode_auto(true);
  traits.set_supports_fan_mode_on(true);
  traits.set_visual_min_temperature(18);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(0.5);
  return traits;
}

void DaikinS21::send_command_() {
  uint8_t buf[8];
  buf[0] = 0x02;  // STX
  buf[1] = 'F';
  buf[2] = '1';
  buf[3] = this->mode == climate::CLIMATE_MODE_OFF ? '0' : '1';

  buf[4] = [this]() -> uint8_t {
    switch (this->mode) {
      case climate::CLIMATE_MODE_AUTO: return 0;
      case climate::CLIMATE_MODE_DRY: return 2;
      case climate::CLIMATE_MODE_COOL: return 3;
      case climate::CLIMATE_MODE_HEAT: return 4;
      case climate::CLIMATE_MODE_FAN_ONLY: return 6;
      default: return 0;
    }
  }();

  float temp = this->target_temperature;
  if (temp < 18.0f) temp = 18.0f;
  if (temp > 30.0f) temp = 30.0f;
  buf[5] = static_cast<uint8_t>(roundf((temp - 18.0f) * 2.0f)) + '@';

  buf[6] = [this]() -> uint8_t {
    switch (this->fan_mode.value()) {
      case climate::CLIMATE_FAN_AUTO: return 'A';
      case climate::CLIMATE_FAN_LOW: return '3';
      case climate::CLIMATE_FAN_MEDIUM: return '4';
      case climate::CLIMATE_FAN_HIGH: return '5';
      default: return 'A';
    }
  }();

  uint8_t checksum = 0;
  for (int i = 1; i <= 6; ++i)
    checksum += buf[i];
  if (checksum == 0x02 || checksum == 0x03 || checksum == 0x06)
    checksum += 2;
  buf[7] = checksum;

  for (int i = 0; i < 8; ++i)
    this->write_byte(buf[i]);
  this->write_byte(0x03);  // ETX

  ESP_LOGD(TAG, "Sent command: Power=%c Mode=%d Temp=%.1f Fan=%c Checksum=0x%02X", buf[3], buf[4], temp, buf[6], buf[7]);
}

void DaikinS21::send_swing_command_() {
  uint8_t buf[5];
  buf[0] = 0x02;
  buf[1] = 'F';
  buf[2] = '5';
  buf[3] = (swing_vertical_ ? 0x01 : 0x00) | (swing_horizontal_ ? 0x02 : 0x00);
  uint8_t checksum = buf[1] + buf[2] + buf[3];
  if (checksum == 0x02 || checksum == 0x03 || checksum == 0x06)
    checksum += 2;
  buf[4] = checksum;
  for (int i = 0; i < 5; ++i)
    this->write_byte(buf[i]);
  this->write_byte(0x03);

  ESP_LOGD(TAG, "Sent swing command: vert=%d, horiz=%d, payload=0x%02X", swing_vertical_, swing_horizontal_, buf[3]);
}

void DaikinS21::update_internal_state_from_response_(const uint8_t *data, size_t length) {
  // TODO: thêm xử lý các gói G1, G6, Sx tại đây như đã trình bày ở các bước trước
}

}  // namespace daikin_s21
}  // namespace esphome
