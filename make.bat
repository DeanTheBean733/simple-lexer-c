@echo off

echo Building the project...

gcc -Wall -Wextra -std=c99 -o example.exe example.c simple-lexer.c
if errorlevel 1 (
    echo Build failed.
    pause
    exit /b %errorlevel%
)

echo Build successful. Running the executable...
example.exe

pause
