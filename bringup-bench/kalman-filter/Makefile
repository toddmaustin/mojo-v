
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=kalman-filter.o ../common/simon.o ../target/mojov-utils.o

PROG=kalman-filter

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO kalman-filter.$(TARGET_REFEXT)
	rm -f FOO

