local alien = require("alien")

if alien.platform ~= "windows" then
    return
end

local kernel32 = alien.load("kernel32")
for _,n in ipairs(alien.functionlist(kernel32)) do
assert(alien.hasfunction(kernel32, n))
end

local GetTickCount = kernel32.GetTickCount
GetTickCount:types("int")
local count = GetTickCount()
assert(count > 0)

GetTickCount:hook(function () return 0 end)
count = GetTickCount()
assert(count == 0)

count = GetTickCount:trampoline()()
assert(count > 0)

GetTickCount:unhook()
count = GetTickCount()
assert(count > 0)
