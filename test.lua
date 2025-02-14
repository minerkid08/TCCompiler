inline asm function out(value)
  out $value
end

asm function doThing(a, b, c)
  add r4, $a, $b
  shl r4, r4, $c
  out r4
end


function main()
  out(4);
  local e = 7;
  doThing(4, e, 6);
end
