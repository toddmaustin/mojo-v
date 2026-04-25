LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=chi-squared.o ../common/simon.o ../target/mojov-utils.o

PROG=chi-squared

include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) --mojov-verbose ./$(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO chi-squared.$(TARGET_REFEXT)
	rm -f FOO
