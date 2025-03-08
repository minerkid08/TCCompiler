require("stdlib.lua");

function moveDisk(src, dest) end

function move(numb, src, dest, spare) end

function main()
  local numb;
  local src;
  local dest;
  local spare;

  in() -> numb;
  in() -> src;
  in() -> dest;
  in() -> spare;

  move(numb, src, dest, spare);
end

function move(numb, src, dest, spare)
  if(numb == 0) then
    moveDisk(src, dest);
  else
    move(numb - 1, src, spare, dest);
    moveDisk(src, dest);
    move(numb - 1, spare, dest, src);
  end
end
