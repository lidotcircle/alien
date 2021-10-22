local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local user32 = alien.load("user32")
local abi    = require("alien.win32util").WINAPI
local types  = require("alien.win32types")

user32.MessageBoxA:types{ret = types.int, abi = abi, types.handle, types.string, types.string, types.int}
function _M.MessageBox(hwnd, text, caption, type)
    return user32.MessageBoxA(hwnd, text, caption, type)
end

_M.raw = user32
return _M