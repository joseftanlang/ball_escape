
**Overview**
- **Project**: Embedded games firmware and framework for the AK Embedded Base Kit (STM32L151).
- **Purpose**: Provide a small, portable game platform and example games (including Ball Escape) that run on the AK base kit hardware. The codebase includes a lightweight cooperative event kernel, drivers for display, input and peripherals, example game screens, build scripts and flashing helpers.

**What to expect**
- **Hardware target**: STM32L151-series microcontrollers (example: `STM32L151CBT6`).
- **Display**: OLED drivers included (SH1106 / SSD1309 compatible) via Adafruit-style GFX wrapper.
- **Build system**: GNU Make + GNU ARM Embedded toolchain. Flashing supported via STM32CubeProgrammer and an `ak-flash` helper for the AK bootloader.
- **Example games / screens**: `ball_escape` (this repo), `archery-game`, `peashooter` and several utility/demo screens under `application/sources/app/screens`.

**Quick Highlights**
- **Main build file**: [application/Makefile](application/Makefile#L1) — controls compiler paths, build flags, and flash targets.
- **Game sources**: [application/sources/app/screens](application/sources/app/screens#L1) — where screen/game logic lives.
- **Core framework**: [application/sources/ak](application/sources/ak#L1) — small message/timer/task kernel used across screens.
- **Platform layer**: [application/sources/platform/stm32l](application/sources/platform/stm32l#L1) — CMSIS/StdPeriph glue, linker script and platform helpers.

**Requirements**
- **Host tools**: GNU Make, bash/Git-Bash or POSIX shell recommended for full feature set.
- **Compiler**: GNU ARM Embedded toolchain (arm-none-eabi-*). The Makefile shows an example Windows path; on Linux/macOS set `GCC_PATH` in `application/Makefile` or install the toolchain to a standard path.
- **Flashing / Debugging**: STM32CubeProgrammer (CLI) or OpenOCD + GDB. The Makefile has hooks for both: `make flash` (uses STM32_Programmer_CLI by default) and `make debug` (OpenOCD + GDB).
- **Optional**: `ak-flash` (for bootloader flashing), `minicom` or similar serial terminal for console access.

**Build & Flash (quick)**
- **Build**: From repository root run:

```bash
cd application
make all
```

- **Flash (STM32CubeProgrammer)**: (Windows example in Makefile) — from `application` run:

```bash
make flash
```

- **Flash via bootloader**: (requires `ak-flash` installed)

```bash
make flash dev=/dev/ttyUSB0
```

- **Open serial console**:

```bash
make com dev=/dev/ttyUSB0
```

- **Debug (GDB)**:

```bash
make debug
# or use `make debug gdb=ddd` to open DDD UI if installed
```

See [application/Makefile](application/Makefile#L1) for configuration knobs such as `GCC_PATH`, `APP_START_ADDR` and hardware driver options.

**Project layout (high level)**
- **application/**: Main firmware project and Makefile. See [application/Makefile](application/Makefile#L1).
- **application/sources/**: Source tree split into logical areas:
	- **ak/**: tiny framework (message, timer, task state machine) used by all apps.
	- **platform/stm32l/**: CMSIS/StdPeriph integration, linker script (`ak.ld`) and low level startup.
	- **driver/**: Peripheral drivers (OLED display, button, buzzer, eeprom, flash, gpio).
	- **app/**: Application-level tasks and screens (`task_display`, `task_shell`, game screens).
	- **libraries/** and **networks/**: third-party libs and modbus stacks.
- **application/sources/app/screens/**: all game screens and bitmaps. The `ball_escape` game is implemented in `ball_escape.cpp` (see [application/sources/app/screens/ball_escape.cpp](application/sources/app/screens/ball_escape.cpp#L1)).
- **hardware/**: board images, manufacturing and schematic documentation.

**Ball Escape — game summary**
- **File**: [application/sources/app/screens/ball_escape.cpp](application/sources/app/screens/ball_escape.cpp#L1)
- **Gameplay**: A small arcade-style demo where a 5x5 pixel ball bounces vertically and interacts with one or more shrinking rings (circles) that have an angular gap. The player rotates the ring to let the ball escape through the gap. Scoring, ring spawn/shrink logic and collision handling are implemented.
- **Key parameters** (defined in the source): circle center/radius, gap width, spawn interval, ball velocity and score update rules. These constants are near the top of `ball_escape.cpp` and can be tuned for difficulty.
- **Input & feedback**: rotation triggered by button events handled in the screen's message handler (`scr_circle_escape_handle`). Buzzer sounds are used for UX feedback.

**Screens (detailed descriptions)**

- `scr_startup` ([application/sources/app/screens/scr_startup.cpp](application/sources/app/screens/scr_startup.cpp#L1)) — The startup screen is the first visible surface the firmware presents after display initialization. Its primary responsibilities are hardware-friendly: initialize the `view_render` display abstraction, enable display power with `view_render_display_on()`, set a one-shot timer for the idle-transition (`AC_DISPLAY_SHOW_IDLE`) and optionally play a short startup tune. The implementation draws a full-screen bitmap (the Vietnam flag in this repository) and exposes simple button-driven navigation: `MODE` advances to the `scr_dvd` demo and `DOWN` jumps directly into the `scr_circle_escape` game. The handler toggles a silent flag on `UP` so the buzzer can be muted. From a developer perspective, `scr_startup` is intentionally minimal (no heavy loops) and uses the kernel timers and `SCREEN_TRAN` calls to hand off control to other screens. It demonstrates the recommended pattern for screen entry: initialize resources, set timers, draw once, then return to the event-driven loop so the system stays responsive to buttons and other tasks.

- `scr_idle` ([application/sources/app/screens/scr_idle.cpp](application/sources/app/screens/scr_idle.cpp#L1)) — `scr_idle` is a versatile demo and simple stress test for the renderer: it maintains a dynamic vector of `ball` objects that move and bounce around the 128×64 canvas. Each `ball` carries position, velocity-like slope, radius and movement axes; movement is computed with lightweight integer math (and a few uses of `atan`). The screen uses a periodic timer `AC_DISPLAY_SHOW_IDLE_BALL_MOVING_UPDATE` to advance positions, and the view function (`view_scr_idle`) redraws all active balls each tick. Button interactions are deliberately simple and illustrative: `UP` adds a new animated ball (up to a max), `DOWN` removes one, and `MODE` transitions to the next demo. The screen shows how to manage variable-sized animation state safely in embedded C++ (push/pop from `vector`, guard timers when the last ball is removed) while keeping CPU work bounded. It's useful for measuring frame costs, memory use, and rendering latency for new bitmaps or algorithms.

- `scr_dvd` ([application/sources/app/screens/scr_dvd.cpp](application/sources/app/screens/scr_dvd.cpp#L1)) — This screen implements a classic DVD-logo bounce demo optimized for small displays. The screen stores an (x,y) position and (vx,vy) velocity for the DVD bitmap and updates them at a configurable interval driven by `AC_DISPLAY_SHOW_DVD_UPDATE`. The movement logic enforces boundary conditions (flip velocity when hitting an edge, clamp position) and redraws with `view_render.drawBitmap`. Buttons allow the user to accelerate or decelerate the update interval (`UP`/`DOWN`) within defined bounds; `MODE` transitions to `scr_soccer`. The screen demonstrates efficient double-buffer-friendly rendering (redraw only the moving bitmap and a bounding frame) and exposes how to safely change timer intervals at runtime using `timer_set` and `timer_remove_attr`. It’s ideal for quick visual tests of display refresh and to tune animation update rates against the MCU’s available CPU budget.

- `scr_peashooter` ([application/sources/app/screens/scr_peashooter.cpp](application/sources/app/screens/scr_peashooter.cpp#L1)) — `scr_peashooter` is a small sprite animation example composed of four frames (25×13 each) plus a static background (56×64). The screen keeps simple state (frame index, x/y position and a configurable shooting interval), advances the frame and moves the sprite at each periodic tick, and draws both the background and the current animation frame. The input handlers (`UP`/`DOWN`) tune the `shooting_interval_ms` (bounded by `MIN_SHOOTING_INTERVAL_MS`/`MAX_SHOOTING_INTERVAL_MS`) by reconfiguring the shared update timer `AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE`. It’s an excellent reference for adding multi-frame sprite animations: memory layout of frame bitmaps in `screens_bitmap.cpp`, clean state machine in the message handler, and how to change refresh frequency without reworking the renderer. Use it as a template when adding other multi-frame characters or small cutscenes.

- `scr_circle_escape` / `ball_escape` ([application/sources/app/screens/ball_escape.cpp](application/sources/app/screens/ball_escape.cpp#L1)) — The Ball Escape game is the most interaction-rich screen in the repo and demonstrates geometry, collision detection and game rules in tight embedded C++. The screen defines constants for ball bitmap size, circle center and radii, gap width, rotation step and spawn/shrink parameters. Core systems implemented include: continuous ball motion (fixed small vertical steps and multi-step per tick to improve numerical stability), radial collision detection (distance squared checks against inner/outer radius bands), angle normalization and a gap membership test to determine whether the ball escaped through the moving gap or hit the ring. Game flow uses periodic ticks for motion updates and spawns new rings every `CIRCLE_SPAWN_INTERVAL_TICKS`, then shrinks rings over time; scoring is stored to EEPROM via `ar_game_score_write`. User inputs (`UP`/`DOWN`) rotate the ring by `CIRCLE_ROTATE_STEP_DEG`, and the code includes careful overlap handling (`push_ball_out_of_circle`) to avoid repeated immediate collisions. This screen is a great study in translating continuous geometric mechanics into integer-friendly embedded code while keeping UX feedback (buzzer, score display) compact and non-blocking.

- `scr_charts_game` / `scr_score` ([application/sources/app/screens/scr_score.cpp](application/sources/app/screens/scr_score.cpp#L1)) — The charts/high-score screen focuses on persistent state, UI layout and safe EEPROM usage. It reads an `ar_game_score_t` structure from non-volatile storage using `ar_game_score_read()` and renders the top three scores using a large `iconLeaderBoard` bitmap and numeric text primitives. The handler supports basic editing: `UP` resets the scores to zero (and writes them back with `ar_game_score_write()`), `DOWN` jumps to `scr_circle_escape`, and `MODE` transitions to `scr_info` for additional metadata. This screen provides an example of how to combine large static bitmaps (careful flash usage), minimal text typography on a small display, and non-volatile updates — including when to throttle writes to EEPROM (write infrequently, e.g., on game-over or explicit reset) to avoid wear.

- `scr_soccer` ([application/sources/app/screens/scr_soccer.cpp](application/sources/app/screens/scr_soccer.cpp#L1)) — `scr_soccer` is a multi-phase minigame with clear state separation: selection, countdown, playing, and result. It models two roles (goalkeeper vs shooter) and toggles behavior accordingly. Internally it uses a `soccer_game_t` state struct with fixed-point motion (scales positions by `BALL_POSITION_SCALE`), separate timers for countdown/playing/result, and logic for goal detection, keeper collisions and `super_mode` activation. Input handling respects a cooldown to prevent rapid repeated commands, and the playing phase uses periodic `PLAY_TICK_MS` ticks to advance physics and AI (keeper movement). The screen shows advanced patterns for embedded games: predictable state transitions, use of fixed-point to avoid floating-point heavy computation, temporarily enabling special modes (super-mode with LED blink), and clean timer management when switching phases. Consider it a blueprint for any small competitive game with discrete phases and simple physics.

- `scr_info` ([application/sources/app/screens/scr_info.cpp](application/sources/app/screens/scr_info.cpp#L1)) — The info screen is intentionally simple but demonstrates a useful utility: rendering a QR code (via `qrcode_*` helpers) on the display to link to external documentation (the AK base kit GitHub). On entry it sets a one-shot timer to return to the idle screen after the configured logo interval, and it plays a small buzzer sound as feedback. The implementation illustrates how to integrate small third-party helpers (QR code generation) and how to draw bitmaps/pixel-level primitives directly with `view_render.drawPixel`. Use this screen to expose documentation links, Wi‑Fi codes, or other metadata without adding heavy I/O or network stacks.

- `scr_es35sw_th_sensor` ([application/sources/app/screens/scr_es35sw_th_sensor.cpp](application/sources/app/screens/scr_es35sw_th_sensor.cpp#L1)) — This screen is an example of instrumentation UI tied to Modbus data sources. It reads temperature and humidity device registers from `MB_ES35SW_TH_Sensor` via periodic `AC_DISPLAY_SHOW_MODBUS_PULL_UPDATE` ticks and formats the values using device-specific scales/units. The view draws an icon, numeric values (with bounds checks applied), and uses `timer_set`/`timer_remove_attr` to schedule a return to the idle screen. The handler shows how to combine raw device data with UI code: apply ratios/units, protect against unrealistic readings (like >100% humidity), and trigger short buzzer tones on state changes. This is the canonical pattern for any sensor-status screen that reads from background collectors or a Modbus master.

- `scr_lhio404_io_device` ([application/sources/app/screens/scr_lhio404_io_device.cpp](application/sources/app/screens/scr_lhio404_io_device.cpp#L1)) — Focused on control rather than display-only telemetry, this screen presents a 4-relay control grid to the user. It uses a `focus_item` index to highlight which relay box is selected, draws on/off state for each relay based on `MB_LHIO404_IO_Device.listRegDevice[]`, and allows toggling via the `MODE` button. Toggling attempts to write a single coil using `eMBMWriteSingleCoil` through the Modbus master (`xMBMMaster`) and logs the result. The screen demonstrates safe UI/UX for control surfaces: visual focus, tactile feedback via buzzer, and graceful fallback to `scr_idle` when device entries are out-of-range. It’s a practical template for building simple actuator control panels backed by fieldbuses.

- `scr_noen` ([application/sources/app/screens/scr_noen.cpp](application/sources/app/screens/scr_noen.cpp#L1)) — A seasonal/holiday demo that layers simple sprite arrays to create falling-snow visuals over a festive bitmap. On entry it seeds randomized snow icon positions, starts a periodic snow-update timer and plays a jingle via the buzzer. The update handler advances small snowflakes down the screen and resets them to the top when they pass the bottom; a one-shot timer (`AC_DISPLAY_SHOW_MERRY_CHRISTMAS_SLEEP`) transitions back to idle after a defined sleep interval. This screen shows how to create charming, non-interactive animations that are also power-friendly: small bitmaps, short timers and a limited update frequency so the MCU can quickly return to low-power work or service other tasks.

**How the framework fits together**
- **Event kernel**: `application/sources/ak/src` implements an event/message and timer system used by `task_display` and each screen. Screens register handlers and receive `ak_msg_t` messages for entry/exit, button events and timed updates.
- **Rendering**: A display abstraction (`view_render`) sits on top of the Adafruit-style driver. Screens draw primitives (lines, circles, bitmaps) and schedule periodic updates via the kernel timers.

**Customize & add a new game screen**
- Summary steps (detailed guide is included in `way-to-start-coding-a-game-on-ak-base-kit.md`):
	1. Add a header in `application/sources/app/screens` for your screen (declaration + handler).
	2. Add the implementation `.cpp` with drawing + message handler logic.
	3. Add your bitmap assets to `screens_bitmap.cpp` or separate resource files.
	4. Add the new `.cpp` and bitmap files to `application/sources/app/screens/Makefile.mk` so they are compiled.
	5. Build and flash. Use `make com` to test serial output and `make debug` for live debugging.

**Developer notes & important build knobs**
- `APP_START_ADDR_VAL` in [application/Makefile](application/Makefile#L1) defines reserved boot area. Match this with your linker script (`application/sources/platform/stm32l/ak.ld`).
- `GCC_PATH` must point to an installed `arm-none-eabi` toolchain. On Linux you can also rely on PATH if a system toolchain is installed.
- `HARDWARE_OPTION` flags in `application/Makefile` select display driver macros (SSD1309/SH1106). Change these to match your hardware revision.
- Use `make info` to print flash / RAM usage; useful before adding large bitmaps or C++ objects.

**Troubleshooting**
- Build errors about missing toolchain: verify `GCC_PATH` and `arm-none-eabi-gcc` are reachable.
- Flashing failures: ensure target is powered and SWD/ST-Link or bootloader port is correct. Try `make flash dev=/dev/ttyUSB0` (bootloader) or configure STM32CubeProgrammer path.
- Display artifacts: check correct `HARDWARE_OPTION` flags and the display wiring/drivers.

**References & guides**
- Developer quickstart: [way-to-start-coding-a-game-on-ak-base-kit.md](way-to-start-coding-a-game-on-ak-base-kit.md#L1) — a step-by-step guide (with screenshots) for creating screens and building/flashing.
- STM32 tools: STM32CubeProgrammer, GNU ARM Embedded toolchain, OpenOCD.

**License & credits**
- See `LICENSE` at repository root for licensing terms.
- This project uses several bundled libraries (Adafruit GFX, STM32 StdPeriph) — see `application/sources/driver` and `application/sources/platform/stm32l/Libraries` for their respective sources and licenses.

---
If you'd like, I can:
- expand the README with a Mermaid architecture diagram, or
- add a short Quick Start script (`scripts/setup-dev.sh`) that sets up a recommended Linux toolchain and environment variables for building and flashing.

