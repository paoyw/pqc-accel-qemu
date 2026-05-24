#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_device.h"
#include "qom/object.h"

#include <oqs/oqs.h>

#define TYPE_PQC_ACCEL "pqc_accel"
#define REG_CONTROL      0x00
#define REG_STATUS       0x04
#define REG_ALGO         0x08
#define BUFFER_IN_ADDR   0x100
#define BUFFER_OUT_ADDR  0x1000
#define BUFFER_SIZE      2048

typedef enum {
    RESET = 0,
    KEYPAIR= 1,
    ENCAPS = 2,
    DECAPS = 3,
    SIGN = 2,
    VERIFY = 3
} uint32_t;

typedef enum {
    IDLE = 0,
    BUSY = 1,
    DONE = 2,
    ERROR = 3
} uint32_t;

typedef enum {
    ML_KEM_512 = 0,
    ML_KEM_768 = 1,
    ML_KEM_1024 = 2,
    ML_DSA_44 = 3,
    ML_DSA_65 = 4,
    ML_DSA_87 = 5,
    SLH_DSA_SHA2_128S = 6,
    SLH_DSA_SHA2_192S = 7,
    SLH_DSA_SHA2_256S = 8,
    SLH_DSA_SHA2_128F = 9,
    SLH_DSA_SHA2_192F = 10,
    SLH_DSA_SHA2_256F = 11,
    SLH_DSA_SHAKE_128S = 12,
    SLH_DSA_SHAKE_192S = 13,
    SLH_DSA_SHAKE_256S = 14,
    SLH_DSA_SHAKE_128F = 15,
    SLH_DSA_SHAKE_192F = 16,
    SLH_DSA_SHAKE_256F = 17,
} uint32_t;

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
            if (s->reg_control == ENC) {
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
