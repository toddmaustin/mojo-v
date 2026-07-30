LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=certified-lotto.o ../common/simon.o ../target/mojov-utils.o
LOCAL_SIMOPTS=--mojov-proofcarrying

PROG=certified-lotto

include ../Makefile

test: $(TARGET_EXE)
	@set -e
	rm -f FOO
	for i in 0 1 2; do \
	  echo "--- Positive/control test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	for i in 10 11 12 13 14 15 16; do \
	  echo "--- Rejection test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO certified-lotto.$(TARGET_REFEXT)
	rm -f FOO
