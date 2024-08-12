require("stdlib.lua")
require("math.lua")

println(-1)

local a = 4
local opr = 4
local b = 4
while 1 do
  cin(ref opr)
  local type = 0
  type(opr, ref type)
  if type == "string" then
    println("opr must be a number")
  else
    if opr == 0 then
      break
    end
    cin(ref a)
    cin(ref b)
    local out = 0
    math(a, b, opr, ref out)
    println(out)
  end
end

exit2()
