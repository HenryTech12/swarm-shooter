g++ main.cpp -o game.exe -Iinclude -Llib -lraylib -lopengl32 -lgdi32 -lwinmm
if %errorlevel% equ 0 (
    game.exe
)