require("stdlib.lua")
require("math.lua")

local a = 4
local opr = 4
local b = 4
local s = "opr is "
while 1 do
  cin(ref opr);
  if opr == 0 then
    break
  end
  println(s + opr)
  cin(ref a);
  cin(ref b);
  local out = 0;
  math(a, b, opr, ref out);
  println(out);
end

exit2()
