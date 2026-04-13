
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=kalman-filter.o ../common/simon.o ../target/mojov-utils.o

PROG=kepler-calc

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO kepler-calc.$(TARGET_REFEXT)
	rm -f FOO
