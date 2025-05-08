#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/daikin_s21/daikin_s21.h"

namespace esphome {
namespace daikin_s21 {

static DaikinS21 *make_daikin_s21(UARTComponent *uart_parent) {
  auto *ac = new DaikinS21(uart_parent);
  return ac;
}

}  // namespace daikin_s21
}  // namespace esphome

// Đăng ký platform climate tên là "daikin_s21"
ESPHOME_REGISTER_COMPONENT(daikin_s21::DaikinS21, daikin_s21::make_daikin_s21)
ESPHOME_REGISTER_CLIMATE(daikin_s21::DaikinS21, daikin_s21::make_daikin_s21)
