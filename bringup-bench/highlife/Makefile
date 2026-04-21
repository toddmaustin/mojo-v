LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=highlife.o ../common/simon.o ../target/mojov-utils.o

PROG=highlife

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO highlife.$(TARGET_REFEXT)
	rm -f FOO
