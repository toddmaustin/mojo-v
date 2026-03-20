
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=bitonic-sort.o ../common/simon.o ../target/mojov-utils.o

PROG=bitonic-sort

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO bitonic-sort.$(TARGET_REFEXT)
	rm -f FOO

