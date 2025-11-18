@echo off
echo Testing S00K OS boot...
echo Starting QEMU with S00K_OS.img...
"C:\Program Files\qemu\qemu-system-i386.exe" -fda S00K_OS.img -boot a -nographic -serial stdio -no-reboot -no-shutdown
echo QEMU test completed.
echo.
echo Checking if OS image contains valid boot signature...
powershell -Command "$bytes = Get-Content S00K_OS.img -Encoding Byte -TotalCount 512; $signature = $bytes[510..511]; if ($signature[0] -eq 0x55 -and $signature[1] -eq 0xAA) { Write-Host 'Boot signature is VALID (55 AA)' -ForegroundColor Green } else { Write-Host 'Boot signature is INVALID' -ForegroundColor Red }"
echo.
echo Checking if kernel is present in sector 2...
powershell -Command "$bytes = Get-Content S00K_OS.img -Encoding Byte -TotalCount 1024; $kernel_start = $bytes[512..515]; if ($kernel_start[0] -ne 0 -or $kernel_start[1] -ne 0 -or $kernel_start[2] -ne 0 -or $kernel_start[3] -ne 0) { Write-Host 'Kernel code detected in sector 2' -ForegroundColor Green } else { Write-Host 'No kernel code found in sector 2' -ForegroundColor Red }"
echo.
echo OS build and test summary:
for %%I in (S00K_OS.img) do echo - Image size: %%~zI bytes
for %%I in (S00K_OS.img) do set size=%%~zI
for /f %%I in ('powershell -Command "[math]::floor(%size%/512)"') do echo - Total sectors: %%I sectors
echo.
echo Test complete!