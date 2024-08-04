require("stdlib.lua")

function math(a, b, opr, out)
  if opr == 1 then
    out = a + b
    return
  end
  if opr == 2 then
    out = a - b
    return
  end
  if opr == 3 then
    out = a * b
    return
  end
  if opr == 4 then
    out = a / b
    return
  end
  println("invalid opration: " + opr);
end
