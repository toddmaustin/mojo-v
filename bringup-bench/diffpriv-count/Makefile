LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=diffpriv-count.o ../common/simon.o ../target/mojov-utils.o
LOCAL_SIMOPTS=--mojov-proofcarrying

PROG=diffpriv-count

include ../Makefile

test: $(TARGET_EXE)
	@set -e
	rm -f FOO
	for i in 0 1; do \
	  echo "--- Positive/control test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	for i in 10 11 12 13 14; do \
	  echo "--- Rejection test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO diffpriv-count.$(TARGET_REFEXT)
	rm -f FOO
