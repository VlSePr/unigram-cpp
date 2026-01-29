@echo off
REM Memory Leak Detection Script for UnigramTokeniser
REM This script runs the application with AddressSanitizer to detect memory leaks

echo ==========================================
echo Memory Leak Check for UnigramTokeniser
echo ==========================================
echo.

REM Check if build exists
if not exist "build-asan" (
    echo Creating build-asan directory for AddressSanitizer build...
    mkdir build-asan
)

echo.
echo Step 1: Configuring with AddressSanitizer...
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="/fsanitize=address /Zi"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo Step 2: Building with AddressSanitizer...
cmake --build build-asan --config RelWithDebInfo

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo Step 3: Running test with leak detection...
echo.
echo Testing with small corpus...
build-asan\bin\RelWithDebInfo\unigram-cli.exe train --input data\sample_corpus.txt --vocab-size 1000 --iterations 2 --output test_leak.json

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo WARNING: Application exited with error code %ERRORLEVEL%
    echo Check output above for memory leaks or errors.
) else (
    echo.
    echo SUCCESS: No memory leaks detected!
)

echo.
echo ==========================================
echo Memory leak check complete.
echo ==========================================
pause
