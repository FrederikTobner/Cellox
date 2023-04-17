-- Installs CHIP-8 using CMake and Ninja

-- determine if the script is running on Windows
local is_windows = package.config:sub(1,1) == '\\'

local buildType = arg[1] or "Release"
local buildTarget = arg[2] or "all"

if is_windows then
    os.execute([[call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"]])
end
print("Configuring CMake ...")
os.execute(string.format("cmake -B ../build -DCMAKE_BUILD_TYPE=%s -G \"Ninja\" ..", buildType))
print("Building Cellox ...")
os.execute(string.format("cmake --build ../build --config %s --target %s", buildType, buildTarget))
print("Installing Cellox ...")
os.execute("cmake --install ../build/chip8/main/src/ --config Release")