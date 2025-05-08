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

// update_internal_state_from_response_() để xử lý đầy đủ các gói:

// 'G1': trạng thái cơ bản (mode, nhiệt độ, quạt)
// 'G6': các chế độ đặc biệt (powerful, comfort, quiet, streamer, sensor, led)
// 'Sx': cảm biến nhiệt độ (room, outdoor, coil)



void DaikinS21::update_internal_state_from_response_(const uint8_t *data, size_t length) {
  if (length < 6 || data[0] != 0x02 || data[length - 1] != 0x03) {
    ESP_LOGW(TAG, "Invalid frame or length");
    return;
  }

  char cmd0 = data[1];
  char cmd1 = data[2];
  const uint8_t *payload = &data[3];
  size_t payload_len = length - 5;

  if (cmd0 == 'G' && cmd1 == '1' && payload_len >= 4) {
    this->mode = [payload]() {
      switch (payload[1] & 0x7) {
        case 0: return climate::CLIMATE_MODE_AUTO;
        case 2: return climate::CLIMATE_MODE_DRY;
        case 3: return climate::CLIMATE_MODE_COOL;
        case 4: return climate::CLIMATE_MODE_HEAT;
        case 6: return climate::CLIMATE_MODE_FAN_ONLY;
        default: return climate::CLIMATE_MODE_OFF;
      }
    }();

    this->target_temperature = 18.0f + 0.5f * (static_cast<int>(payload[2]) - '@');

    this->fan_mode = [payload]() {
      switch (payload[3]) {
        case 'A': return climate::CLIMATE_FAN_AUTO;
        case '3': return climate::CLIMATE_FAN_LOW;
        case '4': return climate::CLIMATE_FAN_MEDIUM;
        case '5': return climate::CLIMATE_FAN_HIGH;
        default:  return climate::CLIMATE_FAN_AUTO;
      }
    }();

    publish_state();
  } else if (cmd0 == 'G' && cmd1 == '6' && payload_len >= 4) {
    bool powerful = payload[0] & 0x02;
    bool comfort  = payload[0] & 0x40;
    bool quiet    = payload[0] & 0x80;
    bool streamer = payload[1] & 0x80;
    bool sensor   = payload[3] & 0x08;
    bool led      = (payload[3] & 0x0C) != 0x0C;

    if (powerful_sensor_ != nullptr) powerful_sensor_->publish_state(powerful);
    if (comfort_sensor_ != nullptr)  comfort_sensor_->publish_state(comfort);
    if (quiet_sensor_ != nullptr)    quiet_sensor_->publish_state(quiet);
    if (streamer_sensor_ != nullptr) streamer_sensor_->publish_state(streamer);
    if (body_sensor_ != nullptr)     body_sensor_->publish_state(sensor);
    if (led_sensor_ != nullptr)      led_sensor_->publish_state(led);
  } else if (cmd0 == 'S' && payload_len >= 4) {
    float t = (payload[0] - '0') + (payload[1] - '0') * 10 + (payload[2] - '0') * 100;
    if (payload[3] == '-') t = -t;
    t *= 0.1f;
    if (t > -40.0f && t < 100.0f) {
      switch (cmd1) {
        case 'H': if (room_sensor_) room_sensor_->publish_state(t); break;
        case 'a': if (outdoor_sensor_) outdoor_sensor_->publish_state(t); break;
        case 'I': if (coil_sensor_) coil_sensor_->publish_state(t); break;
      }
    }
  }
}


}  // namespace daikin_s21
}  // namespace esphome
