-- Builds the Cellox project using CMake and Ninja

-- Find vcvars64.bat
local function find_vcvars64_bat()
  local search_paths = { os.getenv("ProgramFiles(x86)"), os.getenv("ProgramFiles") }
  local versions = {"2022" , "2019", "2017", "2015"}
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

-- Check if we are running on Windows
local is_windows = package.config:sub(1,1) == '\\'

-- Run vcvars64.bat if we are on Windows
if is_windows then  
  vcvars_path = find_vcvars64_bat()
  if vcvars_path then
      print("Found vcvars64.bat at " .. vcvars64_path)
  else
      print("Could not find vcvars64.bat")
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
local buildTarget = arg[2] or "all"
-- The default generator is Ninja if it is available - otherwise we let cmake decide
local generator = arg[3] or (ninjaAvailable() and "Ninja" or "")

-- We should make the compiler that is used for building the project configurable as well, for now we just use the default
local cmake_command = string.format("cmake -B ../build -DCMAKE_BUILD_TYPE=%s%s ..", buildType, generator and (" -G " .. generator) or "")
-- This command is only needed on Windows
local vcvars_command = is_windows and string.format('"%s" && ', vcvars_path) or ""
os.execute(vcvars_command .. cmake_command .. (string.format(" && cmake --build ../build --config %s --target %s", buildType, buildTarget) or ""))

