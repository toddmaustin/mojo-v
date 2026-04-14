LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=mojov-mathtests.o ../common/simon.o ../target/mojov-utils.o

PROG=mojov-mathtests

include ../Makefile

test: $(TARGET_EXE)
	$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE)
