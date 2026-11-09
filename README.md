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
- 12 V source + and - also go to the converter input (IN+ and IN-)

The buck converter does not isolate input from output, so the 12 V and 5 V grounds are already shared.

### 12 V power side (actuators)

The actuators get 12 V straight from the source through the relay contacts, never through the buck converter. Each relay module has three contact terminals: **COM**, **NO** (normally open) and **NC** (normally closed). Two relays reverse the polarity:

| Relay | COM | NO | NC |
|-------|-----|----|----|
| Relay 1 (UP, GPIO 32) | both actuators' **red** wires | 12 V + | 12 V - |
| Relay 2 (DOWN, GPIO 33) | both actuators' **black** wires | 12 V + | 12 V - |

How it works:

- Both relays off: both actuator wires sit on 12 V -, so the actuators are stopped.
- Relay 1 on: red = 12 V +, black = 12 V -, so the actuators extend.
- Relay 2 on: black = 12 V +, red = 12 V -, so the actuators retract.
- The sketch never switches both relays on at once. With this wiring both on would put 12 V + on both wires (no voltage across the actuators, so they would just stop), and the sketch waits 200 ms with both relays off between direction changes anyway.

Steps:

1. Power everything off before wiring.
2. Join the two red wires into one connection and the two black wires into another (the actuators run in parallel). Use a terminal block or a good crimp or solder joint.
3. Red (both) to relay 1 COM, black (both) to relay 2 COM.
4. Make a 12 V + wire with a fuse and connect it to NO on both relays.
5. Make a 12 V - wire and connect it to NC on both relays.
6. Use thick wire (at least 1.5 mm2, solid core is fine) for everything on the 12 V side. The converter's small terminals only carry the thin input wires.
7. Check the wiring against the table before switching on the 12 V. First test with the ESP32 relay signal running and the actuators disconnected, then connect the actuators.

If the actuators move the wrong way, swap the red and black wires on the two COM terminals.

### Relay trigger jumper

The relay modules have an H/L trigger jumper. With the jumper on **H**, the 3.3 V ESP32 signal did not switch the relay reliably. Set it to **L**. The sketch pulls the pin LOW to turn a relay on and releases it (input) to turn it off, so 3.3 V never has to hold a 5 V module off. Test with the actuators disconnected first.

## Software

1. Install the ESP32 board package (Espressif) in the Arduino IDE.
2. Install the libraries **Adafruit SSD1306** and **Adafruit GFX Library**.
3. Open `actuator_cycle/actuator_cycle.ino`, select your ESP32 board and port, and upload.

### Settings

| Constant | Meaning |
|----------|---------|
| `PIN_RELAY_UP`, `PIN_RELAY_DOWN` | relay signal pins |
| `MOVE_MS` | time per move, full stroke plus a margin so both actuators reach their end stop |
| `REST_MS` | pause after each move |

The OK648 is rated for a 25 % duty cycle. If you run it for long periods, set `REST_MS` to about 3x `MOVE_MS`.

## Safety

- Fuse the 12 V line
- Check the current draw of both actuators together against your supply's rating
- Test with the actuators disconnected before the first real run
