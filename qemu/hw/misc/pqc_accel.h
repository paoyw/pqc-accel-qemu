#ifndef PQC_ACCEL_H
#define PQC_ACCEL_H


#include "hw/pci/pci_device.h"
#include "qom/object.h"
#include <oqs/oqs.h>

#define TYPE_PQC_ACCEL "pqc_accel"
OBJECT_DECLARE_SIMPLE_TYPE(PQCState, PQC_ACCEL)

/* --- Memory Map Registers --- */
#define REG_CONTROL      0x00
#define REG_STATUS       0x04
#define REG_ALGO         0x08
#define BUFFER_IN_ADDR   0x100
#define BUFFER_OUT_ADDR  0x1000
#define BUFFER_SIZE      2048
#define MMIO_BAR_SIZE    8192

/* --- Register Codes --- */
typedef enum {
    RESET = 0,
    KEYPAIR= 1,
    ENCAPS = 2,
    DECAPS = 3,
    SIGN = 4,
    VERIFY = 5
} RegControlCode;

typedef enum {
    IDLE = 0,
    BUSY = 1,
    DONE = 2,
    ERROR = 3
} RegStatusCode;

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
} RegAlgoCode;

struct PQCState {
    PCIDevice pdev;
    MemoryRegion mmio;
    
    uint32_t reg_control;
    uint32_t reg_status;
    uint32_t reg_algo;

    uint8_t buffer_in[BUFFER_SIZE];
    uint8_t buffer_out[BUFFER_SIZE];
};
#endif /* PQC_ACCEL_H */
