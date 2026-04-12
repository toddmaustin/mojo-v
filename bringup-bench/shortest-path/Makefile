
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=shortest-path.o ../common/simon.o ../target/mojov-utils.o

PROG=shortest-path

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO shortest-path.$(TARGET_REFEXT)
	rm -f FOO

