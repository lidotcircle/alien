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
        return false
    end

    local info = ObjectHandleInfo_t:new()
    info.Inherit = false
    info.ProtectFromClose = false
    local status = ntdll.NtQueryObject(handle, 4, info(), 2)

    if (false and info.ProtectFromClose) then
        return 0xC0000235
    end

    if (status < 0 or status >= 0x80000000) then
        return false
    end

    return kernel32.raw.CloseHandle:horigin()(handle)
end)

user32.raw.MessageBoxA:hook(function (hwnd, text, caption, type)
    print(string.format("MessageBox(0x%x, %s, %s, 0x%x)", hwnd, text, caption, type))
    local om = user32.raw.MessageBoxA:horigin()
    return om(hwnd, text, caption, type)
end)
