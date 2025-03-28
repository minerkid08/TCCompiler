call main
exit

main:
push r13
mov r13, sp
sub sp, sp, 2 ; numb
sub sp, sp, 2 ; src
sub sp, sp, 2 ; dest
sub sp, sp, 2 ; spare
call in
add r2, sp, 6
store_16 [r2], r1 ; numb
call in
add r2, sp, 4
store_16 [r2], r1 ; src
call in
add r2, sp, 2
store_16 [r2], r1 ; dest
call in
store_16 [sp], r1 ; spare
add r1, sp, 6
load_16 r1, [r1] ; numb
add r2, sp, 4
load_16 r2, [r2] ; src
add r3, sp, 2
load_16 r3, [r3] ; dest
load_16 r4, [sp] ; spare
call move
mov sp, r13
pop r13
ret ; main

move:
push r13
mov r13, sp
push r1 ; numb
push r2 ; src
push r3 ; dest
push r4 ; spare
add r1, sp, 6
load_16 r1, [r1] ; numb
cmp r1, 0
mov r1, 1
je moveCmp0
mov r1, 0
moveCmp0:
cmp r1, 0
je moveIf0
push r13
mov r13, sp
add r1, sp, 6
load_16 r1, [r1] ; src
add r2, sp, 4
load_16 r2, [r2] ; dest
call moveDisk
mov sp, r13
pop r13
jmp moveElse0
moveIf0:
push r13
mov r13, sp
add r1, sp, 8
load_16 r1, [r1] ; numb
sub r1, r1, 1
add r2, sp, 6
load_16 r2, [r2] ; src
add r3, sp, 2
load_16 r3, [r3] ; spare
add r4, sp, 4
load_16 r4, [r4] ; dest
call move
add r1, sp, 6
load_16 r1, [r1] ; src
add r2, sp, 4
load_16 r2, [r2] ; dest
call moveDisk
add r1, sp, 8
load_16 r1, [r1] ; numb
sub r1, r1, 1
add r2, sp, 2
load_16 r2, [r2] ; spare
add r3, sp, 4
load_16 r3, [r3] ; dest
add r4, sp, 6
load_16 r4, [r4] ; src
call move
mov sp, r13
pop r13
moveElse0:
mov sp, r13
pop r13
ret ; move


in:
in r1
ret

out:
out r1
ret

moveDisk:
out r1
out 5
out r2
out 5
ret

