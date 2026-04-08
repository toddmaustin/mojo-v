LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=mojov-typetests.o ../common/simon.o ../target/mojov-utils.o

PROG=mojov-typetests

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO mojov-typetests.$(TARGET_REFEXT)
	rm -f FOO

