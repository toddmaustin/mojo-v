LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=scrambled-compare.o ../common/simon.o ../target/mojov-utils.o

PROG=scrambled-compare

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO scrambled-compare.$(TARGET_REFEXT)
	rm -f FOO
