LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=dp-count.o ../common/simon.o ../target/mojov-utils.o
LOCAL_SIMOPTS=--mojov-proofcarrying

PROG=dp-count

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
	test `grep -c 'Proof verification:        PASS' FOO` -eq 2
	test `grep -c 'Proof verification:        FAIL' FOO` -eq 5
	test `grep -c 'SUCCESS: exact request-bound certified noise graph accepted.' FOO` -eq 2
	test `grep -c 'ERROR: private-count dfhash changed before client decryption.' FOO` -eq 5
	rm -f FOO
