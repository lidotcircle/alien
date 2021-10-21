local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local kernel32 = alien.load("kernel32")
local types = require("alien.win32types")
local abi   = require("alien.win32util").WINAPI

kernel32.OpenThread:types{ret = types.handle, abi = abi, types.dword, types.boolean, types.dwrod}
function _M.OpenThread(dwDesiredAccess, bInheritHandle, threadId)
    return kernel32.OpenThread(dwDesiredAccess, bInheritHandle, threadId)
end

return _M