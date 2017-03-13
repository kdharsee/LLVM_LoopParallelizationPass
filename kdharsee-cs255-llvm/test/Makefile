#===- runtime/Makefile -------------------------*- Makefile -*-===##

# Relative path to the top of the source tree.
#
LEVEL=..

#LLVM_BIN_DIR=/u/cs255/build-llvm-38/bin
CLANG_BIN_DIR=/u/cs255/build-llvm-38/bin
LLVM_BIN_DIR=/u/cs255/dc_llvm/build/bin

OPT= $(LLVM_BIN_DIR)/opt
LLC= $(LLVM_BIN_DIR)/llc
CC= $(CLANG_BIN_DIR)/clang

check: test.c test2.c
	$(CC) -O0 -c -emit-llvm test.c -o test.bc
	$(OPT) -load ../Debug+Asserts/lib/libLLVMLoopPar.so -loop-par <test.bc> test.bc.opt
	$(LLC) -filetype=obj test.bc.opt -o test.o
	#$(CC) test.o ../Debug+Asserts/lib/libLoopParalleling_rt.a -o test
	$(CC) -O0 -c -emit-llvm test2.c -o test2.bc
	$(OPT) -load ../Debug+Asserts/lib/libLLVMLoopPar.so -loop-par <test2.bc> test2.bc.opt
	$(LLC) -filetype=obj test2.bc.opt -o test2.o
	#$(CC) test2.o ../Debug+Asserts/lib/libLoopParalleling_rt.a -o test2

clean:
	rm -f *.bc *.opt *.o test

