local alien = require("alien")

if alien.platform ~= "windows" then
    return
end

local kernel32 = alien.load("kernel32")
for _,n in ipairs(alien.functionlist(kernel32)) do
assert(alien.hasfunction(kernel32, n))
end

local types = require("alien.win32types")
local winapi = require("alien.win32util").WINAPI

local user32 = alien.load("user32")
local MessageBox = user32.MessageBoxA
MessageBox:types{ret = types.int, abi = winapi, types.handle, types.string, types.string, types.int}
MessageBox:hook(function (hwnd, text, caption, type)
    print(string.format("MessageBox(0x%x, \"%s\", \"%s\", 0x%x)", hwnd, text, caption, type))
    MessageBox:horigin()(hwnd, text, caption, type)
end)

local GetTickCount = kernel32.GetTickCount
GetTickCount:types({ret = "int", abi = winapi})
local count = GetTickCount()
assert(count > 0)

GetTickCount:hook(function () return 0 end)
count = GetTickCount()
assert(count == 0)

count = GetTickCount:horigin()()
assert(count > 0)

GetTickCount:unhook()
count = GetTickCount()
assert(count > 0)

print("END")
MessageBox(0, "what", "are", 0)