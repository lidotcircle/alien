local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local ntdll = alien.load("ntdll")
local types = require("alien.win32types")
local abi   = require("alien.win32util").NTAPI

ntdll.NtWriteVirtualMemory:types(
    {ret = alien.types.int},
    alien.types.int, alien.types.pointer, alien.types.pointer, alien.types.ref_int)
function _M.NtWriteVirtualMemory(handle, baseAddress, buffer, bufsize)
    return ntdll.NtWriteVirtualMemory(handle, baseAddress, buffer, bufsize)
end

ntdll.NtClose:types{ret = types.ntstatus, abi = abi, types.handle}
function _M.NtClose(handle) return ntdll.NtClose(handle) end

ntdll.NtQueryObject:types{ret = types.ntstatus, abi = abi, types.handle, types.int, types.pointer, types.size_t, types.ref_int}
function _M.NtQueryObject(handle, info_class, buffer, size) return ntdll.NtQueryObject(handle, info_class, buffer, size, 0) end

function _M.NtStatusOk(status) return status >= 0 and status < 0x8000000 end

_M.raw = ntdll;
return _M
