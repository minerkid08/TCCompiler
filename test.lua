require("stdlib.lua");

function main()
  local x;
  in() -> x;
  while(1) do
    x = x >> 7 ^ x;
    x = x << 9 ^ x;
    x = x >> 8 ^ x;
    out(x);
  end
end
