# Higher-Order Masked Ciphertext Comparison for Lattice-Based Cryptography

This repository contains ARM Cortex-M4 code for higher-order masked ciphertext comparison for lattice-based cryptography. The implementations are described in our paper "Higher-Order Masked Ciphertext Comparison for Lattice-Based Cryptography", Jan-Pieter D'Anvers, Daniel Heinz, Peter Pessl, Michiel van Beirendonck and Ingrid Verbauwhede [(ePrint 2021/1422)](https://eprint.iacr.org/2021/1422) that appeared in TCHES, Volume 2022, Issue 2 [(TCHES)](https://tches.iacr.org/index.php/TCHES/article/view/9483).

## Requirements

Building and running the code requires:

* [arm-none-eabi toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
* [stlink](https://github.com/stlink-org/stlink)
* `python3` with `pyserial`

### libopencm3

The code in this repository uses the [libopencm3](https://github.com/libopencm3/libopencm3) open-source ARM Cortex-M microcontroller library. The repository is built after the [libopencm3-template](https://github.com/libopencm3/libopencm3-template).

After cloning or downloading this repository, it is necessary to initialize [libopencm3](https://github.com/libopencm3/libopencm3):

```bash
git submodule update --init --recursive
make -C libopencm3
```

## Tests and Benchmarks

### Setup

The [Makefile](./Makefile) allows to configure which benchmarks or tests to run.

It is possible to both compile the code for execution on `STM32F407G-DISC1` board or on a host PC by setting the corresponding flag:

```make
PLATFORM = {ARM, host}
```

In both case, binaries are built with `make`. For execution on the `STM32F407G-DISC1` board, the binary can be flashed to the board with `make flash`. The serial output can be retrieved with `make screen`, which should be set-up before flashing the binary. For host execution, code can be suqbsequently be run with `make run`. Host execution sets the `-DDEBUG` flag, which enables sanity-check assertions within the codebase.

The masked comparison target scheme can be configured for Kyber or Saber:

```make
CFLAGS += {-DKYBER, -DSABER}
```

The number of shares can be set:

```make
CFLAGS += {-DNSHARES>=2}
```

The number of tests to run can be selected:

```make
CFLAGS += {-NTESTS>=1}
```

Finally, there are flags that allow to profile the implementation:

```make
CFLAGS += {-DPROFILE_TOP_CYCLES, -DPROFILE_TOP_RAND, -DPROFILE_STEP_CYCLES, -DPROFILE_STEP_RAND}
```

* `PROFILE_x_CYCLES` profiles the number of cycles for either the total execution (`x=TOP`) or the individual steps (`x=STEP`).

* `PROFILE_x_RAND` profiles the requested number of random bytes for either the total execution (`x=TOP`) or the individual steps (`x=STEP`).

Only one of these flags should be set at any point for consistent benchmarks. For example, counting the requested number of random bytes would incur overheads in the cycle counts.

## ChipWhisperer Integration

It is also possible to compile the code for the ChipWhisperer-Lite target. We provide a jupyter notebook for an easy build and evaluation process in the [jupyter](./jupyter) folder.

Requirement is a working [ChipWhisperer Installation](https://github.com/newaetech/chipwhisperer) in Python.

## License

Files developed in this work are released under the [MIT License](./LICENSE). In addition, if you use or build upon the code in this repository, please cite our paper using our [citation key](./CITATION).

[B2A.c](./src/B2A.c) is licensed under [GNU General Public License version 2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).

---

Jan-Pieter D'Anvers, Daniel Heinz, Peter Pessl, Michiel van Beirendonck and Ingrid Verbauwhede
