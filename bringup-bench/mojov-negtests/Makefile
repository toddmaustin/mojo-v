
LOCAL_CFLAGS=-std=c23
LOCAL_LIBS=
LOCAL_OBJS=mojov-negtests.o ../common/simon.o

PROG=mojov-negtests

include ../Makefile

test: $(TARGET_EXE)
	rm -f FOO
	for i in $(shell seq 0 2); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO mojov-negtests.$(TARGET_REFEXT)
	rm -f FOO

