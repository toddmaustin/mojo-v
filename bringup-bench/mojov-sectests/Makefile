
LOCAL_CFLAGS=-std=c23
LOCAL_LIBS=
LOCAL_OBJS=mojov-sectests.o ../common/simon.o ../target/mojov-utils.o

PROG=mojov-sectests

include ../Makefile

test: $(TARGET_EXE)
	rm -f FOO
	for i in $(shell seq 0 246); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO mojov-sectests.$(TARGET_REFEXT)
	rm -f FOO

