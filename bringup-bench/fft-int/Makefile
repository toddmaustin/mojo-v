
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=fft-int.o ../common/simon.o ../target/mojov-utils.o

PROG=fft-int

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO fft-int.$(TARGET_REFEXT)
	rm -f FOO

