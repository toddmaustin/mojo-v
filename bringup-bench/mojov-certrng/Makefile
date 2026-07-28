LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=mojov-certrng.o ../common/simon.o ../target/mojov-utils.o
LOCAL_SIMOPTS=--mojov-proofcarrying
PROG=mojov-certrng
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO mojov-certrng.$(TARGET_REFEXT)
	rm -f FOO

