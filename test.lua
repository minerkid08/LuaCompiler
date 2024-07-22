function dothing(text) end
function println(text) end
local a = 4;
if a == 4 then
  local b = 5;
  a = a + b;
  dothing(4);
  println(b);
  println(a);
end
println(a);

function dothing(text)
  println(text + 8);
end
