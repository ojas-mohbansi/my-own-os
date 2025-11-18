## Objectives
- Complete visual identity for S00K OS: logo, icon set, wallpapers (light/dark 4K) and text-mode boot animation.
- Finish system-wide string updates and version/codename surfacing.
- Ensure consistency across docs, shell prompts, and startup messages; prepare for future graphical mode.

## Assets Creation
- Create `assets/branding/` with:
  - Vector logo `s00k-os-logo.svg` (≥512×512, square safe area, transparent background)
  - PNG icons: 16, 32, 48, 64, 128, 256, 512 (optimized, sRGB)
  - Wallpapers: `s00k-os-wallpaper-light-4k.png`, `s00k-os-wallpaper-dark-4k.png` with scalable composition and safe margins
  - `assets/branding/README` describing usage, sizes, and licensing

## Boot Screen (Text-Mode)
- Add `boot_animation()` in `kernel.c`:
  - ASCII banner with OS name/version
  - Spinner/progress bar; status lines for Security, Paging, Memory, FS, I/O, Shell
  - Timing hooks compatible with profiler when enabled
- Guard animation with a simple flag (e.g., `BRAND_ANIM_ENABLED`) for tests/dev.

## System-Wide String Updates
- Centralize via `brand.h`:
  - `OS_NAME`, `OS_VERSION`, `OS_CODENAME`
- Apply across:
  - `kernel.c` startup/demos
  - `shell.c` prompt (`s00k>`), help/about strings, login banner
  - Any remaining hard-coded references in docs and examples

## Documentation
- Update `docs/user_guide.md` and `docs/architecture_overview.md` branding sections to reference assets and boot experience.
- Add a short “Branding Assets” section with file list and integration notes.

## Compatibility Notes (Future)
- Prepare placeholders for framebuffer graphics mode (UEFI/BIOS VESA):
  - Document expected entry points for graphical boot and image blitting
  - Keep current build BIOS/VGA text-mode; UEFI work tracked as a future milestone

## Verification
- Visual: run in QEMU and validate banner, spinner, and messages alignment on 80×25
- Strings: grep to ensure no leftover “MyOperatingSystem” references
- Docs: confirm updated titles and examples

## Deliverables
- `assets/branding/` with SVG/PNGs and README
- Text-mode boot animation integrated (flag-toggleable)
- Updated prompts and messages wired to `brand.h`
- Documentation updated to reflect new branding and assets

## Next Actions
- Add assets folder and boot animation function; wire brand constants across modules.
- Update documentation and run quick text-mode verification in QEMU.
