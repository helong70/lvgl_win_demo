# LVGL Windows Demo Release Package Script
# Usage: .\create_release.ps1 -Version v1.0.0 [-NoConsole]

param(
    [Parameter(Mandatory=$true, Position=0)]
    [string]$Version,
    
    [Parameter(Mandatory=$false)]
    [switch]$NoConsole
)

$ErrorActionPreference = "Stop"
$ProjectName = "LVGL_Windows_Demo"
$ReleaseName = "${ProjectName}_${Version}"
$ReleaseDir = "release\$ReleaseName"

# Determine which executable to use
if ($NoConsole) {
    $ExePath = "build\bin\LVGL_Windows_Demo_Noconsole.exe"
    $ReleaseName = "${ProjectName}_${Version}_Noconsole"
    $ReleaseDir = "release\$ReleaseName"
    $ExeName = "LVGL_Windows_Demo_Noconsole.exe"
    $BuildType = "Single-file no-console version (recommended)"
} else {
    $ExePath = "build\bin\LVGL_Windows_Demo.exe"
    $ExeName = "LVGL_Windows_Demo.exe"
    $BuildType = "Standard version"
}

Write-Host "================================" -ForegroundColor Cyan
Write-Host "Creating Release: $ReleaseName" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Build Type: $BuildType" -ForegroundColor Yellow

# Check if executable exists
if (-not (Test-Path $ExePath)) {
    Write-Host ""
    Write-Host "ERROR: Executable file not found: $ExePath" -ForegroundColor Red
    Write-Host ""
    if ($NoConsole) {
        Write-Host "Please run: .\build_ninja.bat pack" -ForegroundColor Yellow
    } else {
        Write-Host "Please run: .\build_ninja.bat" -ForegroundColor Yellow
    }
    exit 1
}

# Get file info
$ExeInfo = Get-Item $ExePath
Write-Host ""
Write-Host "Found: $($ExeInfo.Name)" -ForegroundColor Green
Write-Host "   Size: $([math]::Round($ExeInfo.Length/1MB, 2)) MB" -ForegroundColor Gray
Write-Host "   Modified: $($ExeInfo.LastWriteTime)" -ForegroundColor Gray

# Create release directory
Write-Host ""
Write-Host "Creating release directory..." -ForegroundColor Cyan
if (Test-Path $ReleaseDir) {
    Remove-Item -Path $ReleaseDir -Recurse -Force
}
New-Item -Path $ReleaseDir -ItemType Directory -Force | Out-Null

# Copy files
Write-Host "Copying files..." -ForegroundColor Cyan

# 1. Copy executable
Copy-Item $ExePath "$ReleaseDir\"
if ($NoConsole) {
    Write-Host "   - LVGL_Windows_Demo_Noconsole.exe (single-file, no-console)" -ForegroundColor Green
} else {
    Write-Host "   - LVGL_Windows_Demo.exe" -ForegroundColor Green
}

# 2. Copy LICENSE
if (Test-Path "LICENSE") {
    Copy-Item "LICENSE" "$ReleaseDir\"
    Write-Host "   - LICENSE" -ForegroundColor Green
}

# 3. Create README.txt
Write-Host "   - README.txt" -ForegroundColor Green

$ReadmeContent = @"
# LVGL Windows Demo $Version

A LVGL graphical interface demo project running on Windows platform.

## Quick Start

Double-click $ExeName to run.

## System Requirements

- Windows 10 or higher
- Graphics card with OpenGL 2.0+ support

## Features

- OpenGL hardware-accelerated rendering
- Modern borderless window with smooth rounded corners
- Full keyboard and mouse support
- Interactive UI widget demonstrations
"@

if ($NoConsole) {
    $ReadmeContent += "`n- Single-file version with no console window"
}

$ReadmeContent += @"


## Controls

- Drag title bar to move window
- Tab key to switch between input fields
- Ctrl+A/C/V keyboard shortcuts
- Click settings icon to open settings panel

## License

This project is licensed under GPL-3.0.

## Project Repository

https://github.com/helong70/lvgl_win_demo
"@

Set-Content -Path "$ReleaseDir\README.txt" -Value $ReadmeContent -Encoding UTF8

# 4. Copy screenshots (if exists)
if (Test-Path "screenshots\demo-screenshot.png") {
    $ScreenshotDir = "$ReleaseDir\screenshots"
    New-Item -Path $ScreenshotDir -ItemType Directory -Force | Out-Null
    Copy-Item "screenshots\demo-screenshot.png" "$ScreenshotDir\"
    Write-Host "   - screenshots\demo-screenshot.png" -ForegroundColor Green
}

# Create ZIP package
Write-Host ""
Write-Host "Creating ZIP package..." -ForegroundColor Cyan
$ZipPath = "release\$ReleaseName.zip"
if (Test-Path $ZipPath) {
    Remove-Item $ZipPath -Force
}
Compress-Archive -Path $ReleaseDir -DestinationPath $ZipPath -Force

$ZipInfo = Get-Item $ZipPath
Write-Host "   ✓ 压缩包已创建" -ForegroundColor Green
Write-Host "   路径: $ZipPath" -ForegroundColor Gray
Write-Host "   大小: $([math]::Round($ZipInfo.Length/1MB, 2)) MB" -ForegroundColor Gray

# 显示摘要
Write-Host "`n================================" -ForegroundColor Cyan
Write-Host "✅ 发布包创建完成！" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan
Write-Host "发布目录: $ReleaseDir" -ForegroundColor Yellow
Write-Host "压缩包: $ZipPath" -ForegroundColor Yellow

Write-Host "`n📤 下一步操作:" -ForegroundColor Cyan
Write-Host "1. 测试可执行文件: .\$ReleaseDir\LVGL_Windows_Demo.exe" -ForegroundColor White
Write-Host "2. 创建 Git 标签: git tag -a $Version -m 'Release $Version'" -ForegroundColor White
Write-Host "3. 推送标签: git push origin $Version" -ForegroundColor White
Write-Host "4. 在 GitHub 上创建 Release 并上传 $ReleaseName.zip" -ForegroundColor White

Write-Host "`n💡 提示:" -ForegroundColor Cyan
Write-Host "- 可以直接将 $ZipPath 上传到 GitHub Release" -ForegroundColor Gray
Write-Host "- 或者上传 $ReleaseDir 目录中的各个文件" -ForegroundColor Gray
Write-Host "- 详细说明请参考 RELEASE.md 文档" -ForegroundColor Gray

try {
    Compress-Archive -Path $ReleaseDir -DestinationPath $ZipPath -Force
    Write-Host "   - ZIP created: $ZipPath" -ForegroundColor Green
} catch {
    Write-Host "   - Warning: Cannot create ZIP automatically" -ForegroundColor Yellow
    Write-Host "     Please compress $ReleaseDir manually" -ForegroundColor Yellow
}

# List release package contents
Write-Host ""
Write-Host "Release package contents:" -ForegroundColor Cyan
Get-ChildItem -Path $ReleaseDir -Recurse -File | ForEach-Object {
    $relativePath = $_.FullName.Substring($ReleaseDir.Length + 1)
    $size = if ($_.Length -lt 1KB) { "$($_.Length) B" } 
            elseif ($_.Length -lt 1MB) { "$([math]::Round($_.Length/1KB, 2)) KB" }
            else { "$([math]::Round($_.Length/1MB, 2)) MB" }
    Write-Host "   $relativePath" -NoNewline -ForegroundColor Gray
    Write-Host " ($size)" -ForegroundColor DarkGray
}

Write-Host ""
Write-Host "================================" -ForegroundColor Cyan
Write-Host "Release package created!" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan
Write-Host "Release directory: $ReleaseDir" -ForegroundColor White
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
if ($NoConsole) {
    Write-Host "1. Test: .\$ReleaseDir\LVGL_Windows_Demo_Noconsole.exe"
} else {
    Write-Host "1. Test: .\$ReleaseDir\LVGL_Windows_Demo.exe"
}
Write-Host "2. Create tag: git tag -a $Version -m `"Release $Version`""
Write-Host "3. Push tag: git push origin $Version"
Write-Host "4. Create GitHub Release and upload ZIP file"
Write-Host ""
Write-Host "Tips:" -ForegroundColor Cyan
Write-Host "  - Recommended to use -NoConsole for release (no console window)"
Write-Host "  - See RELEASE.md for detailed instructions"
Write-Host ""
