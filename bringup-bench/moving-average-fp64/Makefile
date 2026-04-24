LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=moving-average-fp64.o ../common/simon.o ../target/mojov-utils.o

PROG=moving-average-fp64

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO moving-average-fp64.$(TARGET_REFEXT)
	rm -f FOO
