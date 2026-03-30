
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=mersenne.o ../common/simon.o ../target/mojov-utils.o

PROG=mersenne

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO mersenne.$(TARGET_REFEXT)
	rm -f FOO

