import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']

daikin_s21_ns = cg.esphome_ns.namespace("daikin_s21")
DaikinS21 = daikin_s21_ns.class_("DaikinS21", cg.Component)

# Sensor schema
SENSOR_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(sensor.Sensor),
}).extend(sensor.sensor_schema(
    accuracy_decimals=1,
    device_class="temperature",
    state_class="measurement",
    unit_of_measurement="°C",
))

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DaikinS21),
    cv.Required("tx_uart"): cv.use_id(uart.UARTComponent),
    cv.Required("rx_uart"): cv.use_id(uart.UARTComponent),
    cv.Optional("sensor"): cv.Schema({
        cv.Required("s21_id"): cv.use_id(DaikinS21),
        cv.Optional("inside_temperature"): SENSOR_SCHEMA,
        cv.Optional("outside_temperature"): SENSOR_SCHEMA,
        cv.Optional("fan_speed"): cv.Schema({
            cv.GenerateID(): cv.declare_id(sensor.Sensor),
        }).extend(sensor.sensor_schema(
            accuracy_decimals=0,
            device_class="speed",
            state_class="measurement",
            unit_of_measurement="rpm",
        )),
    }),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    tx_uart_ = await cg.get_variable(config["tx_uart"])
    rx_uart_ = await cg.get_variable(config["rx_uart"])
    cg.add(var.set_tx_uart(tx_uart_))
    cg.add(var.set_rx_uart(rx_uart_))

    if "sensor" in config:
        s21_ = await cg.get_variable(config["sensor"]["s21_id"])
        if "inside_temperature" in config["sensor"]:
            temp = cg.new_Pvariable(config["sensor"]["inside_temperature"][CONF_ID])
            await sensor.register_sensor(temp, config["sensor"]["inside_temperature"])
            cg.add(s21_.set_inside_temperature_sensor(temp))
        if "outside_temperature" in config["sensor"]:
            outside = cg.new_Pvariable(config["sensor"]["outside_temperature"][CONF_ID])
            await sensor.register_sensor(outside, config["sensor"]["outside_temperature"])
            cg.add(s21_.set_outside_temperature_sensor(outside))
        if "fan_speed" in config["sensor"]:
            fan = cg.new_Pvariable(config["sensor"]["fan_speed"][CONF_ID])
            await sensor.register_sensor(fan, config["sensor"]["fan_speed"])
            cg.add(s21_.set_fan_speed_sensor(fan))