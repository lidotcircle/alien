local _M = require("alien_c")

local alien = _M
local error = error
local io = io


local load = alien.load
_M.loaded = {}
local load_library, find_library = {}, {}

local function find_library_helper(libname, opt)
  local expr = '/[^() ]*lib' .. libname .. '%.so[^() ]*'
  local cmd = '/sbin/ldconfig ' .. opt ..
    ' 2>/dev/null | egrep -o "' .. expr .. '"'
  local pipe = io.popen(cmd)
  if pipe then
    local res = pipe:read()
    pipe:close()
    return res and res:match("([^%s]*)")
  end
  return nil
end

function find_library.linux(libname)
  return find_library_helper(libname, "-p")
end

function find_library.bsd(libname)
  return find_library_helper(libname, "-r")
end

function find_library.darwin(libname)
  local ok, lib = pcall(load, libname .. ".dylib")
  if ok then return lib end
  ok, lib = pcall(load, libname .. ".framework/" .. libname)
  if ok then return lib end
  return nil
end

local function load_library_helper(libname, libext)
  if libname:match("/") or libname:match("%" .. libext) then
    return load(libname)
  else
    local ok, lib = pcall(load, "lib" .. libname .. libext)
    if not ok then
      ok, lib = pcall(load, "./lib" .. libname .. libext)
      if not ok then
        local name = find_library[alien.platform](libname)
        if name then
          lib = load(name)
        else
          return nil, "library " .. libname .. " not found"
        end
      end
    end
    return lib
  end
end

function load_library.linux(libname)
  local lib, errmsg = load_library_helper(libname, ".so")
  if not lib then error (errmsg) end
  return lib
end

load_library.bsd = load_library.linux

function load_library.darwin(libname)
  local lib, errmsg = load_library_helper(libname, ".dylib")
  if not lib then
    lib, errmsg = load_library_helper(libname, ".so")
  end
  if not lib then error (errmsg) end
  return lib
end

function load_library.windows(libname)
  return load(libname)
end

setmetatable(_M.loaded, { __index = function (t, libname)
                                      local lib = load_library[alien.platform](libname)
                                      t[libname] = lib
                                      return lib
                                    end, __mode = "kv" })

function _M.load(libname)
  return _M.loaded[libname]
end

return _M
