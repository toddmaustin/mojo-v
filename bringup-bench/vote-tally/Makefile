LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=vote-tally.o ../common/simon.o ../target/mojov-utils.o

PROG=vote-tally

include ../Makefile

test: $(TARGET_EXE)
	rm -f FOO
	for i in $(shell seq 0 9); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO vote-tally.$(TARGET_REFEXT)
	rm -f FOO
