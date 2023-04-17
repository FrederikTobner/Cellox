-- Builds the Cellox project using CMake and Ninja

-- determine if the script is running on Windows
local is_windows = package.config:sub(1,1) == '\\'

local buildType = arg[1] or "Release"
local buildTarget = arg[2] or "all"
if is_windows then
  os.execute([[call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"]])
end
print("Configuring CMake ...")
os.execute(string.format("cmake -B ../build -DCMAKE_BUILD_TYPE=%s -G \"Ninja\" ..", buildType))
print("Building CHIP-8 ...")
os.execute(string.format("cmake --build ../build --config %s --target %s", buildType, buildTarget))