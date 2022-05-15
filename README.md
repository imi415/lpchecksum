# LPChecksum

This is a simple CLI utility for NXP LPC series MCUs.
These MCUs utilizes a reserved word in IVT, located at `0x1C` for Cortex-M,
contains a checksum for the first 7 words of the IVT.

This is done by adding the first 8 words together, if the result is 0, the
bootloader will jumps to the user application, otherwise ISP mode will be activated.

## Dependencies
* `elfutils`

## How to build
```bash
mkdir build && cd build
cmake ..
make
```

## How to use
```bash
./lpchecksum ${PATH_TO_TARGET_ELF_FILE}
```

## Is this necessary?
Sort of. Programmers and debug utilities such as OpenOCD or JLink will
automatically computes this value and flash the correct result to targets,
however the verification process will fail if the ELF and target flash contents differs.

So the programmers will not be able to skip the erase/program procedures without patching the ELF.