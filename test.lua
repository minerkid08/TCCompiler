require("stdlib.lua");

function main()
  local x;
  in() -> x;
  while(1) do
    local a = x >> 7;
    local b = a ^ x;
    a = b << 9;
    b = a ^ b;
    a = b >> 8;
    b = a ^ b;
    out(a);
  end
end
