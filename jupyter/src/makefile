# Hey Emacs, this is a -*- makefile -*-
#----------------------------------------------------------------------------
#
# Makefile for ChipWhisperer SimpleSerial-AES Program
#
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make coff = Convert ELF to AVR COFF.
#
# make extcoff = Convert ELF to AVR Extended COFF.
#
# make program = Download the hex file to the device, using avrdude.
#                Please customize the avrdude settings below first!
#
# make debug = Start either simulavr or avarice as specified for debugging,
#              with avr-gdb or avr-insight as the front end for debugging.
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# make filename.i = Create a preprocessed source file for use in submitting
#                   bug reports to the GCC project.
#
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------

# Target file name (without extension).
# This is the name of the compiled .hex file.
TARGET = simpleserial-hoComp

# List C source files here.
# Header files (.h) are automatically pulled in.
SRC += simpleserial-hoComp.c ../../src/ReduceComparisons.c ../../src/SecAdd.c ../../src/SecAnd.c ../../common/randombytes.c ../../src/A2B.c ../../src/B2A.c ../../src/BooleanEqualityTest.c ../../src/MaskedComparison.c 

ASRC += ../../src/B2A_asm.S ../../src/A2B_asm.S

EXTRAINCDIRS += -I ../../src -I ../../common -I ../../. -I ../hal

# -----------------------------------------------------------------------------

ifeq ($(CRYPTO_OPTIONS),)
CRYPTO_OPTIONS = AES128C
endif

#Add simpleserial project to build
include ../simpleserial/Makefile.simpleserial

FIRMWAREPATH = ../.
include $(FIRMWAREPATH)/Makefile.inc

CFLAGS += -DNO_RANDOM -DSABER -DNSHARES=2 -DCW