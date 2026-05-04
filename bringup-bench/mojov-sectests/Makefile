
LOCAL_CFLAGS= -std=c2x
LOCAL_LIBS=
LOCAL_OBJS=mojov-sectests.o ../common/simon.o ../target/mojov-utils.o

PROG=mojov-sectests

include ../Makefile

#
# Janky fix to make this build LLVM only.
# GCC cannot build this due to a JAL relocation error in the linker, not yet debugged...
#
TARGET_CC = /opt/riscv/bin/clang
TARGET_REGOUT = -ffixed-x24 -ffixed-x25 -ffixed-x26 -ffixed-x27 -ffixed-x28 -ffixed-x29 -ffixed-x30 -ffixed-x31


test: $(TARGET_EXE)
	rm -f FOO
	for i in $(shell seq 0 246); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO mojov-sectests.$(TARGET_REFEXT)
	rm -f FOO

