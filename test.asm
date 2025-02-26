main:
push r13
mov r13, sp
sub sp, sp, 2 ; x
mainWhile0:
load r1, [sp] ; x
cmp r1, 0
mov r1, 1
jg mainCmp1
mov r1, 0
mainCmp1:
cmp r1, 0
je mainWhileEnd0
push r13
mov r13, sp
call in
add r2, sp, 4
store [r2], r1 ; x
add r1, sp, 4
load r1, [r1] ; x
call out
mov sp, r13
pop r13
jmp mainWhile0:
mainWhileEnd0:
mov sp, r13
pop r13
ret ; main
