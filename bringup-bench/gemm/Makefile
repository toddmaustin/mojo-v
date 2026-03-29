
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=gemm.o ../common/simon.o ../target/mojov-utils.o

PROG=gemm

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO gemm.$(TARGET_REFEXT)
	rm -f FOO

