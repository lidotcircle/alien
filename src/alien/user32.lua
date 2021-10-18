local alien = require("alien")

local _M = {}
if (alien.platform ~= "windows") then
    return _M
end

local user32 = alien.load("user32")

user32.MessageBoxA:types({ret = alien.types.int, alien.types.int, alien.types.string, alien.types.string, alien.types.int})
function _M.MessageBox(hwnd, text, caption, type)
    return user32.MessageBoxA(hwnd, text, caption, type)
end

return _M