LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=private-auction.o ../common/simon.o ../target/mojov-utils.o

PROG=private-auction

include ../Makefile

test: $(TARGET_EXE)
	rm -f FOO
	for i in $(shell seq 0 4); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO private-auction.$(TARGET_REFEXT)
	rm -f FOO
