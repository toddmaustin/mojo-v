LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=bloom-filter.o ../common/simon.o ../target/mojov-utils.o

PROG=bloom-filter

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO bloom-filter.$(TARGET_REFEXT)
	rm -f FOO
