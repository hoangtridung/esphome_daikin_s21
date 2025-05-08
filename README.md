Daikin S21 Component for ESPHome

Custom ESPHome climate component to control Daikin air conditioners using the S21 protocol via UART.

✅ Features

Full control of Daikin S21-compatible ACs:

On/off, temperature, mode, fan speed

Swing vertical & horizontal (F5 command)

Powerful, Quiet, Comfort mode detection (G6)

Streamer, Sensor, LED status (G6)

Internal & external temperature sensors (Sx)

Works with Home Assistant

🛠 Hardware Requirements

ESP32/ESP8266 with UART pins connected to Daikin indoor unit S21 port

Voltage level shifter if required (Daikin uses 5V logic)

🧱 Installation

1. Clone or copy into your ESPHome project

config/
├── my_ac.yaml
└── custom_components/
    └── daikin_s21/
        ├── __init__.py
        ├── daikin_s21.h
        ├── daikin_s21.cpp
        ├── register_platform.cpp
        └── README.md

2. Configure YAML

external_components:
  - source:
      type: local
      path: ./custom_components
    components: [daikin_s21]

uart:
  id: uart1
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

sensor:
  - platform: template
    name: "Room Temperature"
    id: room_temp
  - platform: template
    name: "Outdoor Temperature"
    id: outdoor_temp
  - platform: template
    name: "Coil Temperature"
    id: coil_temp

binary_sensor:
  - platform: template
    name: "Powerful Mode"
    id: powerful_mode
  - platform: template
    name: "Comfort Mode"
    id: comfort_mode
  - platform: template
    name: "Quiet Mode"
    id: quiet_mode
  - platform: template
    name: "Streamer"
    id: streamer_status
  - platform: template
    name: "Body Sensor"
    id: body_sensor_status
  - platform: template
    name: "LED Status"
    id: led_status

climate:
  - platform: daikin_s21
    uart_id: uart1
    room_sensor: room_temp
    outdoor_sensor: outdoor_temp
    coil_sensor: coil_temp
    powerful_sensor: powerful_mode
    comfort_sensor: comfort_mode
    quiet_sensor: quiet_mode
    streamer_sensor: streamer_status
    body_sensor: body_sensor_status
    led_sensor: led_status

📡 Notes

You must match the Daikin indoor unit S21 protocol format (3-wire UART serial)

The component automatically parses responses and updates states

Manual swing and advanced mode sending is supported (F1, F5, G6)

💬 Credit

Developed based on Daikin S21 protocol analysis and custom ESPHome extension principles.

