
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=gcd-list.o ../common/simon.o ../target/mojov-utils.o

PROG=gcd-list

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO gcd-list.$(TARGET_REFEXT)
	rm -f FOO

