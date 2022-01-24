#ifndef EDP_H
#define EDP_H

/**
 * @file
*/

#ifdef __cplusplus

#include "log.h"
#include "boardconfig.h"

#include "snappy.h"

// DATA TYPES

// Efficient Dmx Protocol Commands

enum Edp_Commands : uint8_t {
    Ping                      = 0x00, // Payload: Serial number of requester
    Pong                      = 0x01, // Payload: Serial number of responder
    DmxDataAllZero            = 0x10, // Followed by 1 byte (universeId), no chunk header, no packet header
    DmxData                   = 0x11, // One command for compressed and uncompressed data, sent in chunks
    DmxDataRequest            = 0x12, // Poll the content of a universe
    DiscoveryRequest          = 0x20,
    DiscoveryRespone          = 0x21,
    DiscoveryMute             = 0x22,
    DiscoveryUnMuteAll        = 0x23,
};

// The smallest chunk size this is designed to work on is 32 bytes (RF24 max payload length)
// However, we need to transfer at most 512 byte (One DMX frame). How many chunks do we need?
// 1 byte COMMAND
// 1 byte "DmxData" chunk header (= universe & chunk counter)
//     = 30 byte DMX data per packet maximum
//       512 byte + 4 byte DmxData packet header (crc + universe + full/sparse + sparseOffset)
//       = 516 Byte DmxData Playload
//     516/30 = 18 packets MAX (= 540 byte)  => 5 bit required for the chunk counter => 32 possible values
// => since we could now count 31 chunks, we could also use even smaller chunk sizes (~18 byte)

// Special values for the chunk counter
// actually only 5 bit => 0-31
enum Edp_DmxData_ChunkCounter : uint8_t {
    FirstPacket               = 0
};

// Should occupy one byte
struct Edp_DmxData_ChunkHeader {
    uint8_t                   RESERVED0    : 2; // Reserved for future use ;)
    Edp_DmxData_ChunkCounter  chunkCounter : 5;
    bool                      lastChunk    : 1; // 0 = first or middle chunk, 1 = last chunk
};

// 4 byte
struct Edp_DmxData_PacketHeader {
    uint16_t              crc;
    uint8_t               compressed   : 1; // 0 = raw, 1 = compressed
    uint8_t               sparse      : 1; // 0 = full frame, 1 = sparse
    uint8_t               universeId   : 6; // Universe Id (64 possibilities)
    uint8_t               sparseOffset;    // If sparse: Position the frame starts at
};

/**
 * EDP - The efficient DMX protocol.
 * The EDP class constructs and parses EDP packets that can then be transmitted
 * or are received by classes using EDP (such as Wireless, UDP_EDP, ...).
 * The advantages of EDP are: Sending very short "allZeroe"-packets when the
 * buffer only contains the values "0", data compression using the Snappy
 * algorithm if it reduces the size, sparse packets when the first n slots are
 * not used and automatic chunking (chunk sizes 32 to 520 byte).
*/
class Edp {
  public:
    void init(uint8_t* inData, uint8_t* outData, uint8_t patchingOffset, uint16_t maxSendChunkSize);

    // TODO: Chunk generation with buffer, universe id and max chunk size given
    bool prepareDmxData(uint8_t universeId, uint16_t inDataSize, uint16_t* thisChunkSize, bool* callAgain);

    bool processIncomingChunk(uint16_t chunkSize);

  private:
    bool initOkay;
    uint8_t* inData;
    uint8_t* outData;
    uint8_t patchingOffset;
    uint16_t maxSendChunkSize;

    size_t prepareDmxData_sizeOfDataToBeSent;  // Packetheader + payload length
    uint16_t prepareDmxData_chunkOffset;

    Patching findPatching(uint8_t universeId);

};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

// None yet :)

#ifdef __cplusplus
}
#endif

#endif // EDP_H
