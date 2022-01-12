// /**
//  * CPSC 3780 Project
//  * @author Siebrand Soule <siebrand.soule@uleth.ca>
//  * @author Christian Walker <christian.walker@uleth.ca>
//  */
//
#include "protocol.h"

#include <algorithm>
#include <iostream>

Protocol::Protocol() { packet.timestamp = 0; }

uint8_t Protocol::getType() {
  if (packet.type == 1) return PTYPE_DATA;
  if (packet.type == 2) return PTYPE_ACK;
  if (packet.type == 3)
    return PTYPE_NACK;
  else
    return -1;
}

void Protocol::setType(uint8_t val) { packet.type = (val >> 6); }

uint8_t Protocol::getTR() { return packet.tr; }

void Protocol::setTR(uint8_t val) { packet.tr = ((val >> 5) % 2); }

uint8_t Protocol::getWindow() { return packet.window; }

void Protocol::setWindow(uint8_t val) { packet.window = (val & 0x1F); }

uint8_t Protocol::getSeqnum() { return packet.seqnum; }

// This value will most likely be getType();
void Protocol::setSeqnum(uint8_t value) {
  if (value == PTYPE_DATA)
    packet.seqnum = 0;
  else if (value == PTYPE_ACK)
    packet.seqnum += 1 % static_cast<int>(pow(2, 8));
  else if (value == PTYPE_NACK)
    packet.seqnum = 0;
}

uint16_t Protocol::getLength() {
  return packet.lengthLSB | (packet.lengthMSB << 8);
}

void Protocol::setLength(uint16_t val) {
  if (val >= 512) val = 511;
  packet.lengthMSB = (val >> 8);
  packet.lengthLSB = (val & 255);
}

uint32_t Protocol::getTimestamp() { return packet.timestamp; }

// This value will most likely be getType();
void Protocol::setTimestamp(uint8_t value) {
  if (value == PTYPE_DATA) packet.timestamp += 1;
}

uint32_t Protocol::calcCRC32(const char *data, size_t length) {
  uLong crc = crc32(0L, NULL, 0);
  return crc32(crc, (const Bytef *)data, length);
}

void Protocol::checkCRC(uint32_t crc1, uint32_t crc2, char *buf) {
  uint32_t c1 = calcCRC32(buf, 8);
  uint32_t c2 = calcCRC32(getPayload(), getLength());

  if (c1 != crc1) {
    std::cerr << "\nERROR: Incorrect CRC1 value!" << std::endl;
    std::cerr << " Expected CRC1 value: " << std::hex << crc1 << std::endl;
    std::cerr << " Calculated CRC1 value: " << c1 << std::dec << std::endl;
    packet.type = PTYPE_NACK;
  }
  if (c2 != crc2) {
    std::cerr << "\nERROR: Incorrect CRC2 value!" << std::endl;
    std::cerr << " Expected CRC2 value: " << std::hex << crc2 << std::endl;
    std::cerr << " Calculated CRC2 value: " << c2 << std::dec << std::endl;
    packet.type = PTYPE_NACK;
  }
}

uint32_t Protocol::getCRC1() { return packet.crc1; }

void Protocol::setCRC1(uint32_t c1) { packet.crc1 = c1; }

char *Protocol::getPayload() { return packet.payload; }

char *Protocol::setPayload(char *message) {
  char *new_buf;
  if (getTR() == 0) {
    if (message != nullptr) {
      if (strlen(message) > 512) {
        char buf1[512];
        char buf2[512];
        snprintf(buf1, 513, (const char *)message);
        packet.payload = buf1;
        strncpy(buf2, message + 512, strlen(message));
        new_buf = buf2;
      } else {
        packet.payload = message;
        new_buf = (char)0;
      }
    }
  } else {
    packet.payload = (char)0;
    new_buf = message;
  }
  return new_buf;
}

uint32_t Protocol::getCRC2() { return packet.crc2; }

void Protocol::setCRC2(uint32_t c2) { packet.crc2 = c2; }

void *Protocol::thePacket() { return &packet; }

void *Protocol::thePayload() { return packet.data; }

char *Protocol::getPktArr() { return pktArr; }

int Protocol::getPktArrSize() { return (16 + getLength()); }

void Protocol::encode(char *message, int type) {
  std::cout << "\nEncoding packet..." << std::endl;

  if (type == 2) {
    packet.type = PTYPE_ACK;
    setTR(0b01000000);
    setSeqnum(getType());
    setLength(0);
    setPayload({0});
    setTimestamp(getType());
    setWindow(31 - getSeqnum());
  } else if (type == 3) {
    packet.type = PTYPE_NACK;
    setTR(0b01000000);
    setSeqnum(getType());
    setLength(0);
    setPayload({0});
    setTimestamp(getType());
    setWindow(31 - getSeqnum());
  } else if (type == 1) {
    packet.type = PTYPE_DATA;
    setTR(0b01000000);
    setWindow(0b01000000);
    setSeqnum(getType());
    setLength(strlen(message));
    setTimestamp(1);
  }

  uint8_t firstByte = (getType() << 6) | (getTR() << 5) | getWindow();
  pktArr[0] = (char)firstByte;
  uint8_t secondByte = getSeqnum();
  pktArr[1] = (char)secondByte;
  uint8_t thirdByte = packet.lengthMSB;
  pktArr[2] = (char)thirdByte;
  uint8_t fourthByte = packet.lengthLSB;
  pktArr[3] = (char)fourthByte;
  uint32_t feByte = packet.timestamp;

  uint64_t firstSegment =
      (((firstByte << 8 | secondByte) << 8 | thirdByte) << 8 | fourthByte);
  packet.header = (firstSegment << 32) | packet.timestamp;

  uint8_t t1 = (packet.timestamp >> 24) & 0b11111111;
  uint8_t t2 = (packet.timestamp >> 16) & 0b11111111;
  uint8_t t3 = (packet.timestamp >> 8) & 0b11111111;
  uint8_t t4 = packet.timestamp & 0b11111111 & 0b11111111;

  pktArr[4] = (char)t1;
  pktArr[5] = (char)t2;
  pktArr[6] = (char)t3;
  pktArr[7] = (char)t4;

  std::cout << " Type: " << (int)packet.type << std::endl;
  std::cout << " TR: " << (int)packet.tr << std::endl;
  std::cout << " Window: " << (int)packet.window << std::endl;
  std::cout << " Seqnum: " << (int)packet.seqnum << std::endl;
  if (getType() == PTYPE_DATA) {
    std::cout << " Length: " << getLength() << std::endl;
    std::cout << " LengthMSB: " << (int)packet.lengthMSB << std::endl;
    std::cout << " LengthLSB: " << (int)packet.lengthLSB << std::endl;
  }
  std::cout << " Timestamp: " << (int)packet.timestamp << std::endl;

  uint32_t crc1 = calcCRC32(pktArr, sizeof(packet.header));
  std::cout << " Crc1: " << std::hex << crc1 << std::dec << std::endl;
  setCRC1(crc1);
  uint8_t c1 = (packet.crc1 >> 24) & 0b11111111;
  uint8_t c2 = (packet.crc1 >> 16) & 0b11111111;
  uint8_t c3 = (packet.crc1 >> 8) & 0b11111111;
  uint8_t c4 = packet.crc1 & 0b11111111 & 0b11111111;

  pktArr[8] = (char)c1;
  pktArr[9] = (char)c2;
  pktArr[10] = (char)c3;
  pktArr[11] = (char)c4;

  setPayload(message);
  if (getType() == PTYPE_DATA)
    std::cout << " Payload: " << getPayload() << std::endl;

  int offset = 12;
  for (int i = 0; i < getLength(); i++) pktArr[i + offset] = message[i];

  uint32_t crc2 = calcCRC32((const char *)message, getLength());
  if (getType() == PTYPE_DATA)
    std::cout << " Crc2: " << std::hex << crc2 << std::dec << std::endl;
  setCRC2(crc2);

  uint8_t c5 = (packet.crc2 >> 24) & 0b11111111;
  uint8_t c6 = (packet.crc2 >> 16) & 0b11111111;
  uint8_t c7 = (packet.crc2 >> 8) & 0b11111111;
  uint8_t c8 = packet.crc2 & 0b11111111 & 0b11111111;

  int index = getLength() + 12;
  pktArr[index] = c5;
  pktArr[index + 1] = c6;
  pktArr[index + 2] = c7;
  pktArr[index + 3] = c8;
}

char *Protocol::decode(char *buf) {
  std::cout << "\nDecoding packet..." << std::endl;
  setType(buf[0]);
  setTR(buf[0]);
  setWindow(buf[0]);
  setSeqnum(getType());
  setLength(buf[2] << 8 | buf[3]);
  setTimestamp(getType());

  uint8_t c1 = buf[8];
  uint8_t c2 = buf[9];
  uint8_t c3 = buf[10];
  uint8_t c4 = buf[11];
  setCRC1(((c1 << 8 | c2) << 8 | c3) << 8 | c4);

  int offset = 12;
  char message[getLength()];
  for (int i = 0; i < getLength(); i++) message[i] = buf[i + offset];
  setPayload(message);

  c1 = buf[getLength() + 12];
  c2 = buf[getLength() + 13];
  c3 = buf[getLength() + 14];
  c4 = buf[getLength() + 15];
  setCRC2(((c1 << 8 | c2) << 8 | c3) << 8 | c4);

  if (getType() == PTYPE_ACK) {
    std::cout << "\nACK received\n" << std::endl;
    std::cout << " Type: " << (int)getType() << std::endl;
    std::cout << " TR: " << (int)getTR() << std::endl;
    std::cout << " Window: " << (int)getWindow() << std::endl;
    std::cout << " Seqnum: " << (int)getSeqnum() << std::endl;
    std::cout << " Timestamp: " << getTimestamp() << std::endl;
    std::cout << " Crc1: " << std::hex << getCRC1() << std::dec << std::endl;
  } else if (getType() == PTYPE_NACK) {
    std::cout << "\nNACK received\n" << std::endl;
    std::cout << " Type: " << (int)getType() << std::endl;
    std::cout << " TR: " << (int)getTR() << std::endl;
    std::cout << " Window: " << (int)getWindow() << std::endl;
    std::cout << " Seqnum: " << (int)getSeqnum() << std::endl;
    std::cout << " Timestamp: " << getTimestamp() << std::endl;
    std::cout << " Crc1: " << std::hex << getCRC1() << std::dec << std::endl;
  } else {
    std::cout << " Type: " << (int)getType() << std::endl;
    std::cout << " TR: " << (int)getTR() << std::endl;
    std::cout << " Window: " << (int)getWindow() << std::endl;
    std::cout << " Seqnum: " << (int)getSeqnum() << std::endl;
    std::cout << " Length: " << getLength() << std::endl;
    std::cout << " LengthMSB: " << (int)packet.lengthMSB << std::endl;
    std::cout << " LengthLSB: " << (int)packet.lengthLSB << std::endl;
    std::cout << " Timestamp: " << getTimestamp() << std::endl;
    std::cout << " Crc1: " << std::hex << getCRC1() << std::dec << std::endl;
    if (getTR() == 0) {
      std::cout << " Payload: " << getPayload() << std::endl;
      std::cout << " Crc2: " << std::hex << getCRC2() << std::dec << std::endl;
    }
    checkCRC(getCRC1(), getCRC2(), buf);
  }

  return getPayload();
}
