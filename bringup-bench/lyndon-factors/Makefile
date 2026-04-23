LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=lyndon-factors.o ../common/simon.o ../target/mojov-utils.o

PROG=lyndon-factors

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO lyndon-factors.$(TARGET_REFEXT)
	rm -f FOO
