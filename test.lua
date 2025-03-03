require("stdlib.lua");

function move(numb, src, dest, spare) end

function main()
  local numb;
  in() -> numb;
  local src;
  in() -> src;
  local dest;
  in() -> dest;
  local spare;
  in() -> spare;

  move(numb, src, dest, spare);
end

function move(numb, src, dest, spare)
  if(numb == 0) then
    out(src);
    out(5);
    out(dest);
    out(5);
    return;
  end
  move(numb - 1, src, spare, dest);
  out(src);
  out(5);
  out(dest);
  out(5);

  move(numb - 1, spare, dest, src);
end
