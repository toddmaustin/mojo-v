
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=rad-to-deg.o ../common/simon.o ../target/mojov-utils.o

PROG=rad-to-deg

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO rad-to-deg.$(TARGET_REFEXT)
	rm -f FOO

