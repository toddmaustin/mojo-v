
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=psi.o ../common/simon.o ../target/mojov-utils.o

PROG=psi

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO psi.$(TARGET_REFEXT)
	rm -f FOO

