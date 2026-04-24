LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=variability-sample.o ../common/simon.o ../target/mojov-utils.o

PROG=variability-sample

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO variability-sample.$(TARGET_REFEXT)
	rm -f FOO
