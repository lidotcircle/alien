local ntdll = require("alien.ntdll")
local alien = require("alien")
local user32 = require("alien.user32")
local types  = require("alien.win32types")
local kernel32 = require("alien.kernel32")


local ObjectHandleInfo_t = alien.defstruct{
    { "Inherit",          types.char},
    { "ProtectFromClose", types.char}
}


kernel32.raw.CloseHandle:hook(function (handle)
    if (handle == -1) then
        print("close -1 handle")
        return false
    end

    print(string.format("CloseHandle(0x%x)", handle))
    local info = ObjectHandleInfo_t:new()
    info.Inherit = false
    info.ProtectFromClose = false
    local status = ntdll.NtQueryObject(handle, 4, info(), 2)

    if (false and info.ProtectFromClose) then
        print("    protect")
        return 0xC0000235
    end

    if (status < 0 or status >= 0x80000000) then
        print(string.format("    => false    bad handle"))
        return false
    end

    local ans = ntdll.NtClose(handle)
    print(string.format("    => %s", ntdll.NtStatusOk(ans)))
    return ntdll.NtStatusOk(ans)
    -- print(string.format("CloseHandle(0x%x)", handle))
    -- return kernel32.raw.CloseHandle:horigin()(handle)
end)
print(string.format("CloseHandle: 0x%x", kernel32.raw.CloseHandle:addr()))
print(string.format("Trampoline CloseHandle: 0x%x", kernel32.raw.CloseHandle:horigin():addr()))
kernel32.raw.CloseHandle:unhook()

user32.raw.MessageBoxA:hook(function (hwnd, text, caption, type)
    print(string.format("MessageBox(0x%x, %s, %s, 0x%x)", hwnd, text, caption, type))
    return user32.raw.MessageBoxA:horigin()(hwnd, text, caption, type)
end)

print(string.format("MessageBoxA: 0x%x", user32.raw.MessageBoxA:addr()))
print(string.format("Trampoline MessageBoxA: 0x%x", user32.raw.MessageBoxA:horigin():addr()))
