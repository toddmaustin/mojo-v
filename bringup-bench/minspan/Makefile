
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=minspan.o ../common/simon.o ../target/mojov-utils.o

PROG=minspan

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO minspan.$(TARGET_REFEXT)
	rm -f FOO

