require("stdlib.lua")

local count = 0
cin(ref count)
local i = 0
local p1 = 1
local p2 = 0
while i != count do
  i = i + 1
  local num = p1 + p2
  println(num)
  p2 = p1
  p1 = num
end

exit2()
