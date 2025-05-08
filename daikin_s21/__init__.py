import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate, uart, sensor, binary_sensor
from esphome.const import (
    CONF_ID, CONF_UART_ID,
    CONF_ROOM_TEMPERATURE, CONF_OUTDOOR_TEMPERATURE, CONF_COIL_TEMPERATURE,
)

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
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_UART_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    await uart.register_uart_device(var, config[CONF_UART_ID])

    if "room_sensor" in config:
        sens = await cg.get_variable(config["room_sensor"])
        cg.add(var.set_room_sensor(sens))
    if "outdoor_sensor" in config:
        sens = await cg.get_variable(config["outdoor_sensor"])
        cg.add(var.set_outdoor_sensor(sens))
    if "coil_sensor" in config:
        sens = await cg.get_variable(config["coil_sensor"])
        cg.add(var.set_coil_sensor(sens))

    for name in ["powerful_sensor", "comfort_sensor", "quiet_sensor", "streamer_sensor", "body_sensor", "led_sensor"]:
        if name in config:
            bsen = await cg.get_variable(config[name])
            cg.add(getattr(var, f"set_{name}")(bsen))
