
LOCAL_CFLAGS=
LOCAL_LIBS=

PROG=edit-distance
OBJS=edit-distance.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO edit-distance.out
	rm -f FOO

