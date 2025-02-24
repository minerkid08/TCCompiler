main:
push r13
mov r13, sp
sub sp, sp, 2 ; x
call in
store [sp], r1 ; x
load r1, [sp] ; x
call out
mov sp, r13
pop r13
ret ; main
