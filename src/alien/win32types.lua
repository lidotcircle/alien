local alien = require("alien")

if (alien.platform ~= "windows") then
    return {}
end

local types = alien.types
types.handle    = types.int
types.pvoid     = types.pointer
types.dword     = types.int
types.boolean   = types.int
types.ref_dword = types.ref_int
types.ntstatus  = types.int

---@class HANDLE
---@class PVOID

return types