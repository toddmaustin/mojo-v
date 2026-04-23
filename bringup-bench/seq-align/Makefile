LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=seq-align.o ../common/simon.o ../target/mojov-utils.o

PROG=seq-align

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO seq-align.$(TARGET_REFEXT)
	rm -f FOO
