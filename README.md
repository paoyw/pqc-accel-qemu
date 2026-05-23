# pqc-accel-qemu

## Build

1. Build QEMU with The modified driver

- `git clone --depth 1 https://gitlab.com/qemu-project/qemu.git`
- Softlink the `hw/misc/pqc_accel.c` with `qemu/hw/misc/pqc_accel.c`, once.
- Append the bulid rule in `hw/misc/meson.build` to `qemu/hw/misc/meson.build`.
- Build QEMU for x86-64 using the following command.
```
# pwd: qemu/build/
../configure --target-list=x86_64-softmmu --enable-debug
make -j$(nproc)
```

2. Build kernal and image with Buildroot
- `git clone --depth 1 https://github.com/buildroot/buildroot.git`
- Setup and build Buildroot.
```
make qemu_x86_64_defconfig
make menuconfig
# Enable C++ support
# Enable "Root password"
make -j$(nproc)
```

## Run
- Run without gdb
```
./qemu-system-x86_64  \
  -L ../pc-bios \
  -kernel ../../buildroot/output/images/bzImage  \
  -drive file=../../buildroot/output/images/rootfs.ext2,format=raw  \
  -append "root=/dev/sda console=ttyS0"  \
  -nographic \
  -device pqc_accel
```

- Run with gdb
Start and wait for gdb.
```
./qemu-system-x86_64  \
  -L ../pc-bios \
  -kernel ../../buildroot/output/images/bzImage  \
  -drive file=../../buildroot/output/images/rootfs.ext2,format=raw  \
  -append "root=/dev/sda console=ttyS0"  \
  -nographic \
  -device pqc_accel \
  -s -S
```

Connect with gdb.
```
gdb qemu-system-x86_64
(gdb) target remote :1234
(gdb) b pqc_mmio_write
(gdb) c
```

