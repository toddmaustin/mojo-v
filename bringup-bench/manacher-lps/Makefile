LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=manacher-lps.o ../common/simon.o ../target/mojov-utils.o

PROG=manacher-lps

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO manacher-lps.$(TARGET_REFEXT)
	rm -f FOO
