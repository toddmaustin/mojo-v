CC  = gcc
CXX = g++
LD  = g++

OPT_CFLAGS = -O2 -g -I../common

NA_CFLAGS =
NA_LIBS =
NA_SIM =
NA_DIFF = diff

DO_CFLAGS =
DO_LIBS =
DO_SIM =
DO_DIFF = diff

ENC_CFLAGS = -I../../tf-sdk/include -DTARGET_EMUL
ENC_LIBS = ../../tf-sdk/lib/libtf.a -lssl -lcrypto
ENC_SIM = TF_CONTRACT=../../tf-sdk/trustforge/keyfiles/dataowner-keyenc.bin TF_DEBUGKEY=../../tf-sdk/trustforge/keyfiles/dataowner-keybin.bin

ENC_DIFF = diff

