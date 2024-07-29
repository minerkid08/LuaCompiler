require("stdlib.lua")

local a = 4
local opr = 4
local b = 4
while(a != 5) do
  cin(ref a);
  cin(ref opr);
  cin(ref b);
  if(opr == 1) then
    println(a + b);
  end
  if(opr == 2) then
   println(a - b);
  end
  if(opr == 3) then
    println(a * b);
  end
  if(opr == 4) then
    println(a / b);
  end
end

exit2()
