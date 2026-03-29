
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=ntt-kernel.o ../common/simon.o ../target/mojov-utils.o

PROG=ntt-kernel

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO ntt-kernel.$(TARGET_REFEXT)
	rm -f FOO

