LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=secret-regex.o ../common/simon.o ../target/mojov-utils.o

PROG=secret-regex

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO secret-regex.$(TARGET_REFEXT)
	rm -f FOO
