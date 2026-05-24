# pqc-accel-qemu

## Build
1. Build and install `liboqs`
- Reference: https://openquantumsafe.org/liboqs/getting-started.html
```
git clone -b main https://github.com/open-quantum-safe/liboqs.git
cd liboqs
mkdir build && cd build
cmake -GNinja -DBUILD_SHARED_LIBS=ON ..
ninja
sudo ninja install
sudo ldconfig
pkg-config --modversion liboqs
```

2. Build QEMU with The modified driver

- `git clone --depth 1 https://gitlab.com/qemu-project/qemu.git`
- Copy `qemu/hw/misc/pqc_accel.c` to corresponding place in qemu.
- Copy `qemu/hw/misc/meson.build` to corresponding place in qemu.
- Build QEMU for x86-64 using the following command.
```
# qemu/
mkdir build && cd build
../configure --target-list=x86_64-softmmu --enable-debug
make -j$(nproc)
```
- (Optional) Build with `pqc_accel.c` only.
```
ninja libsystem.a.p/hw_misc_pqc_accel.c.o`
```

3. Build kernal and image with Buildroot
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
  -s \
  -S # if want to stop, before boot
```

Connect with gdb.
```
gdb ./qemu/build/qemu-system-x86_64
(gdb) target remote :1234
(gdb) b pqc_mmio_write
(gdb) c
```

## Manually verify the funcionalities.
```
lspci -v
# 00:04.0 Class 00ff: 1af4:1054
cat /sys/bus/pci/devices/0000:00:04.0/resource
# 0x00000000febf1000 ... <- BAR0
busybox devmem 0xfebf1000 32 # Read BAR0+0
busybox devmem 0xfebf1000 32 1 # Write 1 to BAR0+0
```
