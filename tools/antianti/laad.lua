local kernel32 = require("alien.kernel32")


kernel32.raw.CloseHandle:hook(function (handle)
    return kernel32.raw.CloseHandle:horigin()(handle)
end)
