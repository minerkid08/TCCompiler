main:
push r13
mov r13, sp
sub sp, sp, 2 ; numb
call in
store [sp], r1 ; numb
sub sp, sp, 2 ; src
call in
store [sp], r1 ; src
sub sp, sp, 2 ; dest
call in
store [sp], r1 ; dest
sub sp, sp, 2 ; spare
call in
store [sp], r1 ; spare
add r1, sp, 6
load r1, [r1] ; numb
add r2, sp, 4
load r2, [r2] ; src
add r3, sp, 2
load r3, [r3] ; dest
mov r4, r1 ; spare
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
load r1, [r1] ; numb
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
load r1, [r1] ; src
call out
mov r1, 5
call out
add r1, sp, 4
load r1, [r1] ; dest
call out
mov r1, 5
call out
mov sp, r13
pop r13
mov sp, r13
pop r13
mov sp, r13
pop r13
ret
mov sp, r13
pop r13
moveIf0:
add r1, sp, 6
load r1, [r1] ; numb
sub r1, r1, 1
add r2, sp, 4
load r2, [r2] ; src
load r3, [sp] ; spare
add r4, sp, 2
load r4, [r4] ; dest
call move
add r1, sp, 4
load r1, [r1] ; src
call out
mov r1, 5
call out
add r1, sp, 2
load r1, [r1] ; dest
call out
mov r1, 5
call out
add r1, sp, 6
load r1, [r1] ; numb
sub r1, r1, 1
load r2, [sp] ; spare
add r3, sp, 2
load r3, [r3] ; dest
add r4, sp, 4
load r4, [r4] ; src
call move
mov sp, r13
pop r13
ret ; move
