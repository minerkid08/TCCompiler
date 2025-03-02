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
load r1, [sp] ; x
lsr r1, r1, 7
load r2, [sp] ; x
xor r1, r1, r2
store [sp], r1 ; x
lsl r1, r1, 9
load r2, [sp] ; x
xor r1, r1, r2
store [sp], r1 ; x
lsr r1, r1, 8
load r2, [sp] ; x
xor r1, r1, r2
store [sp], r1 ; x
call out
mov sp, r13
pop r13
jmp mainWhile0
mainWhileEnd0:
mov sp, r13
pop r13
ret ; main
