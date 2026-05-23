#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_device.h"
#include "qom/object.h"

#define TYPE_PQC_ACCEL "pqc_accel"
#define REG_CONTROL      0x00 // 0=reset, 1=start
#define REG_STATUS       0x04 // 0=idle, 1=busy, 2=done, 3=error
#define REG_ALGO         0x08 // 0=ML-KEM-512, 1=ML-KEM-768, 2=ML-KEM-1024
#define BUFFER_IN_ADDR   0x100
#define BUFFER_OUT_ADDR  0x800
#define BUFFER_SIZE      1024

OBJECT_DECLARE_SIMPLE_TYPE(PQCState, PQC_ACCEL)


struct PQCState {
    PCIDevice pdev;
    MemoryRegion mmio;
    
    uint32_t reg_control;
    uint32_t reg_status;
    uint32_t reg_algo;

    uint8_t buffer_in[BUFFER_SIZE];
    uint8_t buffer_out[BUFFER_SIZE];
};

static void pqc_execution(PQCState *s) {
    printf("PQC_ACCEL: Start algorithm %d.\n", s->reg_algo);
    printf("PQC_ACCEL: BUFFER_IN 0%x.\n", s->buffer_in[0]);
    s->buffer_out[0] = s->buffer_in[0] ^ 0x42;
    printf("PQC_ACCEL: BUFFER_OUT 0%x.\n", s->buffer_out[0]);
    s->reg_status = 2;
    s->reg_control = 0;
    printf("PQC_ACCEL: End algorithm %d.\n", s->reg_algo);
}

static uint64_t pqc_mmio_read(void *opaque, hwaddr addr, unsigned size) {
    PQCState *s = opaque;

    if (addr == REG_CONTROL) return s->reg_control;
    if (addr == REG_STATUS) return s->reg_status;
    if (addr == REG_ALGO) return s->reg_algo;

    if (addr >= BUFFER_OUT_ADDR && addr < (BUFFER_OUT_ADDR + BUFFER_SIZE)) {
        hwaddr offset = addr - BUFFER_OUT_ADDR;
        uint64_t val = 0;
        memcpy(&val, &s->buffer_out[offset], size);
        return val;
    }
    return 0;
}

static void pqc_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {
    PQCState *s = opaque;
    if (addr >= BUFFER_IN_ADDR && addr < (BUFFER_IN_ADDR + BUFFER_SIZE)) {
        hwaddr offset = addr - BUFFER_IN_ADDR;
        memcpy(&s->buffer_in[offset], &val, size);
        return;
    }
    switch (addr) {
        case REG_ALGO:
            s->reg_algo = (uint32_t)val;
            break;
        case REG_CONTROL:
            s->reg_control = (uint32_t)val;
            if (s->reg_control == 1) {
                s->reg_status = 1;
                pqc_execution(s);
            }
            break;
        default:
            break;
    }
}

static const MemoryRegionOps pqc_mmio_ops = {
    .read = pqc_mmio_read,
    .write = pqc_mmio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void pqc_realize(PCIDevice *pdev, Error **errp) {
    PQCState *s = PQC_ACCEL(pdev);
    memory_region_init_io(&s->mmio, OBJECT(s), &pqc_mmio_ops, s, "pqc-mmio", 4096);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &s->mmio);
}

static void pqc_class_init(ObjectClass *klass, const void *data) {
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);
    k->realize = pqc_realize;
    k->vendor_id = 0x1af4; 
    k->device_id = 0x1054;
    k->class_id = PCI_CLASS_OTHERS;
}

static const TypeInfo pqc_accel_info = {
    .name          = TYPE_PQC_ACCEL,
    .parent        = TYPE_PCI_DEVICE,
    .instance_size = sizeof(PQCState),
    .class_init    = pqc_class_init,
    .interfaces = (InterfaceInfo[]) {
        { INTERFACE_CONVENTIONAL_PCI_DEVICE },
        { },
    },
};

static void pqc_accel_register_types(void)
{
    type_register_static(&pqc_accel_info);
}

type_init(pqc_accel_register_types)
