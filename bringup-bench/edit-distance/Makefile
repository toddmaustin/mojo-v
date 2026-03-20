
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=edit-distance.o ../common/simon.o ../target/mojov-utils.o

PROG=edit-distance

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO edit-distance.$(TARGET_REFEXT)
	rm -f FOO

