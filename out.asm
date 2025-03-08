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
store [sp, 6], r1 ; numb
call in
store [sp, 4], r1 ; src
call in
store [sp, 2], r1 ; dest
call in
store [sp], r1 ; spare
load r1, [sp, 6] ; numb
load r2, [sp, 4] ; src
load r3, [sp, 2] ; dest
load r4, [sp] ; spare
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
load r1, [sp, 6] ; numb
cmp r1, 0
mov r1, 1
je moveCmp0
mov r1, 0
moveCmp0:
cmp r1, 0
je moveIf0
push r13
mov r13, sp
load r1, [sp, 6] ; src
load r2, [sp, 4] ; dest
call moveDisk
mov sp, r13
pop r13
jmp moveElse0
moveIf0:
push r13
mov r13, sp
load r1, [sp, 8] ; numb
sub r1, r1, 1
load r2, [sp, 6] ; src
load r3, [sp, 2] ; spare
load r4, [sp, 4] ; dest
call move
load r1, [sp, 6] ; src
load r2, [sp, 4] ; dest
call moveDisk
load r1, [sp, 8] ; numb
sub r1, r1, 1
load r2, [sp, 2] ; spare
load r3, [sp, 4] ; dest
load r4, [sp, 6] ; src
call move
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

