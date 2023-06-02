/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Pennsylvania
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#include "penn-chord.h"
#include "../lib/thread_pool.hpp"


#define DEBUG 0


PennChord::PennChord ()
{

}

PennChord::~PennChord ()
{

}

//These aren't class methods as it makes it easier to use with pthread library if they're not.
void* StabilizeThread(void* args) {
  while (true) {
    sleep(1); //1000ms
    static_cast<PennChord*>(args)->Stabilize();
  }
  return NULL;
}

void* FixFingersThread(void* args) {
  while (true) {
    usleep(100000); //100ms
    static_cast<PennChord*>(args)->FixFingers();
  }
  return NULL;
}

void* CommandLineThread(void* args) {
  while (true) {
    std::string s;
    std::getline(std::cin, s);
    std::stringstream ss {s};
    std::vector<std::string> tokens;

    while(ss.good()) {
      std::string substr;
      std::getline(ss, substr, ' ');
      tokens.push_back(substr);
    }
    
    if (tokens.at(0) == "QUIT") {
      exit(0); //End program
    }

    std::cout << std::endl << "Please wait..." << std::endl << std::flush;
    static_cast<PennChord*>(args)->ProcessCommand(tokens);
    std::cout << "Command Executed" << std::endl << std::endl << std::flush;
  }
  return NULL;
}


/** https://stackoverflow.com/questions/15653695/how-to-convert-ip-address-in-char-to-uint32-t-in-c
 * Convert human readable IPv4 address to UINT32
 * @param pDottedQuad   Input C string e.g. "192.168.0.1"
 * @param pIpAddr       Output IP address as UINT32
 * return 1 on success, else 0
 */
int ipStringToNumber (const char*       pDottedQuad,
                              unsigned int *    pIpAddr)
{
   unsigned int            byte3;
   unsigned int            byte2;
   unsigned int            byte1;
   unsigned int            byte0;
   char              dummyString[2];

   /* The dummy string with specifier %1s searches for a non-whitespace char
    * after the last number. If it is found, the result of sscanf will be 5
    * instead of 4, indicating an erroneous format of the ip-address.
    */
   if (sscanf (pDottedQuad, "%u.%u.%u.%u%1s",
                  &byte3, &byte2, &byte1, &byte0, dummyString) == 4)
   {
      if (    (byte3 < 256)
           && (byte2 < 256)
           && (byte1 < 256)
           && (byte0 < 256)
         )
      {
         *pIpAddr  =   (byte3 << 24)
                     + (byte2 << 16)
                     + (byte1 << 8)
                     +  byte0;

         return 1;
      }
   }

   return 0;
}

void* ReceiveThread(void* args) {
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  uint8_t buff[4096];
  struct sockaddr_in my_addr, cli_addr;
  int n;


  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket in ReceiveThread()");
    exit(1);
  }

  int opt = 1;
  // Forcefully attaching socket to the port
  if (setsockopt(sockfd, SOL_SOCKET,
                  SO_REUSEADDR, &opt,
                  sizeof(opt))) {
      perror("setsockopt");
      exit(1);
  }

  bzero((char *) &my_addr, sizeof(my_addr));
  portno = static_cast<PennChord*>(args)->GetAppPort();

  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY; //monitor all interfaces (Aka IPs this host associated with)
  my_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
    perror("ERROR on binding in ReceiveThread()");
    exit(1);
  }

  if (listen(sockfd, 100) < 0) { // Allow up to 100 pending TCP SYN connections 
    perror("Error on listen()");
    exit(1);
  } 

  while(true) {
    clilen = sizeof(cli_addr);
    std::cout << "Waiting to accept connection" << std::endl << std::flush;
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
      perror("ERROR on accept in ReceiveThread()");
      continue;
    }
    
    fprintf(stderr, "established TCP connection from %s port %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    n = read(newsockfd, buff, 4095);
    if (n == -1) {
      perror("Error on read() in ReceiveThread()");
      continue;
    }

    BufferV2 b{};
    b.Write(buff, n);

    // std::cout << "Size of message rxed: " << n << std::endl << std::flush;
    // for (int i = 0; i < n; i++) {
    //     std::cout << buff[i] << ", ";
    // } 
    // std::cout << std::endl << std::flush;

    PennChordMessage pcm;
    pcm.Deserialize(b);

    //Add in thread pool once this actually works TODO
    unsigned int* addrPtr = (unsigned int*) malloc(sizeof(unsigned int));
    ipStringToNumber(inet_ntoa(cli_addr.sin_addr), addrPtr);
    static_cast<PennChord*>(args)->RecvMessage(pcm, Ipv4Address(static_cast<uint32_t>(*addrPtr)));
  }

  return NULL;
}

int
PennChord::GetAppPort() {
  return m_appPort;
}

std::map<std::string, pthread_t> 
PennChord::StartApplication (std::map<uint32_t, Ipv4Address> m_nodeAddressMap, std::map<Ipv4Address, uint32_t> m_addressNodeMap,  Ipv4Address m_local, std::string nodeId)
{
  SetNodeAddressMap(m_nodeAddressMap);
  SetAddressNodeMap(m_addressNodeMap);
  SetLocalAddress(m_local);
  g_nodeId = nodeId;
  m_appPort = 3000;

  std::cout << "PennChord::StartApplication()!!!!!" << std::endl;
  std::cout << "Node: " << g_nodeId << ", Hash: " << PennKeyHelper::KeyToHexString(PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(g_nodeId))), m_addressNodeMap)) << std::endl;

  pthread_mutexattr_t lockAttr;
  initMutexAndAttr(lock, lockAttr);

  std::map<std::string, pthread_t> threadMap;

  //Stabilize Periodic Thread
  pthread_t stabilize_thread;
  pthread_create(&stabilize_thread, NULL, StabilizeThread, this);
  threadMap.insert({"stabilize_thread", stabilize_thread});

  //Fix fingers periodic thread
  pthread_t fix_fingers_thread;
  pthread_create(&fix_fingers_thread, NULL, FixFingersThread, this);
  threadMap.insert({"fix_fingers_thread", fix_fingers_thread});

  pthread_t command_line_thread;
  pthread_create(&command_line_thread, NULL, CommandLineThread, this);
  threadMap.insert({"command_line_thread", command_line_thread});

  pthread_t receive_thread;
  pthread_create(&receive_thread, NULL, ReceiveThread, this);
  threadMap.insert({"receive_thread", receive_thread});

  return threadMap;
}

void
PennChord::StopApplication (void) {};

void
PennChord::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;

  for (std::string& s : tokens) {
    std::cout << " " << s;
  }

  std::cout << std::endl << std::flush;
  
  if (command == "JOIN") {
    if (tokens.size() < 2) {
      ERROR_LOG("Insufficient Parameters for JOIN!");
      return;
    }

    // std::cout << "Node Id: [" << g_nodeId << "], tokens.at(1) = [" << tokens.at(1) << "]" << std::endl;
    if (g_nodeId == tokens.at(1)) {
      CreateRing(g_nodeId);
    } else {
      Join(tokens.at(1), g_nodeId);
    }
  } else if (command == "LEAVE") {
    Leave();
  } else if (command == "RINGSTATE") {
    RingState();
  }
}

void PennChord::CreateRing(std::string& currNum) {
  if (DEBUG) fprintf(stderr, "\n Created Ring \n");

  pthread_mutex_lock(&lock);

  currNumber = currNum;
  currHash = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(currNum))), m_addressNodeMap);
  currIP = m_nodeAddressMap.at(std::stoi(currNumber));
  
  successorNumber = currNumber;
  successorHash = currHash;
  successorIP = currIP;

  predecessorNumber = "-1";
  predecessorHash =  -1;
  predecessorIP = currIP;

  for (int i = 0; i < 32; i++) {
    m_fingerTable[i] = "-1";
  }

  inRing = true;
  isSingleton = true;

  pthread_mutex_unlock(&lock);
}

void PennChord::Join(std::string& nodeContained, std::string& currNum) {
  
  if (DEBUG) fprintf(stderr, "\n Node %s sent join request to node %s \n", currNum.c_str(), nodeContained.c_str());
  pthread_mutex_lock(&lock);

  predecessorNumber = "-1";
  predecessorHash = -1;

  currNumber = currNum;
  currHash = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(currNum))), m_addressNodeMap);
  currIP = m_nodeAddressMap.at(std::stoi(currNumber));

  successorNumber = "-1";
  successorHash = -1;

  nextFinger = 0;

  for (int i = 0; i < 32; i++) {
    m_fingerTable[i] = "-1";
  }

  tryingToJoin = true;

  PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ);
  findPredReq.SetFindPredReq(currNumber + "," + currNumber + "," + NOT_HASHED_FIELD);

  pthread_mutex_unlock(&lock);

  sendTo(findPredReq, m_appPort, m_nodeAddressMap.at(std::stoi(nodeContained)));
}



void PennChord::FixFingers() {
  pthread_mutex_lock(&lock);
  if (!inRing || isSingleton || !haveNotifiedOnce) {
    pthread_mutex_unlock(&lock);
    return; //Ensures node fields are initialized before calling fix fingers code.
  }

  fixingFingers = true;

  pthread_mutex_unlock(&lock);
  
  FindSuccessor(currHash + pow(2, nextFinger));

  //Increment of nextFinger happens in processFindPredReq()
}

void PennChord::FindSuccessor(uint32_t hashOfNode) {
  pthread_mutex_lock(&lock);
  fixingFingers = true;

  PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ);
  findPredReq.SetFindPredReq(currNumber + "," + std::to_string(hashOfNode) + "," + HASHED_FIELD); //0 because we are giving it the hashed value

  std::string closest_finger = ClosestPrecedingNode(hashOfNode);
  uint32_t closest_finger_number = static_cast<uint32_t>(std::stoul(closest_finger));

  pthread_mutex_unlock(&lock);

  sendTo(findPredReq, m_appPort, m_nodeAddressMap.at(closest_finger_number));
}

void PennChord::Stabilize() {
  // (1) find your successor's pred, x
  // (2) if x is a better successor for you, make that your succ
  // (3) notify (new) successor
  pthread_mutex_lock(&lock);
  if (inRing && successorNumber != currNumber && predecessorNumber != currNumber) { //Ring consists of more than one node and both nodes' fields are initialized 
    // FIND PRED REQUEST FOR YOUR SUCCESSOR'S PRED
    tryingToJoin = false;
    Ipv4Address successorIPCopy = successorIP;

    pthread_mutex_unlock(&lock);

    PennChordMessage stabilizeReq = PennChordMessage(PennChordMessage::MessageType::STABILIZE_REQ);
    stabilizeReq.SetStabilizeReq("");

    sendTo(stabilizeReq, m_appPort, successorIPCopy);

    return;
  }

  pthread_mutex_unlock(&lock);

}

void PennChord::ProcessStabilizeReq(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  pthread_mutex_lock(&lock);
  std::string fromNode = ReverseLookup(sourceAddress);
  std::string stabilizeMessage = message.GetStabilizeReq().stabilizeMessage;
  if (DEBUG) CHORD_LOG ("Received STABILIZE_REQ, From Node: " << fromNode << ", Message: " << stabilizeMessage);

  PennChordMessage stabilizeRsp = PennChordMessage(PennChordMessage::MessageType::STABILIZE_RSP);
  stabilizeRsp.SetStabilizeRsp(predecessorNumber);

  pthread_mutex_unlock(&lock);

  sendTo(stabilizeRsp, m_appPort, sourceAddress);
}


void PennChord::ProcessStabilizeRsp(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup(sourceAddress);
  std::string stabilizeMessage = message.GetStabilizeRsp().stabilizeMessage;
  if (DEBUG) CHORD_LOG ("Received STABILIZE_RSP, From Node: " << fromNode << ", Message: " << stabilizeMessage);

  std::string msg = message.GetStabilizeRsp().stabilizeMessage;
  int successor_predecessor_num = std::stoi(msg);
  uint32_t suc_pred_hash = (successor_predecessor_num == -1) ? 0 : PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(successor_predecessor_num)), m_addressNodeMap);

  pthread_mutex_lock(&lock);
  if ((successor_predecessor_num != -1) && ((successorHash > currHash && suc_pred_hash > currHash && suc_pred_hash < successorHash) || (successorHash < currHash && (suc_pred_hash > currHash || suc_pred_hash < successorHash)))) {
    successorNumber = msg;
    successorHash = suc_pred_hash;
    successorIP = m_nodeAddressMap.at(successor_predecessor_num);

   if (DEBUG) fprintf(stderr, "\n Node %s updated its successor due to stabilize. New Info: Node = %s, Successor = %s, Predecessor = %s \n", currNumber.c_str(), currNumber.c_str(), successorNumber.c_str(), predecessorNumber.c_str());
  } 

  pthread_mutex_unlock(&lock);

  Notify();
}

void PennChord::Notify() {
  pthread_mutex_lock(&lock);
  haveNotifiedOnce = true;
  Ipv4Address successorIPCopy = successorIP;
  pthread_mutex_unlock(&lock);

  PennChordMessage notify = PennChordMessage(PennChordMessage::MessageType::NOTIFY);
  notify.SetNotify("");

  sendTo(notify, m_appPort, successorIPCopy);
  
}

void PennChord::ProcessNotify(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup(sourceAddress);
  uint32_t fromNodeHash = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(fromNode))), m_addressNodeMap);

  pthread_mutex_lock(&lock);
  if (DEBUG) fprintf(stderr, "\n Node %s sent notify to node %s \n", fromNode.c_str(), g_nodeId.c_str());
  bool inRange = (currHash > predecessorHash && fromNodeHash > predecessorHash && fromNodeHash < currHash) || (currHash < predecessorHash && (fromNodeHash > predecessorHash || fromNodeHash < currHash));
  if (predecessorNumber == "-1" || inRange) {
    predecessorNumber = fromNode;
    predecessorHash = fromNodeHash;
    predecessorIP = m_nodeAddressMap.at(std::stoi(fromNode)); 

    m_rehashKeys(sourceAddress, ""); //parameters don't get used

    if (DEBUG) fprintf(stderr, "\n Node %s sent notify to node %s. In Range = %u, PredHash = %u , CurrHash = %u , FromHash = %u, SuccHash = %u, Updates to predecessor made! Node: %s, Predecessor: %s, Successor %s \n",
     fromNode.c_str(), g_nodeId.c_str(), inRange, predecessorHash, currHash, fromNodeHash, successorHash, g_nodeId.c_str(), predecessorNumber.c_str(), successorNumber.c_str());
    if (isSingleton) {
      //Another node has joined the ring so if this was a singleton node it is no longer one.
      successorNumber = predecessorNumber;
      successorHash = predecessorHash;
      successorIP = predecessorIP;
      isSingleton = false;
      if (DEBUG) fprintf(stderr, "\n Node %s sent notify to node %s. Singleton update made! Node: %s, Predecessor: %s, Successor %s \n", fromNode.c_str(), g_nodeId.c_str(), g_nodeId.c_str(), predecessorNumber.c_str(), successorNumber.c_str());
    }
  } 

  pthread_mutex_unlock(&lock);
  
}

void PennChord::Lookup(uint32_t id_hash) {
    pthread_mutex_lock(&lock);
    CHORD_LOG("LookupIssue<" << std::to_string(currHash) << ", " << std::to_string(id_hash) << ">");

    PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ);
    findPredReq.SetFindPredReq(currNumber + "," + std::to_string(id_hash) + "," + HASHED_FIELD + "," + (makingSearchQuery ? SEARCH_QUERY : PUBLISH_QUERY) + ",0"); //The 0 if for the initial hop count size.

    std::string closestNodeString = ClosestPrecedingNode(id_hash);
    Ipv4Address nextToSendTo = m_nodeAddressMap.at(static_cast<uint32_t>(std::stoul(closestNodeString)));
    pthread_mutex_unlock(&lock);

    sendTo(findPredReq, m_appPort, nextToSendTo);

    totalLookups++; //Update total lookups
}


void PennChord::ProcessFindPredRsp(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string findPredMessage = message.GetFindPredRsp().findPredMessage;

  pthread_mutex_lock(&lock);
  if (DEBUG) CHORD_LOG ("Received FIND_PRED_RSP, From Node: " << fromNode << ", Message: " << findPredMessage << ", Trying to Join: " << tryingToJoin);

  std::vector<std::string> v;

  std::stringstream ss(findPredMessage);
 
  while (ss.good()) {
      std::string substr;
      std::getline(ss, substr, ',');
      v.push_back(substr);
  }

  bool pennSearchRequest = false;
  bool pennPublishRequest = false;
  if (v.size() >= 4 && v.at(3) == SEARCH_QUERY) {
    // CHORD_LOG ("Received FIND_PRED_RSP for SEARCH_QUERY, From Node: " << fromNode << ", Message: " << findPredMessage);
    pennSearchRequest = true;
  } else if (v.size() >= 4 && v.at(3) == PUBLISH_QUERY) {
    // CHORD_LOG ("Received FIND_PRED_RSP for PUBLISH_QUERY, From Node: " << fromNode << ", Message: " << findPredMessage);
    pennPublishRequest = true;
  }

  
  if (pennPublishRequest) {
    // LookupResult<currentNodeKey, targetKey, originatorNodeNum, originatorNodeKey>
    // CHORD_LOG("LookupResult<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ", " << fromNode  << ", " << PennKeyHelper::CreateShaKey(fromNode) << ">");
    m_publishFn(sourceAddress, findPredMessage);
    totalHops += std::stoi(v.at(5)); //Update hop count
  } else if (pennSearchRequest) {
    // CHORD_LOG("LookupResult<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ", " << fromNode  << ", " << PennKeyHelper::CreateShaKey(fromNode) << ">");
    m_searchFn(sourceAddress, findPredMessage);
    totalHops += std::stoi(v.at(5)); //Update hop count
  } else if (tryingToJoin) {
    successorNumber = v.at(1); //The successor of the predecessor is the successor of our node now.
    successorIP = m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(successorNumber))); 
    successorHash = PennKeyHelper::CreateShaKey(successorIP, m_addressNodeMap);
   if (DEBUG) fprintf(stderr, "\n Node %s received tryingToJoin response with successor as %s \n", g_nodeId.c_str(), successorNumber.c_str());

    tryingToJoin = false; 
    inRing = true;
  } else if (fixingFingers) {
    m_fingerTable[nextFinger] = v.at(1); //The successor of the predecessor is the successor. 

    if (DEBUG) fprintf(stderr, "\n Node %s received fixingFingers response with nextFinger = %u and value %s \n", g_nodeId.c_str(), nextFinger, v.at(1).c_str());
    fixingFingers = false;
    nextFinger = nextFinger + 1;
    if(nextFinger > 31) {
      nextFinger = 0;
    }
    
  }

  pthread_mutex_unlock(&lock);
  
}

void PennChord::ProcessFindPredReq(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string findPredMessage = message.GetFindPredReq().findPredMessage;

  pthread_mutex_lock(&lock);
  if (DEBUG) CHORD_LOG ("Received FIND_PRED_REQ, From Node: " << fromNode << ", Message: " << findPredMessage << ", Trying to Join: " << tryingToJoin);

  std::vector<std::string> v;
  
  std::stringstream ss(findPredMessage);

  while (ss.good()) {
      std::string substr;
      std::getline(ss, substr, ',');
      v.push_back(substr);
  }

  //4th comma separated value should be equal to 1 for penn search request
  bool pennSearchRequest = false;
  bool pennPublishRequest = false;
  if (v.size() >= 5 && v.at(3) == SEARCH_QUERY) {
    //CHORD_LOG ("Received FIND_PRED_REQ for SEARCH_QUERY, From Node: " << fromNode << ", Message: " << findPredMessage);
    pennSearchRequest = true;
  } else if (v.size() >= 5 && v.at(3) == PUBLISH_QUERY) {
    //CHORD_LOG ("Received FIND_PRED_REQ for PUBLISH_QUERY, From Node: " << fromNode << ", Message: " << findPredMessage);
    pennPublishRequest = true;
  }



  int requesterNode = std::stoi(v.at(0));
  uint32_t hashOfNode = strcmp(v.at(2).c_str(), NOT_HASHED_FIELD) == 0 ? PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(v.at(1)))), m_addressNodeMap) : static_cast<uint32_t>(std::stoul(v.at(1))); //
  
  if (DEBUG) fprintf(stderr, "\tSuccHash: %u, CurrHash: %u, targHash: %u\n", successorHash, currHash, hashOfNode);

  // you're at the final node (send response back to original requester)
  if (isSingleton || ((successorHash > currHash && hashOfNode > currHash && hashOfNode <= successorHash) || (successorHash <= currHash && (hashOfNode > currHash || hashOfNode <= successorHash)))) {

    PennChordMessage findPredRsp = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_RSP);

    std::string rsp = currNumber + "," + successorNumber + "," + predecessorNumber; 
    if (pennSearchRequest) {
      rsp += "," + std::string(SEARCH_QUERY); //Indicates to response parser that this is a SEARCH_QUERY
      rsp += "," + std::to_string(hashOfNode); //The original lookupQuery
      rsp += "," + std::to_string(std::stoi(v.at(4)) + 1); //The total hop count
      // LookupResult<currentNodeKey, targetKey, originatorNodeNum, originatorNodeKey>
      CHORD_LOG("LookupResult<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ", " << fromNode  << ", " << PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(fromNode))), m_addressNodeMap) << ">");
    } else if (pennPublishRequest) {
      rsp += "," + std::string(PUBLISH_QUERY); //Indicates to response parser that this is a PUBLISH_QUERY
      rsp += "," + std::to_string(hashOfNode); //The original lookupQuery
      rsp += "," + std::to_string(std::stoi(v.at(4)) + 1); //The total hop count
      // LookupResult<currentNodeKey, targetKey, originatorNodeNum, originatorNodeKey>
      CHORD_LOG("LookupResult<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ", " << fromNode  << ", " << PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(fromNode))), m_addressNodeMap) << ">");
    }

    findPredRsp.SetFindPredRsp(rsp);

    if (DEBUG) fprintf(stderr, "\t FIND_PRED_Request RSP message: %s. SuccHash: %u, CurrHash: %u, targHash: %u\n", findPredRsp.GetFindPredRsp().findPredMessage.c_str(), successorHash, currHash, hashOfNode);
    // CHORD_LOG("LookupIssue<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ">");
    Ipv4Address requesterNodeIP = m_nodeAddressMap.at(requesterNode);

    pthread_mutex_unlock(&lock);

    sendTo(findPredRsp, m_appPort, requesterNodeIP);

  // you're a forwarding node (send response to closest finger)
  } else {
  
    PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ);

    std::string msgString;
    if (pennPublishRequest || pennSearchRequest) {
      for (size_t i = 0; i < v.size(); i++) {
        if (i == 4) {
          msgString +=  std::to_string(std::stoi(v.at(4)) + 1);
          break;
        } else {
          msgString += v.at(i) + ",";
        }
      }
    } else msgString = findPredMessage;

    findPredReq.SetFindPredReq(msgString); //The total hop count);

    std::string closestFingerString = ClosestPrecedingNode(hashOfNode);
    Ipv4Address nextToSendTo = m_nodeAddressMap.at(static_cast<uint32_t>(std::stoul(closestFingerString)));
    if (DEBUG) fprintf(stderr, "\n Forwarding FIND_PRED_REQ to %s \n", closestFingerString.c_str());

     // DEBUG statement should only print for lookup requests, not while fixing fingers 
    if (pennSearchRequest || pennPublishRequest) {
      CHORD_LOG("LookupRequest<" << std::to_string(currHash) << ">: NextHop<" << closestFingerString << ", " << PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(closestFingerString))), m_addressNodeMap) << ", " << std::to_string(hashOfNode) << ">");
    }
    
    pthread_mutex_unlock(&lock);
    
    sendTo(findPredReq, m_appPort, nextToSendTo);
  }
}


// Used by PennSearch for rehashing files to send to successor via PUBLISH messages
uint32_t PennChord::getSuccessorHash() {
  return successorHash;
}

uint32_t PennChord::getPredecessorHash() {
  return predecessorHash;
}

std::string PennChord::getSuccessorNode() {
  return successorNumber;
}

std::string PennChord::getPredecessorNode() {
  return predecessorNumber;
}

// If you are receiving this message, your successor left the ring -- update successor accordingly (you should receive your new successor in the message)
void PennChord::ProcessLeaveP(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string newSuccessorNum = message.GetLeaveP().leavePMessage;
  uint32_t hashOfNode = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(newSuccessorNum))), m_addressNodeMap);
  
  pthread_mutex_lock(&lock);
  successorNumber = newSuccessorNum;
  successorHash = hashOfNode;
  successorIP = m_nodeAddressMap.at(std::stoi(successorNumber)); 
  pthread_mutex_unlock(&lock);
}

// If you are receiving this message, your predecessor left the ring -- update successor accordingly (you should receive your new pred in the message)
void PennChord::ProcessLeaveS(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string newPredecessorNum = message.GetLeaveS().leaveSMessage;
  uint32_t hashOfNode = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(newPredecessorNum))), m_addressNodeMap);
  
  pthread_mutex_lock(&lock);
  predecessorNumber = newPredecessorNum;
  predecessorHash = hashOfNode;
  predecessorIP = m_nodeAddressMap.at(std::stoi(predecessorNumber));
  pthread_mutex_unlock(&lock);
}

// Called when a node voluntarily leaves
// Need to (1) transfer keys to successor MS2 (2) notif pred (3) notif success
void PennChord::Leave() {

  pthread_mutex_lock(&lock);
  m_leaveFn(currIP, "");
  // message to pred contain's current node's successor 
  inRing = false;


  PennChordMessage leaveP = PennChordMessage(PennChordMessage::MessageType::LEAVE_P);
  leaveP.SetLeaveP(successorNumber);
  Ipv4Address predecessorIPCopy = predecessorIP;
  pthread_mutex_unlock(&lock);
  sendTo(leaveP, m_appPort, predecessorIPCopy);

  // message to successor contain's current node's pred 
  PennChordMessage leaveS = PennChordMessage(PennChordMessage::MessageType::LEAVE_S);
  pthread_mutex_lock(&lock);
  leaveS.SetLeaveS(predecessorNumber);
  Ipv4Address successorIPCopy = successorIP;
  pthread_mutex_unlock(&lock);

  sendTo(leaveS, m_appPort, successorIPCopy);
}

// Called when a RINGSTATE command is issued
void PennChord::RingState() {
  // Send out a message to successor
  PennChordMessage ringStateMsg = PennChordMessage(PennChordMessage::MessageType::RING_STATE);
  pthread_mutex_lock(&lock);

  ringStateMsg.SetRingState(currNumber);
  Ipv4Address successorIPCopy = successorIP;

  pthread_mutex_unlock(&lock);

  // PRINT_LOG("Sending to " << successorIP << " Node = " << successorNumber);
  sendTo(ringStateMsg, m_appPort, successorIPCopy);
}

void PennChord::ProcessRingState(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // PRINT_LOG("CurrNumber = " << currNumber << " ringStateMessage = " << message.GetRingState().ringStateMessage);
  // Print out the ring state at the current node
  pthread_mutex_lock(&lock);
  PRINT_LOG("Ring State\n"
            << "\tCurr<Node " << currNumber << ", " << currIP << ", " << PennKeyHelper::KeyToHexString(currHash)<< ">\n"
            << "\tPred<Node " << predecessorNumber << ", " << predecessorIP << ", " << PennKeyHelper::KeyToHexString(predecessorHash) << ">\n"
            << "\tSucc<Node " << successorNumber << ", " << successorIP << ", " << PennKeyHelper::KeyToHexString(successorHash) << ">\n") 
  // for (int i = 0; i < 32; i++) {
  //   std::cerr << "Node: " << g_nodeId << ". m_fingerTable[" << i << "] = " << m_fingerTable[i] << std::endl;
  // }
  // If not originator, send RING_STATE message to successor
  if (currNumber != message.GetRingState().ringStateMessage) {
    PennChordMessage ringStateMsg = PennChordMessage(PennChordMessage::MessageType::RING_STATE);
    ringStateMsg.SetRingState(message.GetRingState().ringStateMessage);
    // PRINT_LOG("Sending to " << successorIP << " Node = " << successorNumber);
    Ipv4Address successorIPCopy = successorIP;
    pthread_mutex_unlock(&lock);

    sendTo(ringStateMsg, m_appPort, successorIPCopy);
  } else {
    // If originator, print End of Ring State message
    pthread_mutex_unlock(&lock);
    PRINT_LOG("End of Ring State");
  }
}


void
PennChord::RecvMessage (PennChordMessage message, Ipv4Address sourceAddress)
{
  uint16_t sourcePort = m_appPort; //Not actually used by any of the methods below
  std::cout << "Received message of type " << message.GetMessageType() << ", from: " << sourceAddress.Ipv4ToString() << std::endl << std::flush;

  switch (message.GetMessageType ())
    {
      case PennChordMessage::RING_STATE:
        ProcessRingState (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::FIND_PRED_RSP:
        ProcessFindPredRsp(message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::FIND_PRED_REQ:
        ProcessFindPredReq(message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::LEAVE_P:
        ProcessLeaveP(message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::LEAVE_S:
        ProcessLeaveS(message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::NOTIFY:
        ProcessNotify(message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::STABILIZE_REQ:
        ProcessStabilizeReq(message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::STABILIZE_RSP:
        ProcessStabilizeRsp(message, sourceAddress, sourcePort);
        break;
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}


// Finger table implementation
std::string PennChord::ClosestPrecedingNode(uint32_t id) {
  for (int i = 31; i >= 0; i--) {
    // If the finger table entry is nil, skip to next entry
    if (m_fingerTable[i] == "-1") {
      continue;
    }

    uint32_t f = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(m_fingerTable[i]))), m_addressNodeMap);
    uint32_t n = currHash;


    // Check if the current finger is the closest preceding 
    // finger of the node being searched for
    if ((id > n && f > n && f < id) || (id < n && (f > n || f < id))) {
      return m_fingerTable[i];
    }
  }

  // If no finger was the closest preceding finger, return 
  // the successor node as default behavior
  return (isSingleton ? currNumber : successorNumber);
}


void
PennChord::StopChord ()
{
  StopApplication ();
}


// Following 2 functions are custom callbacks we need for PennSearch
void
PennChord::SetPublishCallback (Callback publishFn)
{
  m_publishFn = publishFn;
}

void
PennChord::SetSearchCallback (Callback searchFn)
{
  m_searchFn = searchFn;
}

void
PennChord::SetLeaveCallback (Callback leaveFn)
{
  m_leaveFn = leaveFn;
}

void PennChord::SetRehashKeysCallback (Callback rehashKeysFn) {
  m_rehashKeys = rehashKeysFn;
}