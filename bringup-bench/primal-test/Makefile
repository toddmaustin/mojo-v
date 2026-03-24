LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=primal-test.o ../common/simon.o ../target/mojov-utils.o

PROG=primal-test

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO primal-test.$(TARGET_REFEXT)
	rm -f FOO
