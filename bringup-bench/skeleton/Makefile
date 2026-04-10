
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=skeleton.o ../common/simon.o ../target/mojov-utils.o

PROG=skeleton

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO skeleton.$(TARGET_REFEXT)
	rm -f FOO

