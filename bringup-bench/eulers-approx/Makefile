
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=eulers-approx.o ../common/simon.o ../target/mojov-utils.o

PROG=eulers-approx

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO eulers-approx.$(TARGET_REFEXT)
	rm -f FOO

