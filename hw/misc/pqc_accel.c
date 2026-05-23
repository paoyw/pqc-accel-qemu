#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_device.h"
#include "qom/object.h"

#define TYPE_PQC_ACCEL "pqc_accel"
OBJECT_DECLARE_SIMPLE_TYPE(PQCState, PQC_ACCEL)

struct PQCState {
    PCIDevice pdev;
    MemoryRegion mmio;
};

// This handles the "Read" from the hardware
static uint64_t pqc_mmio_read(void *opaque, hwaddr addr, unsigned size) {
    if (addr == 0x00) {
        return 0xABCDEF01; // The "Hello World" magic value
    }
    return 0;
}

// This handles the "Write" to the hardware
static void pqc_mmio_write(void *opaque, hwaddr addr, uint64_t val, unsigned size) {
    if (addr == 0x00) {
        printf("PQC_ACCEL: Guest wrote 0x%" PRIx64 " to hardware!\n", val);
    }
}

static const MemoryRegionOps pqc_mmio_ops = {
    .read = pqc_mmio_read,
    .write = pqc_mmio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void pqc_realize(PCIDevice *pdev, Error **errp) {
    PQCState *s = PQC_ACCEL(pdev);
    // Initialize the MMIO region (BAR 0)
    memory_region_init_io(&s->mmio, OBJECT(s), &pqc_mmio_ops, s, "pqc-mmio", 4096);
    pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &s->mmio);
}

// Register the device with QEMU
static void pqc_class_init(ObjectClass *klass, const void *data) {
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);
    k->realize = pqc_realize;
    k->vendor_id = 0x1234; // Custom Vendor ID
    k->device_id = 0x5678; // Custom Device ID
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
