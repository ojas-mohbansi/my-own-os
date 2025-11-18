## Overview
- Adapt the provided `setup_environment.sh` (Linux/Debian) for your Windows environment
- Install and verify compiler, emulator, editor, and Git
- Create `src`, `bin`, `docs` and build a "Hello, World!" to validate

## Review of Provided Script
- Purpose: installs GCC, QEMU, Git, Vim; creates `MyOperatingSystem` with `src/bin/docs`; writes `hello.c`
- Linux‑specific: uses `apt-get` and `bash`; suitable for Ubuntu/Debian or WSL Ubuntu
- Minor improvements if used on Linux: add `sudo apt-get update`, use correct QEMU package (`qemu-system-x86`), use `int main(void)` signature

## Windows PowerShell Equivalent (Proposed)
- Create `setup_environment.ps1` (to be run in an elevated PowerShell):
  ```powershell
  # Ensure apps via winget; fall back with hints if winget unavailable
  function Ensure-App($Id, $Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
      Write-Host "$Name is not installed. Installing..."
      winget install --id $Id -e | Out-Host
    } else {
      Write-Host "$Name is already installed."
    }
  }

  # Install tools (IDs may vary per catalog)
  Ensure-App 'MSYS2.MSYS2' 'bash'
  Ensure-App 'Git.Git' 'git'
  Ensure-App 'Microsoft.VisualStudioCode' 'code'
  Ensure-App 'SoftwareFreedomConservancy.QEMU' 'qemu-system-x86_64'

  # After MSYS2 installs, add mingw64 to PATH (manual step):
  # Add C:\msys64\mingw64\bin to your user PATH; restart PowerShell

  # Verify GCC (via MSYS2 mingw64)
  if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    Write-Host "Installing GCC toolchain in MSYS2..."
    Write-Host "Open 'MSYS2 MinGW 64-bit' and run: pacman -Syu"
    Write-Host "Then run: pacman -S --needed base-devel mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-gdb"
  }

  # Project structure
  $PROJECT_DIR = "c:\\Users\\styli\\OneDrive\\Desktop\\my-own-os"  # reuse existing folder
  New-Item -ItemType Directory -Force -Path "$PROJECT_DIR\\src" | Out-Null
  New-Item -ItemType Directory -Force -Path "$PROJECT_DIR\\bin" | Out-Null
  New-Item -ItemType Directory -Force -Path "$PROJECT_DIR\\docs" | Out-Null

  # Hello World
  $hello = @"
  #include <stdio.h>
  int main(void) {
      printf("Hello, World!\n");
      return 0;
  }
  "@
  Set-Content -Path "$PROJECT_DIR\\src\\hello.c" -Value $hello -Encoding ASCII

  Write-Host "Build with: gcc $PROJECT_DIR\\src\\hello.c -o $PROJECT_DIR\\bin\\hello.exe"
  Write-Host "Run with: $PROJECT_DIR\\bin\\hello.exe"

  # Quick version checks
  Write-Host "Versions:"; foreach ($cmd in @('gcc','git','code','qemu-system-x86_64')) { try { & $cmd --version | Select-Object -First 1 } catch {} }
  ```
- Outcome: tools installed, workspace created, `hello.c` ready; compile/run validates setup

## WSL/Linux Alternative (Using Your Bash Script)
- If using Ubuntu (native or WSL), use your script with small fixes:
  - Add update first: `sudo apt-get update`
  - Install build essentials: `sudo apt-get install -y build-essential`
  - Install QEMU: `sudo apt-get install -y qemu-system-x86`
  - Keep Git/Vim as in the script
- Compile and run:
  - `gcc MyOperatingSystem/src/hello.c -o MyOperatingSystem/bin/hello`
  - `./MyOperatingSystem/bin/hello`

## Verification Steps
- Check versions:
  - Windows: `gcc --version`, `git --version`, `code --version`, `qemu-system-x86_64 --version`
  - WSL/Linux: `gcc --version`, `git --version`, `vim --version`, `qemu-system-x86_64 --version`
- Directory tree exists with `src`, `bin`, `docs`
- "Hello, World!" prints once when executed

## Acceptance Criteria Mapping
- Tools installed and functioning: verified via version checks
- Project directory created: under `c:\Users\styli\OneDrive\Desktop\my-own-os` (or `MyOperatingSystem` on Linux)
- Build and run program: GCC compiles `hello.c` to `bin/hello(.exe)` and runs successfully

## Next Step
- With the environment validated, proceed to introduce a true cross‑compiler (`i686-elf-gcc`) and a minimal bootable image for QEMU in the following step. We can do this on Windows via MSYS2 or under WSL to simplify the build process.