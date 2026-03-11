
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=bubble-sort.o ../common/simon.o ../target/mojov-utils.o
LOCAL_SIMOPTS=--mojov-strong

PROG=bubble-sort

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-dc=../target/dc-strong.txt --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO bubble-sort.$(TARGET_REFEXT)
	rm -f FOO

