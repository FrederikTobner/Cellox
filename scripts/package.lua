-- Packages the project as a zip file for Windows and Linux for x86 and x64
-- and creates an installer for Windows using NSIS (nullsoft scriptable install system)

-- determine if the script is running on Windows
local is_windows = package.config:sub(1,1) == '\\'

-- Find vcvars64.bat
local function find_vcvars64_bat()
  local search_paths = { os.getenv("ProgramFiles(x86)"), os.getenv("ProgramFiles") }
  local versions = {"2015", "2017", "2019", "2022" }
  local editions = { "Community", "Enterprise", "Professional" }
  for _, path in ipairs(search_paths) do
      for _, version in ipairs(versions) do
          for _, edition in ipairs(editions) do
              local vcvars64_path = string.format("%s\\Microsoft Visual Studio\\%s\\%s\\VC\\Auxiliary\\Build\\vcvars64.bat", path, version, edition)
              for file in io.popen('dir "' .. vcvars64_path .. '" /B /A:-D 2> nul'):lines() do
                  return vcvars64_path:gsub("%*", file)
              end
          end
      end
  end
  return nil
end

-- Find vcvars32.bat
local function find_vcvars32_bat()
  local search_paths = { os.getenv("ProgramFiles(x86)"), os.getenv("ProgramFiles") }
  local versions = {"2015", "2017", "2019", "2022" }
  local editions = { "Community", "Enterprise", "Professional" }
  for _, path in ipairs(search_paths) do
      for _, version in ipairs(versions) do
          for _, edition in ipairs(editions) do
              local vcvars64_path = string.format("%s\\Microsoft Visual Studio\\%s\\%s\\VC\\Auxiliary\\Build\\vcvars32.bat", path, version, edition)
              for file in io.popen('dir "' .. vcvars64_path .. '" /B /A:-D 2> nul'):lines() do
                  return vcvars64_path:gsub("%*", file)
              end
          end
      end
  end
  return nil
end

-- Check if we are running on Windows
local is_windows = package.config:sub(1,1) == '\\'

-- Run vcvars64.bat if we are on Windows
if is_windows then  
  vcvars64_path = find_vcvars64_bat()
  if vcvars64_path then
      print("Found vcvars64.bat at " .. vcvars64_path)
  else
      print("Could not find vcvars64.bat")
      os.exit(1)
  end
end

-- Run vcvars32.bat if we are on Windows
if is_windows then  
  vcvars32_path = find_vcvars32_bat()
  if vcvars32_path then
      print("Found vcvars32.bat at " .. vcvars32_path)
  else
      print("Could not find vcvars32.bat")
      os.exit(1)
  end
end


function ninjaAvailable()
  local handle = io.popen("ninja --version")
  local result = handle:read("*a")
  handle:close()
  return string.match(result, "[%d.]+[%d]")
end

-- The default build type is Release
local buildType = arg[1] or "Release"
-- The default build target is all
local buildTarget = arg[2] or "Cellox"
-- The default generator is Ninja if it is available - otherwise we let cmake decide
local generator = arg[3] or (ninjaAvailable() and "Ninja" or "")

-- We should make the compiler that is used for building the project configurable as well, for now we just use the default
local cmake_command = string.format("cmake -B ../build/x64 -DCMAKE_BUILD_TYPE=%s%s ..", buildType, generator and (" -G " .. generator) or "")
-- This command is only needed on Windows
local vcvars64_command = is_windows and string.format('"%s" && ', vcvars64_path) or ""
os.execute(vcvars64_command .. cmake_command .. (string.format(" && cmake --build ../build/x64 --config %s --target %s", buildType, buildTarget) or ""))

-- We should make the compiler that is used for building the project configurable as well, for now we just use the default
local cmake_command = string.format("cmake -B ../build/x86 -DCMAKE_BUILD_TYPE=%s%s ..", buildType, generator and (" -G " .. generator) or "")
-- This command is only needed on Windows
local vcvars32_command = is_windows and string.format('"%s" && ', vcvars32_path) or ""
os.execute(vcvars32_command .. cmake_command .. (string.format(" && cmake --build ../build/x86 --config %s --target %s", buildType, buildTarget) or ""))

-- Zipping binaries and creating installer using NSIS (nullsoft scriptable install system) 
-- (https://sourceforge.net/projects/nsis/) for Windows for x86 and x64

-- x64
print("Zipping Cellox x64 ...")
local zip_path = "../build/x64/src/"
if is_windows then
  os.execute("cd " .. zip_path .. "&& cpack -G ZIP --config ../CPackConfig.cmake && cpack -G NSIS64 --config ../CPackConfig.cmake")
else
  os.execute("cd " .. zip_path .. " && zip -r ../build/Cellox-x64.zip " .. zip_path)
end

-- x86
print("Zipping Cellox x86 ...")
zip_path = "../build/x86/src"
if is_windows then
  os.execute("cd " .. zip_path .. "&& cpack -G ZIP --config ../CPackConfig.cmake && cpack -G NSIS --config ../CPackConfig.cmake")
else
  os.execute("cd " .. zip_path .. " && zip -r ../build/Cellox-x86.zip " .. zip_path)
end
