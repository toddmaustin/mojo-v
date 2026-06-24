
LOCAL_CFLAGS=-std=c23
LOCAL_LIBS=
LOCAL_OBJS=mojov-sectests.o ../common/simon.o ../target/mojov-utils.o

PROG=mojov-sectests

include ../Makefile

#
# GCC cannot build this due to a JAL relocation overflow: GCC generates the
# 247-case switch dispatch in a separate .text.jmpN section, which the linker
# places after libgcc (~1MB of soft-float code), making JAL targets unreachable.
# Use TARGET=mojov-llvm which builds with our custom Mojo-V clang.
#


test: $(TARGET_EXE)
	rm -f FOO
	for i in $(shell seq 0 246); do \
	  echo "--- Test $$i ---" >> FOO; \
	  $(TARGET_SIM) --mojov-arg=$$i --mojov-verbose ./$(TARGET_EXE) >> FOO; \
	done
	$(TARGET_DIFF) FOO mojov-sectests.$(TARGET_REFEXT)
	rm -f FOO
