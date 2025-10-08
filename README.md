# final-project
A please

## Build & Test

- Build app:
  - `gcc -o payment.exe payment.c`

- Run unit tests (AAA style):
  - `gcc -DUNIT_TEST -o test_payment_unit.exe test_payment_unit.c payment.c`
  - `./test_payment_unit.exe`

- Run end-to-end (E2E):
  - `powershell -ExecutionPolicy Bypass -File .\test_payment_e2e.ps1`

Notes:
- Header `payment.h` declares all function prototypes and shared globals.
- `payment.c` wraps `main` with `#ifndef UNIT_TEST` so test binaries can link without conflicts.

In-App UI Shortcuts
- From the main menu, choose `5` to build and run unit tests.
- Choose `6` to run the PowerShell E2E harness.

Security & Robustness
- CSV load caps at `MAX` (100); extra rows are ignored.
- Invalid payment IDs are rejected during load (`^P\d{3}$`).
- Interactive numeric inputs use `fgets` + parse with validation loops.
- Saving is atomic: writes to `*.tmp` and renames to the final file.
- CSV injection mitigation: fields starting with `= + - @` are prefixed with `'` when saving.
- Platform: in-app runners for tests are wrapped with `#ifdef _WIN32`.

Suggested Build Flags
- Debug: `-std=c11 -Wall -Wextra -Werror -O0 -g -fsanitize=address,undefined`
- Release: `-std=c11 -O2 -DNDEBUG`
Security & Robustness (extended)
- CSV quoting per RFC4180: fields containing `,` or `"` are quoted and quotes are doubled.
- Saving is atomic: writes to `*.tmp`, fsync/commit, then renames to the final file; `.lock` file prevents concurrent writers.
- Amount now uses `double` (was `float`).
- CSV path configurable via env var `PAYMENT_CSV`.
