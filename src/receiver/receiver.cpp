/**
 * CPSC 3780 Project
 * @author Siebrand Soule <siebrand.soule@uleth.ca>
 * @author Christian Walker <christian.walker@uleth.ca>
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>

#include "../protocol.h"

int sockfd;
char buffer[528];
struct sockaddr_in servaddr, cliaddr;
char *message, *fileName;
int port, len, new_socket;
bool isFile = false;

Protocol *p = new Protocol();
dataPacket *pkt = static_cast<struct dataPacket *>(p->thePacket());

void recv_pkt() {
  while (true) {
    new_socket = recvfrom(sockfd, (char *)buffer, sizeof(buffer), MSG_WAITALL,
                          (struct sockaddr *)&cliaddr, (socklen_t *)&len);
    p->decode(buffer);
    if (p->getTR() == 1) {
      std::cout << "\nPacket is truncated!" << std::endl;
      std::cout << "\nReturning NACK" << std::endl;
      p->encode(buffer, 3);
    } else if (p->getType() == PTYPE_NACK) {
      std::cout << "\nReturning NACK" << std::endl;
      p->encode(buffer, 3);
      sendto(sockfd, p->getPktArr(), p->getPktArrSize(), MSG_CONFIRM,
             (const struct sockaddr *)&cliaddr, len);
      std::cout << "\nNACK sent" << std::endl;
    } else {
      if (isFile) {
        std::ofstream file(fileName);
        file << p->decode(buffer);
        std::cout << "\nMessage received. Contents stored in " << fileName
                  << std::endl;
        file.close();
      } else {
        std::cout << "\nMessage received: " << p->getPayload() << std::endl;
      }
      std::cout << "\nReturning ACK" << std::endl;
      p->encode(buffer, 2);
      sendto(sockfd, p->getPktArr(), p->getPktArrSize(), MSG_CONFIRM,
             (const struct sockaddr *)&cliaddr, len);
      std::cout << "\nACK sent" << std::endl;
      return;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Invalid arguments: [-f data_file] <port number>" << std::endl;
    return -1;
  }

  std::string arg(argv[1]);
  if (arg != "-f") {
    port = atoi(argv[1]);
  } else {
    isFile = true;
    fileName = argv[2];
    port = atoi(argv[3]);
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);

  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  } else {
    std::cout << "Socket binded. Receiver running..." << std::endl;
  }

  len = sizeof(cliaddr);

  std::thread recv_thread(recv_pkt);

  recv_thread.join();
  close(sockfd);
  return 0;
}
