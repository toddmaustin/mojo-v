LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=private-join.o ../common/simon.o ../target/mojov-utils.o

PROG=private-join

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO private-join.$(TARGET_REFEXT)
	rm -f FOO
