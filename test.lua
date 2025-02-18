inline asm function out(value)
  out $value
end

asm function doThing(a, b, c)
  add r4, $a, $b
  lsl r4, r4, $c
  out r4
end

function fun(a, b)
  doThing(a, b, 2);
  out(a);
end

function main()
  out(4);
  local e = 4;
  local f = 5;
  local g = 6;
  doThing(g, f, e);
end
