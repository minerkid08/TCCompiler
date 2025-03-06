# compiler
echo "building compiler"
gcc -o comp -Ilib/src -Icompiler/src compiler/src/*.c compiler/src/parser/*.c compiler/src/codegen/*.c compiler/src/tokenizer/*.c lib/src/*.c

# linker
echo "building linker"
gcc -o link -Ilib/src -Ilinker/src linker/src/*.c lib/src/*.c

# interperter
echo "building interperter"
gcc -o interp -Ilib/src -Iinterperter/src interperter/src/*.c lib/src/*.c
