LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=private-auction.o ../common/simon.o ../target/mojov-utils.o

PROG=private-auction

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO private-auction.$(TARGET_REFEXT)
	rm -f FOO
