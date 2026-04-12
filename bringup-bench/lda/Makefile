
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=lda.o ../common/simon.o ../target/mojov-utils.o

PROG=lda

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO 
	$(TARGET_DIFF) FOO lda.$(TARGET_REFEXT)
	rm -f FOO

