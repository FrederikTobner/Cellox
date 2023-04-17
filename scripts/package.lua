-- Packages the project as a zip file for Windows and Linux for x86 and x64
-- and creates an installer for Windows using NSIS (nullsoft scriptable install system)

-- determine if the script is running on Windows
local is_windows = package.config:sub(1,1) == '\\'

-- function to execute a command and return the output
local function execute_command(cmd)
  local handle = io.popen(cmd)
  local result = handle:read("*a")
  handle:close()
  return result
end

-- x64
if is_windows then
  execute_command([[call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"]])
end
print("Configuring CMake for x64 ...")
execute_command("cmake -B ../build/x64 -DCMAKE_BUILD_TYPE=Release -G \"Ninja\" ..")
print("Building Compiler for x64 ...")
execute_command("cmake --build ../build/x64 --config Release --target Cellox")

-- x86
if is_windows then
  execute_command([[call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"]])
end
print("Configuring CMake for x86 ...")
execute_command("cmake -B ../build/x86 -DCMAKE_BUILD_TYPE=Release -G \"Ninja\" -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 ..")
print("Building Compiler for x86 ...")
execute_command("cmake --build ../build/x86 --config Release --target Cellox")

-- Zipping binaries and creating installer using NSIS (nullsoft scriptable install system) 
-- (https://sourceforge.net/projects/nsis/) for Windows for x86 and x64

-- x64
print("Zipping Cellox x64 ...")
local zip_path = "../build/x64/src/"
if is_windows then
  if execute_command("if not exist " .. zip_path .. " (exit 1)") == "1" then
    print("Can not find x64 Build folder ...")
    os.exit(1)
  end
  execute_command("cd " .. zip_path)
  execute_command("cpack -G ZIP --config ../CPackConfig.cmake")
  print("Creating installer - x64 using NSIS ...")
  execute_command("cpack -G NSIS64 --config ../CPackConfig.cmake")
else
  execute_command("zip -r ../build/Cellox-x64.zip " .. zip_path)
end

-- x86
print("Zipping Cellox x86 ...")
zip_path = "../build/x86/src"
if execute_command("if not exist " .. zip_path .. " (exit 1)") == "1" then
  print("Can not find x86 Build folder ...")
  os.exit(1)
end
execute_command("cd " .. zip_path)
if is_windows then
  execute_command("cpack -G ZIP --config ../CPackConfig.cmake")
  print("Creating installer - x86 using NSIS ...")
  execute_command("cpack -G NSIS --config ../CPackConfig.cmake")
else
  execute_command("zip -r ../build/Cellox-x86.zip " .. zip_path)
end

execute_command("cd ../../..") -- go back to the starting directory
