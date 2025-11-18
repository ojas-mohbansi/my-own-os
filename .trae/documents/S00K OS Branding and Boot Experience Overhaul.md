## Objectives
- Rebrand the system to "S00K OS" everywhere (code, prompts, docs, build artifacts).
- Add a visual identity: logo, icon set, wallpapers (light/dark, 4K), and an animated boot screen suitable for current VGA text‑mode.
- Keep changes portable and compatible with current BIOS bootloader and kernel.

## Scope and Constraints
- Current OS uses VGA text mode and a BIOS bootloader; no graphics driver/UI yet.
- Boot visuals will be implemented as a text‑mode animation and status/progress indicators.
- UEFI support will be planned with assets and structure, but the actual UEFI bootloader is out of scope for this pass.

## Asset Plan
- Create `assets/branding/` with:
  - Vector logo: `s00k-os-logo.svg` (512×512+)
  - Raster icons: `png` at 16, 32, 48, 64, 128, 256, 512
  - Wallpapers (PNG): `s00k-os-wallpaper-light-4k.png`, `s00k-os-wallpaper-dark-4k.png`
  - README in `assets/branding/` describing usage and licensing
- Provide ASCII art banner for text‑mode boot to ensure consistent identity.

## Code Changes
- Add `brand.h` with centralized identifiers:
  - `#define OS_NAME "S00K OS"`
  - `#define OS_VERSION "1.0.0"`
  - `#define OS_CODENAME "S00K"`
- Update `kernel.c` welcome and status strings to use `OS_NAME` and show ASCII banner + spinner animation and progress messages.
- Update `shell.c` prompt to `s00k>` and help/about to show `OS_NAME` version/codename.
- Update `file_system.c`, `io.c`, error outputs where product name appears.
- Add a simple boot animation function in `kernel.c`:
  - Spinner/progress bar with phases: Security, Paging, Memory, FS, I/O, Shell
  - Per‑phase status and timing hooks (compatible with profiler when enabled)
- Prepare stubs for future UEFI boot logo display (commented, documented) without altering BIOS flow.

## Documentation Changes
- Rewrite titles and references in `docs/user_guide.md` and `docs/architecture_overview.md` from "MyOperatingSystem" to "S00K OS".
- Add a Branding section explaining assets, sizes, and usage.
- Update screenshots/examples (ASCII mock where images are not available).

## Build and Packaging
- Include assets in repo under `assets/branding/` (non‑boot critical).
- Keep image build unchanged (raw, BIOS boot). Add a note in README about non‑graphical boot.

## Verification
- Unit: verify strings updated via existing tests; adjust expectations if needed.
- Manual: boot in QEMU, confirm banner, spinner, and updated prompts.
- Resolutions: text‑mode scales; wallpaper/icon assets validated via external viewers.

## Deliverables
- Updated code: centralized branding constants, boot animation, shell prompt.
- Assets folder with logo, icons, wallpapers.
- Updated docs with S00K OS branding and instructions.

## Timeline
1. Add `brand.h`, update kernel/shell strings and boot animation.
2. Update docs and test strings.
3. Add assets folder with SVG/PNGs.
4. Verify in QEMU and finalize README.
