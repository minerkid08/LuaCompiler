require("stdlib.lua")
require("header.lua")

local a = 4
if a == 4 then
	local b = 5
	a = a + b
	dothing(4)
	println(b)
	println(a)
end
println(a)
dothing();
exit2()
