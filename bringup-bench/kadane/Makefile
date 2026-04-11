
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=kadane.o ../common/simon.o ../target/mojov-utils.o

PROG=kadane

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO kadane.$(TARGET_REFEXT)
	rm -f FOO

