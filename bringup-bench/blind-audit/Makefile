LOCAL_CFLAGS=
LOCAL_LIBS=
LOCAL_OBJS=blind-audit.o ../common/simon.o ../target/mojov-utils.o
LOCAL_SIMOPTS=--mojov-proofcarrying

PROG=blind-audit

include ../Makefile

test: $(TARGET_EXE)
	@set -e; \
	for i in 0 1 2; do \
	  echo "--- Positive/control test $$i ---"; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE); \
	done; \
	for i in 10 11 12 13 14 15 16 17; do \
	  echo "--- Rejection test $$i ---"; \
	  if $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE); then \
	    echo "ERROR: malicious test $$i was accepted"; exit 1; \
	  fi; \
	done
