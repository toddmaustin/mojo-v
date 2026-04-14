LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=soundex.o ../common/simon.o ../target/mojov-utils.o

PROG=soundex

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO soundex.$(TARGET_REFEXT)
	rm -f FOO
