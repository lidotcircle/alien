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

-- integer
local n1 = alien.types.uint8_t:new(100)
local n2 = alien.types.uint8_t:new(0x11ff)
local n3 = alien.types.uint8_t:new(-1)
local n4 = alien.types.uint16_t:new(-1)
assert(n1 == 100)
assert(n2 == 0xff)
assert(n3 == 0xff)
assert(n4 == 0xffff)

-- string
local s1 = alien.types.string:new("hello world")
local s2 = alien.types.string:new()
assert(s1 == "hello world")
assert(s2 == "")

-- buffer
local b1 = alien.types.buffer:new(1000)
b1:uint8_set(1, 0x33)
b1:uint8_set(2, 0x44)
assert(b1:uint8_get(1) == 0x33)
assert(b1:uint8_get(2) == 0x44)
assert(b1:uint16_get(1) == 0x4433)
assert(#b1 == 1000)

-- function test1
local get2times = testl.get2times
get2times:types({ret = alien.types.int, alien.types.int})
local n2t = get2times(100)
assert(n2t == 200)
local printstr = testl.print_string
printstr:types({ret = alien.types.void, alien.types.string})
printstr("hello world")

-- struct
alien.defstruct("abc", {
    { "a", alien.types.int },
    { "b", alien.types.int },
})
alien.defstruct("uzv", {
    { "x", alien.types.abc },
    { "y", alien.types.char },
})
local f = alien.types.abc:new()
assert(type(f) == "userdata")
f.a = 100
f.b = 200
assert(f.a == 100)
assert(f.b == 200)
local uzv1 = alien.types.uzv:new()
uzv1.x = f
assert(uzv1.x.a == 100)
local ux1 = uzv1.x
ux1.a = 500
assert(uzv1.x.a == 500)

local sumabc = testl.sumstruct1
sumabc:types({ret = alien.types.int, alien.types.abc})
local sabc = sumabc(f)
assert(sabc == 300)
assert(sabc == f.a + f.b)

