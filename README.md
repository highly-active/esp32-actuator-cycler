# esp32-actuator-cycler

ESP32 sketch that drives two 12 V linear actuators (VEVOR OK648) in parallel through relays, cycling them up and down endlessly. The built-in OLED shows the current state.

## Features

- Both actuators move together: UP, rest, DOWN, rest, repeat
- OLED status (SSD1306, 128x64):
  - top band: cycle count and seconds left in the current state
  - main area: UP / DOWN / REST with an arrow
  - bottom: progress bar for the current state
- Relays are switched off at boot, and one relay opens before the other closes
- Move time and rest time are set with two constants at the top of the sketch

## Hardware

| Part | Notes |
|------|-------|
| ESP32 with built-in 0.96" OLED (Ideaspark) | SDA = GPIO 21, SCL = GPIO 22, address 0x3C |
| 2x VEVOR OK648 linear actuator | 12 V DC, wired in parallel |
| 2x 5 V single-channel relay module (30 A) | one for UP, one for DOWN |
| LM2596 or MP1584 buck converter | 12 V to 5 V for the ESP32 and relay coils |
| 12 V supply | must handle both actuators (about 10 A peak with margin) and have a fuse or breaker |

## Wiring

**Control side (5 V from the buck converter)**

- Converter OUT+ to ESP32 `VIN` and both relays' `DC+`
- Converter OUT- to ESP32 `GND` and both relays' `DC-`
- ESP32 GPIO 32 to relay 1 `IN` (UP)
- ESP32 GPIO 33 to relay 2 `IN` (DOWN)

**Power side (12 V, not through the converter)**

- 12 V source + and - go to the converter input
- Relays switch the actuator polarity: relay 1 feeds the red wires, relay 2 feeds the black wires
- Use thick wire for the actuator connections

The buck converter does not isolate input from output, so the 12 V and 5 V grounds are already shared.

### Relay trigger jumper

The relay modules have an H/L trigger jumper. With the jumper on **H**, the 3.3 V ESP32 signal did not switch the relay reliably. Setting it to **L** fixed it. Check that `RELAY_ON` in the sketch matches how your module behaves, and test with the actuators disconnected first.

## Software

1. Install the ESP32 board package (Espressif) in the Arduino IDE.
2. Install the libraries **Adafruit SSD1306** and **Adafruit GFX Library**.
3. Open `actuator_cycle/actuator_cycle.ino`, select your ESP32 board and port, and upload.

### Settings

| Constant | Meaning |
|----------|---------|
| `PIN_RELAY_UP`, `PIN_RELAY_DOWN` | relay signal pins |
| `RELAY_ON` | signal level that turns a relay on |
| `MOVE_MS` | time per move, full stroke plus a margin so both actuators reach their end stop |
| `REST_MS` | pause after each move |

The OK648 is rated for a 25 % duty cycle. If you run it for long periods, set `REST_MS` to about 3x `MOVE_MS`.

## Safety

- Fuse the 12 V line
- Check the current draw of both actuators together against your supply's rating
- Test with the actuators disconnected before the first real run
