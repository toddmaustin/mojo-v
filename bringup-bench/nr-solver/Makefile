LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=nr-solver.o ../common/simon.o ../target/mojov-utils.o

PROG=nr-solver

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO nr-solver.$(TARGET_REFEXT)
	rm -f FOO
