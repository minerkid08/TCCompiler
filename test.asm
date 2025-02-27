main:
push r13
mov r13, sp
sub sp, sp, 2 ; x
call in
store [sp], r1 ; x
mainWhile0:
mov r1, 1
cmp r1, 0
je mainWhileEnd0
push r13
mov r13, sp
add r1, sp, 2
load r1, [r1] ; x
lsr r1, r1, 7
sub r2, sp, 2
store [r2], r1 ; t1
sub r1, sp, 2
load r1, [r1] ; t1
push r1 ; a
add r1, sp, 2
load r1, [r1] ; a
add r2, sp, 4
load r2, [r2] ; x
xor r1, r1, r2
sub r2, sp, 2
store [r2], r1 ; t1
sub r1, sp, 2
load r1, [r1] ; t1
push r1 ; b
add r1, sp, 2
load r1, [r1] ; a
call out
mov sp, r13
pop r13
jmp mainWhile0:
mainWhileEnd0:
mov sp, r13
pop r13
ret ; main
