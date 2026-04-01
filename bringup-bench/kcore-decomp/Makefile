LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=kcore-decomp.o ../common/simon.o ../target/mojov-utils.o

PROG=kcore-decomp

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO kcore-decomp.$(TARGET_REFEXT)
	rm -f FOO
