local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local kernel32 = alien.load("kernel32")
local types = require("alien.win32types")
local abi   = require("alien.win32util").WINAPI

---@class ProcessAccessRight

---@type table<string,ProcessAccessRight>
_M.ProcessAccessRight = {
    PROCESS_CREATE_PROCESS = 0x0080;
    PROCESS_CREATE_THREAD = 0x0002;
    PROCESS_DUP_HANDLE = 0x0040;
    PROCESS_QUERY_INFORMATION = 0x0400;
    PROCESS_QUERY_LIMITED_INFORMATION = 0x1000;
    PROCESS_SET_INFORMATION = 0x0200;
    PROCESS_SET_QUOTA = 0x0100;
    PROCESS_SUSPEND_RESUME = 0x0800;
    PROCESS_TERMINATE = 0x0001;
    PROCESS_VM_OPERATION = 0x0008;
    PROCESS_VM_READ = 0x0010;
    PROCESS_VM_WRITE = 0x0020;
    SYNCHRONIZE = 0x00100000;
}

kernel32.OpenProcess:types{ret = types.handle, abi = abi, types.dword, types.boolean, types.dword}
---@param dwDesiredAccess ProcessAccessRight
---@return HANDLE
function _M.OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId)
    return kernel32.OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId)
end

kernel32.GetProcessId:types{ret = types.dword, abi = abi, types.handle}
---@return number
function _M.GetProcessId(handle) return kernel32.GetProcessId(handle) end

kernel32.GetCurrentProcessId:types{ret = types.dword, abi = abi}
---@return number
function _M.GetCurrentProcessId() return kernel32.GetCurrentProcessId() end

return _M