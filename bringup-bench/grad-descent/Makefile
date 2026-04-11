
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=grad-descent.o ../common/simon.o ../target/mojov-utils.o

PROG=grad-descent

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO grad-descent.$(TARGET_REFEXT)
	rm -f FOO

