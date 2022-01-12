/**
 * CPSC 3780 Project
 * @author Siebrand Soule <siebrand.soule@uleth.ca>
 * @author Christian Walker <christian.walker@uleth.ca>
 */

#ifndef PROTOCOL_H_INCLUDED
#define PROTOCOL_H_INCLUDED

#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

#include <bitset>
#include <cmath>

#define DATA_LEN 512

struct dataPacket {
  uint8_t type;
  uint8_t tr;
  uint8_t window;
  uint8_t seqnum;
  uint8_t lengthMSB;
  uint8_t lengthLSB;
  uint32_t timestamp;
  uint32_t crc1;
  char *payload;
  uint32_t crc2;
  uint64_t header;
  char data[DATA_LEN];
};

enum {
  PTYPE_DATA = 1,
  PTYPE_ACK = 2,
  PTYPE_NACK = 3,
};

class Protocol {
 public:
  // default constructor
  Protocol();

  // 2 bit field indicating 1 for data, 2 for ack, 3 for nack
  uint8_t getType();
  void setType(uint8_t val);

  /**
   * 1 bit field indicating
   * that network has stripped off payload from PTYPE_DATA
   */
  uint8_t getTR();
  void setTR(uint8_t val);

  // 5 bit field setting size of receiving window of source host (0-31)
  uint8_t getWindow();
  void setWindow(uint8_t val);

  /** 8 bit field corresponding to:
   * PTYPE_DATA: sequence number of data packet
   * PTYPE_ACK: number of next sequence expected
   * PTYPE_NACK: sequence number of the truncated PTYPE_DATA packet received
   */
  uint8_t getSeqnum();
  // This value will most likely be getType();
  void setSeqnum(uint8_t value);

  // 16 bit field representing the number of bytes in the payload of packet
  uint16_t getLength();
  void setLength(uint16_t val);

  /**
   * Optional, 4 bytes representing an unspecified value with unspecified
   * endianness
   */
  uint32_t getTimestamp();
  // This value will most likely be getType();
  void setTimestamp(uint8_t value);

  // Calculates CRC32
  uint32_t calcCRC32(const char *data, size_t length);

  void checkCRC(uint32_t crc1, uint32_t crc2, char *buf);

  // 4 bytes containing the value obtained by applying the CRC32 algo to header
  uint32_t getCRC1();
  void setCRC1(uint32_t c1);

  // max 512 bytes containing the data transferred by the protocol
  char *getPayload();
  // This value will most likely be getTR();
  char *setPayload(char *message);

  // Optional, 4 bytes containing the result of CRC32 function over the payload
  uint32_t getCRC2();
  void setCRC2(uint32_t c2);

  // returns pointer to the structure holding the thePacket, including the
  // headers To be used with recvfrom or sendto void *thePacket();
  void *thePacket();

  void *thePayload();

  void encode(char *message, int type);
  char *decode(char *buf);

  uint64_t setHeader();
  uint64_t getHeader();

  char *getPktArr();
  int getPktArrSize();

 private:
  struct dataPacket packet;
  char pktArr[528];
};

#endif
