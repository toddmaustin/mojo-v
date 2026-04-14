
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=string-search.o ../common/simon.o ../target/mojov-utils.o

PROG=string-search

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO string-search.$(TARGET_REFEXT)
	rm -f FOO
