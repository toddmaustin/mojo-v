
LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=mojov-pctests.o ../common/simon.o
LOCAL_SIMOPTS=--mojov-proofcarrying

PROG=mojov-pctests

include ../Makefile

test: $(TARGET_EXE)
	rm -f FOO
	for i in $(shell seq 0 3); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-dc ../target/dc-proof.txt --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	for i in $(shell seq 20 27); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-dc ../target/dc-proof.txt --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO mojov-pctests.$(TARGET_REFEXT)
	rm -f FOO

