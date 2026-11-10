# Handoff: SmartKnob Matter / HomeKit

Paste this into a new conversation to continue exactly where we stopped.

## Goal
Add Matter over WiFi to the SmartKnob firmware (repo: `/Users/marcel/Desktop/SmartKnob/smartknob`) so it works in Apple Home. The user uses `/plan` mode: no code changes outside the approved plan.

## Hardware and repo facts
- Board: original ESP32 (ESP32-PICO-V3-02), **8 MB flash, 2 MB PSRAM** (unknown whether PSRAM works). PlatformIO env `view` in `platformio.ini`, on `espressif32@3.4` (Arduino core 1.x, **no Matter support**).
- Firmware: `firmware/src/`. Tasks: `MotorTask` (core 1), `InterfaceTask` and `DisplayTask` (core 0). `main.cpp` creates them.
- `interface_task.cpp`: press detection is **strain-gauge based** (`SK_STRAIN`, needs HX711 calibration). A press currently calls `changeConfig(true)`, which cycles the demo `configs[]`. There is **no long-press detection**. `configs[3]` is "On/off, strong detent" (min 0, max 1, 60° per position, detent 1, snap 0.55).
- `display_task.cpp` renders `config.text`, the position number, a fill bar and a dot, so a menu can reuse it. No WiFi code exists. The README says RAM is tight (115 KB display sprite).

## Decisions made with the user
- **Stack:** Arduino-ESP32 3.x Matter library via pioarduino (not esp-matter/IDF).
- **Partitions:** keep OTA, 2 x ~2.75 MB app slots + NVS + ~1 MB FAT (`partitions-matter-8MB.csv`). First flash needs a full erase, which wipes saved config and strain calibration once.
- **Appliances:** modular `Appliance` interface + registry, **2 sockets for now**, each a Matter **On/Off Plug-in Unit (Outlet)** with two-way state sync with HomeKit.
- **Knob UX:** main menu (turn to choose Socket 1/2, **short press** on release confirms). In socket mode, turning flips OFF(0)/ON(1) using the `configs[3]`-style config. **Long press (1.5 s, constant `LONG_PRESS_MS`) fires while still held**, with haptic, and returns to the menu. The display shows ON/OFF instead of 0/1.
- Keep the USB-serial protobuf interface. Keep the old `view` env as fallback. Persist socket on/off states (proto change + nanopb regeneration).
- Matter endpoints are created at boot, so adding appliances needs a re-flash. Pairing uses test credentials. Apple Home needs a Home Hub.

## Plan artifact
`/Users/marcel/.gemini/antigravity-cli/brain/a7a5a599-eac2-43d5-807e-83beb6928c56/matter_implementation_plan.md` (v3, approved by the user: "yes"). It has no open questions.

## Phases
0. **Feasibility spike** (current)
1. Migrate the real firmware to pioarduino (no Matter). Re-check the motor, display, LEDs and serial.
2. `ApplianceManager`, menu + 2 sockets, press/long-press logic, persistence (no Matter yet).
3. `MatterTask`: 2 outlets, BLE commissioning, two-way sync.
4. Pairing screen, factory reset (10 s hold), README, 24 h soak, OTA test.

## State at the end of this chat
- Created a standalone spike in `spike/phase0/` (`platformio.ini`, `partitions.csv`, `src/main.cpp`). It prints chip/flash/PSRAM/heap, starts Matter with one `MatterOnOffPlugin`, and prints the pairing code/QR URL. **It builds successfully** (flash 1.44 MB of 2.88 MB, RAM 32%, static DRAM 84% full).
- PlatformIO + esptool were installed in a venv at `/Users/marcel/.gemini/antigravity-cli/brain/a7a5a599-eac2-43d5-807e-83beb6928c56/scratch/venv`. Verify it still exists, or reinstall. System `pio` is not installed.
- No board was connected (no `/dev/cu.usb*`). **No real firmware has been modified.**

## Next step (where we left off)
The user must connect the knob and flash the spike, then send me the serial output. These commands run from `spike/phase0`:
```
pio run -t erase     # wipes flash, including saved config and strain calibration
pio run -t upload
pio device monitor
```
The values to read are the `Chip:` line, `psram size=`, and the `heap free=` lines (boot, after Matter.begin, waiting). Use these to decide go/no-go:
- PSRAM works: move the display sprite to PSRAM, if it isn't too slow.
- PSRAM missing or slow, and the heap is too low: use a smaller sprite, or no display while WiFi is on.

After the numbers come in, start Phase 1. Ask before running anything that erases the board.
