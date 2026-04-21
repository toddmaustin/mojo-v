LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=tiny-NN.o ../common/simon.o ../target/mojov-utils.o

PROG=tiny-NN

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO tiny-NN.$(TARGET_REFEXT)
	rm -f FOO
