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
assert(alien.types.uint8_t:value_is(0x11))
assert(not alien.types.uint8_t:value_is(nil))
assert(not alien.types.uint8_t:value_is(false))
assert(not alien.types.uint8_t:value_is(""))

-- string
local s1 = alien.types.string:new("hello world")
local s2 = alien.types.string:new()
assert(s1 == "hello world")
assert(s2 == "")
assert(alien.types.string:value_is(0))
assert(not alien.types.string:value_is(nil))
assert(not alien.types.string:value_is(false))
assert(alien.types.string:value_is(s1))
assert(alien.types.string:value_is("nihao"))

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

-- string test2
local helloworld = testl.helloworld
helloworld:types({ret = alien.types.string})
local hw = helloworld()
assert(hw == "hello world")

-- buffer test2
local strlenint = testl.strlen_int
local strlensize_t = testl.strlen_size_t
strlenint:types({ret = alien.types.int, alien.types.string})
strlensize_t:types({ret = alien.types.size_t, alien.types.string})
local strl1 = strlenint("hello world")
local strl2 = strlensize_t("hello world")
assert(strl1 == 11)
assert(strl2 == 11)
local buf1 = alien.types.buffer:new(10)
buf1:uint8_set(1, 0x31)
buf1:uint8_set(2, 0x31)
buf1:uint8_set(3, 0x00)
local strl3 = strlenint(buf1)
assert(strl3 == 2)

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

local abcnew = testl.struct1_new
abcnew:types({ret = alien.types.abc, alien.types.int, alien.types.int})
local f3 = abcnew(100, 200)
assert(f3.a == 100)
assert(f3.b == 200)


-- union
alien.defunion("union1", {
    { "a", alien.types.int },
    { "b", alien.types.int },
})
local uv1 = alien.types.union1:new()
uv1.a = 100
assert(uv1.b == 100)

-- callback
local cb1 = alien.callback(function(a, b) return a * b end, {
    ret = alien.types.int,
    alien.types.int,
    alien.types.int
})
assert(type(cb1) == "userdata")
local addtwo = testl.add2ints
addtwo:types({ret = alien.types.int, alien.types.int, alien.types.int})
assert(addtwo.trampoline == nil)
addtwo:hook(cb1)
local at1 = addtwo(100, 200)
local at2 = addtwo.trampoline(100, 200)
addtwo:unhook()
local at3 = addtwo(100, 200)
assert(at1 == 20000)
assert(at2 == 300)
assert(at3 == 300)
local do2and4 = testl.do2and4
do2and4:types{ret = alien.types.int, alien.types.callback}
local dv1 = do2and4(cb1)
assert(dv1 == 8)
