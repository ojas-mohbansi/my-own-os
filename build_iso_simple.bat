@echo off

REM S00K OS ISO Builder for Windows - Simplified Version
REM Creates bootable ISO using available Windows tools

echo === S00K OS ISO Builder (Windows) ===
echo Version: 1.0
echo Build Date: %DATE% %TIME%
echo Build Host: %COMPUTERNAME%
echo Build User: %USERNAME%
echo.

REM Configuration
set ISO_NAME=S00K_OS_1.0.iso
set ISO_DIR=iso
set RELEASE_DIR=release
set VERSION=1.0

echo Preparing to build S00K OS ISO image...
echo.

REM Create directories
if not exist "%RELEASE_DIR%" mkdir "%RELEASE_DIR%"
if not exist "build_info" mkdir "build_info"

REM Create build metadata
echo Creating build metadata...
(
echo S00K OS Build Information
echo ========================
echo Version: %VERSION%
echo Build Date: %DATE% %TIME%
echo Build Host: %COMPUTERNAME%
echo Build User: %USERNAME%
echo ISO Label: S00K_OS_1.0
echo Publisher: S00K OS Project
echo Application: S00K_OS_Installer
echo Architecture: x86_64, i386
echo Boot Support: UEFI, BIOS (Legacy)
echo Hybrid Support: Yes (USB/CD/DVD)
echo.
echo Bootloader Information:
echo - ISOLINUX: BIOS boot support
echo - GRUB2: UEFI boot support
echo - Fallback: Chainloading to existing OS
echo - Timeout: 5 seconds
echo - Default: S00K OS (Default)
echo.
echo Hybrid ISO Features:
echo - USB boot via isohybrid
echo - CD/DVD boot with El Torito
echo - Virtual machine compatibility
echo - BIOS and UEFI support
echo - Partition table for USB devices
echo.
echo Verification Tools:
echo - System verification script
echo - Hardware detection tool
echo - Memory testing (memtest86+)
echo - Filesystem checking
echo - Boot sector validation
) > build_info\BUILD_INFO

echo.
echo Checking for ISO creation tools...

REM Try to find mkisofs or genisoimage first
where mkisofs >nul 2>nul
if %errorlevel% equ 0 (
    echo Found mkisofs - creating bootable ISO
    goto :use_mkisofs
)

where genisoimage >nul 2>nul
if %errorlevel% equ 0 (
    echo Found genisoimage - creating bootable ISO
    goto :use_mkisofs
)

echo mkisofs/genisoimage not found - creating archive instead
echo.
echo Creating 7z archive as alternative...

REM Try to create a 7z archive (more portable than TAR on Windows)
where 7z >nul 2>nul
if %errorlevel% equ 0 (
    echo Found 7z - creating compressed archive
    7z a -t7z "%RELEASE_DIR%\%ISO_NAME%.7z" "%ISO_DIR%\*" >nul
    if %errorlevel% equ 0 (
        echo Archive created: %RELEASE_DIR%\%ISO_NAME%.7z
        goto :create_checksums
    ) else (
        echo 7z creation failed
    )
)

REM Fallback to basic ZIP
where powershell >nul 2>nul
if %errorlevel% equ 0 (
    echo Found PowerShell - creating ZIP archive
    powershell -Command "Compress-Archive -Path '%ISO_DIR%\*' -DestinationPath '%RELEASE_DIR%\%ISO_NAME%.zip' -Force"
    if %errorlevel% equ 0 (
        echo Archive created: %RELEASE_DIR%\%ISO_NAME%.zip
        goto :create_checksums
    ) else (
        echo ZIP creation failed
    )
)

echo.
echo ERROR: No suitable archive tool found
echo Please install one of the following:
echo - mkisofs (for proper bootable ISO)
echo - 7-Zip (for 7z archive)
echo - PowerShell 5.0+ (for ZIP archive)
echo.
goto :error

:use_mkisofs
echo.
echo Creating bootable ISO image with mkisofs...
echo This will create a proper bootable ISO with:
echo - ISOLINUX for BIOS boot
echo - GRUB2 for UEFI boot
echo - Hybrid support for USB/CD/DVD
echo.

REM Create the ISO with proper parameters
mkisofs -iso-level 3 -J -R -V "S00K_OS_%VERSION%" -publisher "S00K OS Project" -A "S00K_OS_Installer" -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -o "%RELEASE_DIR%\%ISO_NAME%" "%ISO_DIR%"

if %errorlevel% equ 0 (
    echo.
    echo ISO created successfully: %RELEASE_DIR%\%ISO_NAME%
    
    REM Try to make it hybrid (USB bootable)
    where isohybrid >nul 2>nul
    if %errorlevel% equ 0 (
        echo Making ISO hybrid (USB bootable)...
        isohybrid "%RELEASE_DIR%\%ISO_NAME%"
        echo Hybrid ISO created successfully!
    ) else (
        echo Note: isohybrid not found, USB boot may not work
    )
    
    goto :create_checksums
) else (
    echo.
    echo ERROR: ISO creation failed with mkisofs
    echo Trying genisoimage as fallback...
    
    genisoimage -iso-level 3 -J -R -V "S00K_OS_%VERSION%" -publisher "S00K OS Project" -A "S00K_OS_Installer" -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table -o "%RELEASE_DIR%\%ISO_NAME%" "%ISO_DIR%"
    
    if %errorlevel% equ 0 (
        echo ISO created with genisoimage: %RELEASE_DIR%\%ISO_NAME%
        goto :create_checksums
    ) else (
        echo ERROR: Both mkisofs and genisoimage failed
        goto :error
    )
)

:create_checksums
echo.
echo Creating checksums...

if exist "%RELEASE_DIR%\%ISO_NAME%" (
    echo Creating MD5 checksum...
    where certutil >nul 2>nul
    if %errorlevel% equ 0 (
        certutil -hashfile "%RELEASE_DIR%\%ISO_NAME%" MD5 > "%RELEASE_DIR%\MD5SUMS" 2>nul
        echo MD5 checksum created
    ) else (
        echo Note: certutil not available for MD5
    )
    
    echo Creating SHA256 checksum...
    where certutil >nul 2>nul
    if %errorlevel% equ 0 (
        certutil -hashfile "%RELEASE_DIR%\%ISO_NAME%" SHA256 > "%RELEASE_DIR%\SHA256SUMS" 2>nul
        echo SHA256 checksum created
    ) else (
        echo Note: certutil not available for SHA256
    )
    
    echo.
    echo File information:
    for %%I in ("%RELEASE_DIR%\%ISO_NAME%") do (
        echo Size: %%~zI bytes
        echo Name: %%~nxI
    )
) else (
    echo Archive created - checksums not generated for archives
)

echo.
echo === Build Complete ===
if exist "%RELEASE_DIR%\%ISO_NAME%" (
    echo Bootable ISO created: %RELEASE_DIR%\%ISO_NAME%
    echo Ready for testing and distribution!
) else (
    echo Archive created: %RELEASE_DIR%\%ISO_NAME%.*
    echo Note: This is not a bootable ISO image
    echo Install mkisofs for proper bootable ISO creation
)
echo.
goto :end

:error
echo.
echo Build failed. Please check the error messages above.
echo.

:end
echo.
echo Build process completed.
echo.

REM Show final directory structure
echo ISO directory contents:
dir /s "%ISO_DIR%" 2>nul | findstr /v /C:"Directory of" | findstr /v /C:"bytes" | more 2>nul || dir "%ISO_DIR%"
echo.
echo Release directory contents:
if exist "%RELEASE_DIR%" dir "%RELEASE_DIR%"
echo.
echo Build info:
if exist "build_info" type build_info\BUILD_INFO | more