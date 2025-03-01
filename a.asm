main:
push r13
mov r13, sp
sub sp, sp, 2 ; b
add r1, sp, 2
load r1, [r1] ; b
add r1, r1, 2
add r2, sp, 2
store [r2], r1 ; t1
sub r1, r1, 3
add r2, sp, 4
store [r2], r1 ; t2
push r1 ; a
mov sp, r13
pop r13
ret ; main
