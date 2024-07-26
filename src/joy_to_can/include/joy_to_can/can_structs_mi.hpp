#ifndef CAN_STRUCTS_H
#define CAN_STRUCTS_H

#include <stdint.h>

typedef struct {
    uint32_t id:8;       // 8位CAN ID
    uint32_t data:16;    // 16位数据
    uint32_t mode:5;     // 5位模式
    uint32_t res:3;      // 3位保留位
} can_ext_id_t;

typedef struct {
    uint32_t id:8;       // 8位CAN ID
    uint32_t data:16;    // 16位数据
    uint32_t mode:5;     // 5位模式
    uint32_t res:3;      // 3位保留位
    uint8_t tx_data[8];  // 8字节数据
} can_frame_t;

#endif // CAN_STRUCTS_H

