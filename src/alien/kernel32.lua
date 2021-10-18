local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local kernel32 = alien.load("kernel32")

kernel32.GetTickCount:types("int")
function _M.GetTickCount(n) return kernel32.GetTickCount(n) end

return _M