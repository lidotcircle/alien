local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local kernel32 = alien.load("kernel32")
local types = require("alien.win32types")
local abi   = require("alien.win32util").WINAPI


---@class AllocationType

---@type table<string, AllocationType>
_M.AllocationType = {
    MEM_COMMIT     = 0x00001000;
    MEM_RESERVE    = 0x00002000;
    MEM_RESET      = 0x00080000;
    MEM_RESET_UNDO = 0x01000000;

    MEM_LARGE_PAGES = 0x20000000;
    MEM_PHYSICAL    = 0x00400000;
    MEM_TOP_DOWN    = 0x00100000;
}


---@class MemProtect

---@type table<string, MemProtect>
_M.Protect = {
    PAGE_EXECUTE = 0x10;
    PAGE_EXECUTE_READ = 0x20;
    PAGE_EXECUTE_READWRITE = 0x40;
    PAGE_EXECUTE_WRITECOPY = 0x80;
    PAGE_NOACCESS = 0x01;
    PAGE_READONLY = 0x02;
    PAGE_READWRITE = 0x04;
    PAGE_WRITECOPY = 0x08;
    PAGE_TARGETS_INVALID   = 0x40000000;
    PAGE_TARGETS_NO_UPDATE = 0x40000000;

    PAGE_GUARD = 0x100;
    PAGE_NOCACHE = 0x200;
    PAGE_WRITECOMBINE = 0x400;
}


kernel32.VirtualAllocEx:types({ret = types.pointer, abi = abi, types.handle, types.pointer, types.size_t, types.dword, types.dword})
---@param flAllocationType AllocationType
---@param flMemProtect MemProtect
---@return HANDLE
function _M.VirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flMemProtect)
    return kernel32.VirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flMemProtect)
end

kernel32.VirtualAlloc:types({ret = types.pointer, types.pointer, types.size_t, types.dword, types.dword})
---@param flAllocationType AllocationType
---@param flMemProtect MemProtect
---@return HANDLE
function _M.VirtualAlloc(lpAddress, dwSize, flAllocationType, flMemProtect)
    return kernel32.VirtualAlloc(lpAddress, dwSize, flAllocationType, flMemProtect)
end


---@class FreeType

---@type table<string, FreeType>
_M.FreeType = {
    MEM_DECOMMIT = 0x00004000;
    MEM_RELEASE  = 0x00008000;
    MEM_COALESCE_PLACEHOLDERS = 0x00000001;
    MEM_PRESERVE_PLACEHOLDER = 0x00000002;
}

kernel32.VirtualFreeEx:types({ret = types.boolean, types.handle, types.pointer, types.size_t, types.dword})
---@param dwFreeType FreeType
---@return boolean
function _M.VirtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType) return kernel32.VirtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType) end

kernel32.VirtualFree:types({ret = types.boolean, types.pointer, types.size_t, types.dword})
---@param dwFreeType FreeType
---@return boolean
function _M.VirtualFree(lpAddress, dwSize, dwFreeType) return kernel32.VirtualFree(lpAddress, dwSize, dwFreeType) end

kernel32.VirtualProtectEx:types({ret = types.boolean, types.handle, types.pvoid, types.size_t, types.dword, types.ref_dword})
---@return boolean success
---@return MemProtect oldProtect
function _M.VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect)
    return kernel32.VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect)
end

kernel32.VirtualProtect:types({ret = types.boolean, types.pvoid, types.size_t, types.dword, types.ref_dword})
---@return boolean success
---@return MemProtect oldProtect
function _M.VirtualProtect(lpAddress, dwSize, flNewProtect)
    return kernel32.VirtualProtect(lpAddress, dwSize, flNewProtect)
end

return _M