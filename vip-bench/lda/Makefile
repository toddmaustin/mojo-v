
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=lda
OBJS=lda.o ../common/utils.o ../common/mathlib.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO lda.out
	rm -f FOO

