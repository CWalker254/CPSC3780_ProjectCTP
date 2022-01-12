/**
 * CPSC 3780 Project
 * @author Siebrand Soule <siebrand.soule@uleth.ca>
 * @author Christian Walker <christian.walker@uleth.ca>
 */
#include "../src/protocol.h"
#include "gtest/gtest.h"  // google test framework

class protocolTest : public testing::Test {
 protected:
  Protocol *p;

  void SetUp() { p = new Protocol; }

  void TearDown() { delete p; }
};

// Type
TEST_F(protocolTest, getType) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->type = 1;
  EXPECT_EQ(PTYPE_DATA, p->getType());

  ptr->type = PTYPE_ACK;
  EXPECT_EQ(PTYPE_ACK, p->getType());

  ptr->type = PTYPE_NACK;
  EXPECT_EQ(3, p->getType());
}

TEST_F(protocolTest, setType) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setType(0xAA);
  EXPECT_EQ(2, ptr->type);
}

// TR
TEST_F(protocolTest, getTR) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->tr = 1;
  EXPECT_EQ(1, p->getTR());
}

TEST_F(protocolTest, setTR) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setTR(0xAA);
  EXPECT_EQ(1, ptr->tr);
}

// Window
TEST_F(protocolTest, getWindow) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->window = 0x12;
  EXPECT_EQ(0x12, p->getWindow());
}

TEST_F(protocolTest, setWindow) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setWindow(0xB2);
  EXPECT_EQ(0x12, ptr->window);

  p->setWindow(0b10010);
  EXPECT_EQ(0b10010, ptr->window);
}

// Sequence num
TEST_F(protocolTest, getSeqnum) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->seqnum = 1;
  EXPECT_EQ(1, p->getSeqnum());
}

TEST_F(protocolTest, setSeqnum) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setSeqnum(0b01);
  EXPECT_EQ(0, ptr->seqnum);

  p->setSeqnum(0b10);
  EXPECT_EQ(1, ptr->seqnum);

  p->setSeqnum(0b11);
  EXPECT_EQ(0, ptr->seqnum);
}

// Length
TEST_F(protocolTest, getLength) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->lengthMSB = 0x12;
  ptr->lengthLSB = 0x8F;
  EXPECT_EQ(0x128F, p->getLength());
}

TEST_F(protocolTest, setLength) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setLength(0x1FF);
  EXPECT_EQ(0x1, ptr->lengthMSB);
  EXPECT_EQ(0xFF, ptr->lengthLSB);
}

// Timestamp
TEST_F(protocolTest, getTimestamp) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->timestamp = 52;
  EXPECT_EQ(52, p->getTimestamp());
}

TEST_F(protocolTest, setTimestamp) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setTimestamp(0b01);
  EXPECT_EQ(1, ptr->timestamp);

  p->setTimestamp(0b01);
  EXPECT_EQ(2, ptr->timestamp);

  p->setTimestamp(0b01);
  EXPECT_EQ(3, ptr->timestamp);

  p->setTimestamp(0b10);
  EXPECT_EQ(3, ptr->timestamp);

  p->setTimestamp(0b11);
  EXPECT_EQ(3, ptr->timestamp);
}

// CRC1
TEST_F(protocolTest, getCRC1) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->crc1 = 0x129DE48F;
  EXPECT_EQ(0x129DE48F, p->getCRC1());
}

TEST_F(protocolTest, setCRC1) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setCRC1(0xAA55B695);
  EXPECT_EQ(0xAA55B695, ptr->crc1);
}

/// Payload
TEST_F(protocolTest, getPayload) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->payload = (char *)"hello";
  EXPECT_EQ("hello", p->getPayload());
}

TEST_F(protocolTest, setPayload) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setLength(0xAA95);
  p->setPayload((char *)"hello");
  EXPECT_EQ("hello", ptr->payload);
}

// CRC2
TEST_F(protocolTest, getCRC2) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  ptr->crc2 = 0x129DE48F;
  EXPECT_EQ(0x129DE48F, p->getCRC2());
}

TEST_F(protocolTest, setCRC2) {
  struct dataPacket *ptr = static_cast<struct dataPacket *>(p->thePacket());
  p->setCRC2(0xAA55B695);
  EXPECT_EQ(0xAA55B695, ptr->crc2);
}
