local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local ntdll = alien.load("ntdll")

ntdll.NtWriteVirtualMemory:types(
    {ret = alien.types.int},
    alien.types.int, alien.types.pointer, alien.types.pointer, alien.types.ref_int)
function _M.NtWriteVirtualMemory(handle, baseAddress, buffer, bufsize)
    return ntdll.NtWriteVirtualMemory(handle, baseAddress, buffer, bufsize)
end

return _M
