
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=knapsack.o ../common/simon.o ../target/mojov-utils.o

PROG=knapsack

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO knapsack.$(TARGET_REFEXT)
	rm -f FOO

