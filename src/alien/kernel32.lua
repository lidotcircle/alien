local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local util = require("alien.util")
local kernel32 = alien.load("kernel32")
local types = require("alien.win32types")
local abi   = require("alien.win32util").WINAPI

kernel32.GetTickCount:types{ret = types.dword, abi = abi}
function _M.GetTickCount(n) return kernel32.GetTickCount(n) end

kernel32.CloseHandle:types{ret = types.handle, abi = abi, types.boolean}
function _M.CloseHandle(handle) return kernel32.CloseHandle(handle) end

_M.Memory  = require("alien.kernel32.memory")
_M.Thread  = require("alien.kernel32.thread")
_M.Process = require("alien.kernel32.process")
util.TableMergeInto(_M, _M.Memory)
util.TableMergeInto(_M, _M.Thread)
util.TableMergeInto(_M, _M.Process)

-- for intellisense with language server
local A = _M
_M = A.Memory
_M = A.Thread
_M = A.Process
_M.raw = _M
_M = A

_M.raw = kernel32
return _M