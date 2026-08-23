#ifndef UDPPROTOCOL_H
#define UDPPROTOCOL_H

#include <cstdint>


#pragma pack(push, 1)
struct LogPacket {
    char    username[32];
    char    action[32];
    char    target[64];
    int64_t timestamp;
};
#pragma pack(pop)

static constexpr quint16 UDP_PORT = 23118;

#endif
