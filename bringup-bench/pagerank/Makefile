
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=pagerank.o ../common/simon.o ../target/mojov-utils.o

PROG=pagerank

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO pagerank.$(TARGET_REFEXT)
	rm -f FOO
