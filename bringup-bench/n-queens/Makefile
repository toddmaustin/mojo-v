LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=n-queens.o ../common/simon.o ../target/mojov-utils.o

PROG=n-queens

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO n-queens.$(TARGET_REFEXT)
	rm -f FOO
