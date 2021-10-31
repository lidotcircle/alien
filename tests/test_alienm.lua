local alien = require("alien")
local testl = alien.load("alientest")

local m = alien.functionlist(testl);
for _,n in ipairs(m) do
    assert(alien.hasfunction(testl, n))
end

assert(type(alien) == "table")
assert(type(alien.version) == "string")
assert(type(alien.platform) == "string")
assert(type(alien.alias) == "function")

assert(type(alien.types) == "table")
for k,v in pairs(alien.types) do
    assert(type(k) == "string")
    assert(type(v) == "userdata")
end

local n1 = alien.types.uint8_t:new(100)
local n2 = alien.types.uint8_t:new(0x11ff)
assert(n1 == 100)
assert(n2 == 0xff)

