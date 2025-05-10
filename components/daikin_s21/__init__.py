import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, uart, sensor, binary_sensor
# from esphome.const import (
    # CONF_ID, CONF_UART_ID,
    # CONF_ROOM_TEMPERATURE, CONF_OUTDOOR_TEMPERATURE, CONF_COIL_TEMPERATURE,
# )
from esphome.const import CONF_ID, CONF_UART_ID
DEPENDENCIES = ["uart"]

daikin_ns = cg.esphome_ns.namespace("daikin_s21")
DaikinS21 = daikin_ns.class_("DaikinS21", climate.Climate, cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(DaikinS21),
    cv.GenerateID(CONF_UART_ID): cv.use_id(uart.UARTComponent),

    cv.Optional("room_sensor"): cv.use_id(sensor.Sensor),
    cv.Optional("outdoor_sensor"): cv.use_id(sensor.Sensor),
    cv.Optional("coil_sensor"): cv.use_id(sensor.Sensor),

    cv.Optional("powerful_sensor"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("comfort_sensor"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("quiet_sensor"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("streamer_sensor"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("body_sensor"): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional("led_sensor"): cv.use_id(binary_sensor.BinarySensor),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # Lấy biến UARTComponent từ ID trong YAML
    uart_var = await cg.get_variable(config[CONF_UART_ID])
    # Tạo instance của DaikinS21
    var = cg.new_Pvariable(config[CONF_ID], uart_var)

    # Đăng ký component
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    # Ánh xạ các sensor
    if "room_sensor" in config:
        room_sensor = await cg.get_variable(config["room_sensor"])
        cg.add(var.set_room_sensor(room_sensor))
    if "outdoor_sensor" in config:
        outdoor_sensor = await cg.get_variable(config["outdoor_sensor"])
        cg.add(var.set_outdoor_sensor(outdoor_sensor))
    if "coil_sensor" in config:
        coil_sensor = await cg.get_variable(config["coil_sensor"])
        cg.add(var.set_coil_sensor(coil_sensor))

    # Ánh xạ binary_sensor
    for name in ["powerful_sensor", "comfort_sensor", "quiet_sensor", "streamer_sensor", "body_sensor", "led_sensor"]:
        if name in config:
            binary_sensor_var = await cg.get_variable(config[name])
            cg.add(getattr(var, f"set_{name}")(binary_sensor_var))
