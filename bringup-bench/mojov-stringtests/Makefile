LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=mojov-stringtests.o ../common/simon.o ../target/mojov-utils.o

PROG=mojov-stringtests

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO mojov-stringtests.$(TARGET_REFEXT)
	rm -f FOO
