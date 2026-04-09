# LOCAL_CFLAGS=  -I../lib/SEAL/wrapper  -DVIP_INT
# ifeq ($(MODE), enc)
# LOCAL_LIBS=    -L../lib/SEAL/include/lib/ -lseal-3.4 -pthread -lz -L../lib/SEAL/wrapper/ -lheWrapper
# endif

LOCAL_CFLAGS= 
LOCAL_LIBS= 

PROG=grad-x
OBJS=grad-x.o ../common/utils.o

include ../config.mk
include ../Makefile

test: $(TARGET_EXE)
	-$(TARGET_SIM) $(TARGET_EXE) > FOO
	$(TARGET_DIFF) FOO grad-x.out
	rm -f FOO

