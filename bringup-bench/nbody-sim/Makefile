LOCAL_CFLAGS= -Wno-strict-aliasing
LOCAL_LIBS=
LOCAL_OBJS=nbody-sim.o ../common/simon.o ../target/mojov-utils.o

PROG=nbody-sim

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO nbody-sim.$(TARGET_REFEXT)
	rm -f FOO
