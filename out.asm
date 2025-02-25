call main
main:
push r13
mov r13, sp
sub sp, sp, 2 ; x
mainWhile0:
load r1, [sp] ; x
cmp r1, 0
mov r1, 1
jg mainCmp0
mov r1, 0
mainCmp0:
sub r2, sp, 2
store [r2], r1
sub r1, sp, 2
load r1, [r1]
cmp r1, 0
je mainWhileEnd1
push r13
mov r13, sp
call in
add r1, sp, 4
store [r1], r1 ; x
add r1, sp, 4
load r1, [r1] ; x
call out
mov sp, r13
pop r13
jmp mainWhile1:
mainWhileEnd1:
mov sp, r13
pop r13
ret ; main
in:
in r1
ret

out:
out r1
ret
