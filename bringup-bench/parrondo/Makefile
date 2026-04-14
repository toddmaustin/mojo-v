
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=parrondo.o ../common/simon.o ../target/mojov-utils.o

PROG=parrondo

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO parrondo.$(TARGET_REFEXT)
	rm -f FOO

