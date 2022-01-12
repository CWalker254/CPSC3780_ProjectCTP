/**
 * CPSC 3780 Project
 * @author Siebrand Soule <siebrand.soule@uleth.ca>
 * @author Christian Walker <christian.walker@uleth.ca>
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "../protocol.h"

int sockfd;
struct sockaddr_in servaddr;
int len;
Protocol p;
char *message, *file, *address, *tmp = NULL;
int port;
char buffer[528];
using namespace std::chrono;
high_resolution_clock::time_point t1;
high_resolution_clock::time_point t2;

void handle_NACK() {
  p.encode(message, 1);
  sendto(sockfd, p.getPktArr(), p.getPktArrSize(), MSG_CONFIRM,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));
  t1 = high_resolution_clock::now();
}

void handle_socket() {
  char header[16];
  int header_size;
  while (true) {
    header_size = recvfrom(sockfd, (char *)header, sizeof(header), MSG_WAITALL,
                           (struct sockaddr *)&servaddr, (socklen_t *)&len);
    t2 = high_resolution_clock::now();
    header[header_size] = '\0';
    duration<double> time_span =
        duration_cast<duration<double>>((t2 - t1) * 1000);
    p.decode(header);
    std::cout << "\nSpeed of packet send and return: " << time_span.count()
              << " milliseconds" << std::endl;
    if (p.getType() == PTYPE_NACK)
      handle_NACK();
    else if (p.getType() == PTYPE_ACK)
      return;
  }
}

void sendPacket() {
  p.encode(message, 1);

  sendto(sockfd, p.getPktArr(), p.getPktArrSize(), MSG_CONFIRM,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));
  t1 = high_resolution_clock::now();
  std::cout << "\nPacket sent." << std::endl;
  duration<double> time_span;
  while (true) {
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>((t2 - t1) * 1000);
    int retran_t = time_span.count();
    if (retran_t > 2000) {
      handle_NACK();
      retran_t = 0;
    }
    if (p.getType() == PTYPE_ACK) return;
  }
}

int main(int argc, char *argv[]) {
  dataPacket *pkt = static_cast<struct dataPacket *>(p.thePacket());

  /**
   * Command line arguments must contain at least a message,
   * IPv4 address, and port number.
   */
  if (argc < 4) {
    std::cerr << "Invalid arguments: [-f data_file] <address> <port number>"
              << std::endl;
    return -1;
  }

  /**
   * If "-f" isn't entered as first argument, assume terminal message
   * will be sent. Otherwise, read contents from specified file and send
   * in message.
   */
  std::string arg(argv[1]);
  if (arg != "-f") {
    message = argv[1];
    address = argv[2];
    port = atoi(argv[3]);
  } else {
    file = argv[2];
    address = argv[3];
    port = atoi(argv[4]);
    std::ifstream dataFile;
    dataFile.open(file);
    if (dataFile.fail()) {
      std::cerr << "Incorrect file format." << std::endl;
      return -1;
    }
    std::string strMessage((std::istreambuf_iterator<char>(dataFile)),
                           (std::istreambuf_iterator<char>()));
    message = (char *)strMessage.c_str();
    dataFile.close();
  }

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  int new_socket;

  std::thread sendto_thread(sendPacket);
  std::thread recv_thread(handle_socket);

  recv_thread.join();
  sendto_thread.join();

  close(sockfd);
  return 0;
}
