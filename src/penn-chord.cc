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


#define DEBUG 0

using namespace ns3;

PennChord::PennChord ()
    :
{

}

PennChord::~PennChord ()
{

}

void
PennChord::StartApplication (std::map<uint32_t, Ipv4Address> m_nodeAddressMap, std::map<Ipv4Address, uint32_t> m_addressNodeMap,  Ipv4Address m_local, std::string nodeId)
{
  std::cout << "PennChord::StartApplication()!!!!!" << std::endl;
  std::cout << "Node: " << g_nodeId << ", Hash: " << PennKeyHelper::KeyToHexString(PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(g_nodeId))))) << std::endl;
  
  // Configure timers
  m_stabilizeTimer.SetFunction (&PennChord::Stabilize, this);
  m_fixFingersTimer.SetFunction (&PennChord::FixFingers, this);
  // Start timers
  
  std::string m_stabilizeTimeout {"1000ms"};
  std::string m_fixFingersTimeout {"100ms"};
  m_stabilizeTimer.SetDelay(Time(m_stabilizeTimeout));
  m_fixFingersTimer.SetDelay(Time(m_fixFingersTimeout));
  m_stabilizeTimer.Schedule ();
  m_fixFingersTimer.Schedule();

}

void
PennChord::StopApplication (void)
{

}

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

  currNumber = currNum;
  currHash = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(currNum))));
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
}

void PennChord::Join(std::string& nodeContained, std::string& currNum) {
  if (DEBUG) fprintf(stderr, "\n Node %s sent join request to node %s \n", currNum.c_str(), nodeContained.c_str());

  predecessorNumber = "-1";
  predecessorHash = -1;

  currNumber = currNum;
  currHash = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(currNum))));
  currIP = m_nodeAddressMap.at(std::stoi(currNumber));

  successorNumber = "-1";
  successorHash = -1;

  nextFinger = 0;

  for (int i = 0; i < 32; i++) {
    m_fingerTable[i] = "-1";
  }

  tryingToJoin = true;
  Ptr<Packet> packet = Create<Packet>();
  PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ, GetNextTransactionId());
  findPredReq.SetFindPredReq(currNumber + "," + currNumber + "," + NOT_HASHED_FIELD);
  packet->AddHeader(findPredReq);
  m_socket->SendTo (packet, 0, InetSocketAddress (m_nodeAddressMap.at(std::stoi(nodeContained)), m_appPort));
}

void PennChord::FixFingers() {

  if (!inRing || isSingleton || !haveNotifiedOnce) {
    m_fixFingersTimer.Schedule();
    return; //Ensures node fields are initialized before calling fix fingers code.
  }

  fixingFingers = true;
  
  FindSuccessor(currHash + pow(2, nextFinger));

  //Increment of nextFinger happens in processFindPredReq()
}

void PennChord::FindSuccessor(uint32_t hashOfNode) {
  fixingFingers = true;
  Ptr<Packet> packet = Create<Packet>();
  PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ, GetNextTransactionId());
  findPredReq.SetFindPredReq(currNumber + "," + std::to_string(hashOfNode) + "," + HASHED_FIELD); //0 because we are giving it the hashed value
  packet->AddHeader(findPredReq);

  std::string closest_finger = ClosestPrecedingNode(hashOfNode);
  uint32_t closest_finger_number = static_cast<uint32_t>(std::stoul(closest_finger));
  m_socket->SendTo (packet, 0, InetSocketAddress (m_nodeAddressMap.at(closest_finger_number), m_appPort)); 
}

void PennChord::Stabilize() {
  // (1) find your successor's pred, x
  // (2) if x is a better successor for you, make that your succ
  // (3) notify (new) successor

  if (inRing && successorNumber != currNumber && predecessorNumber != currNumber) { //Ring consists of more than one node and both nodes' fields are initialized 
    // FIND PRED REQUEST FOR YOUR SUCCESSOR'S PRED
    tryingToJoin = false;
    Ptr<Packet> packet = Create<Packet>();
    PennChordMessage stabilizeReq = PennChordMessage(PennChordMessage::MessageType::STABILIZE_REQ, GetNextTransactionId());
    stabilizeReq.SetStabilizeReq("");
    packet->AddHeader(stabilizeReq);
    m_socket->SendTo (packet, 0, InetSocketAddress (successorIP, m_appPort));
  }

  m_stabilizeTimer.Schedule();
}

void PennChord::ProcessStabilizeReq(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup(sourceAddress);
  std::string stabilizeMessage = message.GetStabilizeReq().stabilizeMessage;
  if (DEBUG) CHORD_LOG ("Received STABILIZE_REQ, From Node: " << fromNode << ", Message: " << stabilizeMessage);
  Ptr<Packet> packet = Create<Packet>();
  PennChordMessage stabilizeRsp = PennChordMessage(PennChordMessage::MessageType::STABILIZE_RSP, GetNextTransactionId());
  stabilizeRsp.SetStabilizeRsp(predecessorNumber);
  packet->AddHeader(stabilizeRsp);
  m_socket->SendTo (packet, 0, InetSocketAddress (sourceAddress, m_appPort));
}


void PennChord::ProcessStabilizeRsp(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup(sourceAddress);
  std::string stabilizeMessage = message.GetStabilizeRsp().stabilizeMessage;
  if (DEBUG) CHORD_LOG ("Received STABILIZE_RSP, From Node: " << fromNode << ", Message: " << stabilizeMessage);

  std::string msg = message.GetStabilizeRsp().stabilizeMessage;
  int successor_predecessor_num = std::stoi(msg);
  uint32_t suc_pred_hash = (successor_predecessor_num == -1) ? 0 : PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(successor_predecessor_num)));

  if ((successor_predecessor_num != -1) && ((successorHash > currHash && suc_pred_hash > currHash && suc_pred_hash < successorHash) || (successorHash < currHash && (suc_pred_hash > currHash || suc_pred_hash < successorHash)))) {
    successorNumber = msg;
    successorHash = suc_pred_hash;
    successorIP = m_nodeAddressMap.at(successor_predecessor_num);

   if (DEBUG) fprintf(stderr, "\n Node %s updated its successor due to stabilize. New Info: Node = %s, Successor = %s, Predecessor = %s \n", currNumber.c_str(), currNumber.c_str(), successorNumber.c_str(), predecessorNumber.c_str());
  } 

  Notify();
}

void PennChord::Notify() {
  haveNotifiedOnce = true;

  Ptr<Packet> packet = Create<Packet>();
  PennChordMessage notify = PennChordMessage(PennChordMessage::MessageType::NOTIFY, GetNextTransactionId());
  notify.SetNotify("");
  packet->AddHeader(notify);
  m_socket->SendTo (packet, 0, InetSocketAddress (successorIP, m_appPort));
  
}

void PennChord::ProcessNotify(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup(sourceAddress);
  uint32_t fromNodeHash = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(fromNode))));

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
  
}

void PennChord::Lookup(uint32_t id_hash) {
    CHORD_LOG("LookupIssue<" << std::to_string(currHash) << ", " << std::to_string(id_hash) << ">");
    Ptr<Packet> packet = Create<Packet>();
    PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ, GetNextTransactionId());
    findPredReq.SetFindPredReq(currNumber + "," + std::to_string(id_hash) + "," + HASHED_FIELD + "," + (makingSearchQuery ? SEARCH_QUERY : PUBLISH_QUERY) + ",0"); //The 0 if for the initial hop count size.
    packet->AddHeader(findPredReq);
    std::string closestNodeString = ClosestPrecedingNode(id_hash);
    Ipv4Address nextToSendTo = m_nodeAddressMap.at(static_cast<uint32_t>(std::stoul(closestNodeString)));
    m_socket->SendTo (packet, 0, InetSocketAddress (nextToSendTo, m_appPort));

    totalLookups++; //Update total lookups
}


void PennChord::ProcessFindPredRsp(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string findPredMessage = message.GetFindPredRsp().findPredMessage;
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
    successorHash = PennKeyHelper::CreateShaKey(successorIP);
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
    m_fixFingersTimer.Schedule();
    
  }
  
}

void PennChord::ProcessFindPredReq(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string findPredMessage = message.GetFindPredReq().findPredMessage;
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
  uint32_t hashOfNode = strcmp(v.at(2).c_str(), NOT_HASHED_FIELD) == 0 ? PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(v.at(1))))) : static_cast<uint32_t>(std::stoul(v.at(1))); //
  
  if (DEBUG) fprintf(stderr, "\tSuccHash: %u, CurrHash: %u, targHash: %u\n", successorHash, currHash, hashOfNode);

  // you're at the final node (send response back to original requester)
  if (isSingleton || ((successorHash > currHash && hashOfNode > currHash && hashOfNode <= successorHash) || (successorHash <= currHash && (hashOfNode > currHash || hashOfNode <= successorHash)))) {
    Ptr<Packet> packet = Create<Packet>();
    PennChordMessage findPredRsp = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_RSP, GetNextTransactionId());

    std::string rsp = currNumber + "," + successorNumber + "," + predecessorNumber; 
    if (pennSearchRequest) {
      rsp += "," + std::string(SEARCH_QUERY); //Indicates to response parser that this is a SEARCH_QUERY
      rsp += "," + std::to_string(hashOfNode); //The original lookupQuery
      rsp += "," + std::to_string(std::stoi(v.at(4)) + 1); //The total hop count
      // LookupResult<currentNodeKey, targetKey, originatorNodeNum, originatorNodeKey>
      CHORD_LOG("LookupResult<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ", " << fromNode  << ", " << PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(fromNode)))) << ">");
    } else if (pennPublishRequest) {
      rsp += "," + std::string(PUBLISH_QUERY); //Indicates to response parser that this is a PUBLISH_QUERY
      rsp += "," + std::to_string(hashOfNode); //The original lookupQuery
      rsp += "," + std::to_string(std::stoi(v.at(4)) + 1); //The total hop count
      // LookupResult<currentNodeKey, targetKey, originatorNodeNum, originatorNodeKey>
      CHORD_LOG("LookupResult<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ", " << fromNode  << ", " << PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(fromNode)))) << ">");
    }

    findPredRsp.SetFindPredRsp(rsp);
    packet->AddHeader(findPredRsp);
    if (DEBUG) fprintf(stderr, "\t FIND_PRED_Request RSP message: %s. SuccHash: %u, CurrHash: %u, targHash: %u\n", findPredRsp.GetFindPredRsp().findPredMessage.c_str(), successorHash, currHash, hashOfNode);
    // CHORD_LOG("LookupIssue<" << std::to_string(currHash) << ", " << std::to_string(hashOfNode) << ">");
    Ipv4Address requesterNodeIP = m_nodeAddressMap.at(requesterNode);


    m_socket->SendTo (packet, 0, InetSocketAddress (requesterNodeIP, m_appPort));

  // you're a forwarding node (send response to closest finger)
  } else {
  

    Ptr<Packet> packet = Create<Packet>();
    PennChordMessage findPredReq = PennChordMessage(PennChordMessage::MessageType::FIND_PRED_REQ, GetNextTransactionId());

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
    packet->AddHeader(findPredReq);

    std::string closestFingerString = ClosestPrecedingNode(hashOfNode);
    Ipv4Address nextToSendTo = m_nodeAddressMap.at(static_cast<uint32_t>(std::stoul(closestFingerString)));
     if (DEBUG) fprintf(stderr, "\n Forwarding FIND_PRED_REQ to %s \n", closestFingerString.c_str());

     // DEBUG statement should only print for lookup requests, not while fixing fingers 
     if(pennSearchRequest || pennPublishRequest) {
      CHORD_LOG("LookupRequest<" << std::to_string(currHash) << ">: NextHop<" << closestFingerString << ", " << PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(closestFingerString)))) << ", " << std::to_string(hashOfNode) << ">");
     }
    
    m_socket->SendTo (packet, 0, InetSocketAddress (nextToSendTo, m_appPort));
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

// If you are receiving this message, your successer left the ring -- update successor accordingly (you should receive your new successor in the message)
void PennChord::ProcessLeaveP(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string newSuccessorNum = message.GetLeaveP().leavePMessage;
  uint32_t hashOfNode = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(newSuccessorNum))));
  successorNumber = newSuccessorNum;
  successorHash = hashOfNode;
  successorIP = m_nodeAddressMap.at(std::stoi(successorNumber)); 
}

// If you are receiving this message, your predecessor left the ring -- update successor accordingly (you should receive your new pred in the message)
void PennChord::ProcessLeaveS(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  std::string newPredecessorNum = message.GetLeaveS().leaveSMessage;
  uint32_t hashOfNode = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(newPredecessorNum))));
  predecessorNumber = newPredecessorNum;
  predecessorHash = hashOfNode;
  predecessorIP = m_nodeAddressMap.at(std::stoi(predecessorNumber));
}

// Called when a node voluntarily leaves
// Need to (1) transfer keys to successor MS2 (2) notif pred (3) notif success
void PennChord::Leave() {

  m_leaveFn(currIP, "");
  // message to pred contain's current node's successor 
  inRing = false;
  Ptr<Packet> packetP = Create<Packet>();
  PennChordMessage leaveP = PennChordMessage(PennChordMessage::MessageType::LEAVE_P, GetNextTransactionId());
  leaveP.SetLeaveP(successorNumber);
  packetP->AddHeader(leaveP);
  m_socket->SendTo (packetP, 0 , InetSocketAddress (predecessorIP, m_appPort));

  // message to successor contain's current node's pred 
  Ptr<Packet> packetS = Create<Packet>();
  PennChordMessage leaveS = PennChordMessage(PennChordMessage::MessageType::LEAVE_S, GetNextTransactionId());
  leaveS.SetLeaveS(predecessorNumber);
  packetS->AddHeader(leaveS);
  m_socket->SendTo (packetS, 0 , InetSocketAddress (successorIP, m_appPort));
}

// Called when a RINGSTATE command is issued
void PennChord::RingState() {
  // Send out a message to successor
  uint32_t transactionId = GetNextTransactionId ();
  Ptr<Packet> packet = Create<Packet>();
  PennChordMessage ringStateMsg = PennChordMessage(PennChordMessage::MessageType::RING_STATE, transactionId);
  ringStateMsg.SetRingState(currNumber);
  packet->AddHeader(ringStateMsg);
  // PRINT_LOG("Sending to " << successorIP << " Node = " << successorNumber);
  m_socket->SendTo (packet, 0, InetSocketAddress (successorIP, m_appPort));
}

void PennChord::ProcessRingState(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // PRINT_LOG("CurrNumber = " << currNumber << " ringStateMessage = " << message.GetRingState().ringStateMessage);
  // Print out the ring state at the current node
  PRINT_LOG("Ring State\n"
            << "\tCurr<Node " << currNumber << ", " << currIP << ", " << PennKeyHelper::KeyToHexString(currHash)<< ">\n"
            << "\tPred<Node " << predecessorNumber << ", " << predecessorIP << ", " << PennKeyHelper::KeyToHexString(predecessorHash) << ">\n"
            << "\tSucc<Node " << successorNumber << ", " << successorIP << ", " << PennKeyHelper::KeyToHexString(successorHash) << ">\n") 
  // for (int i = 0; i < 32; i++) {
  //   std::cerr << "Node: " << g_nodeId << ". m_fingerTable[" << i << "] = " << m_fingerTable[i] << std::endl;
  // }
  // If not originator, send RING_STATE message to successor
  if (currNumber != message.GetRingState().ringStateMessage) {
    Ptr<Packet> packet = Create<Packet>();
    PennChordMessage ringStateMsg = PennChordMessage(PennChordMessage::MessageType::RING_STATE, m_currentTransactionId);
    ringStateMsg.SetRingState(message.GetRingState().ringStateMessage);
    packet->AddHeader(ringStateMsg);
    // PRINT_LOG("Sending to " << successorIP << " Node = " << successorNumber);
    m_socket->SendTo (packet, 0, InetSocketAddress (successorIP, m_appPort));
  } else {
    // If originator, print End of Ring State message
    PRINT_LOG("End of Ring State");
  }
}

void
PennChord::SendPing (Ipv4Address destAddress, std::string pingMessage)
{
  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = GetNextTransactionId ();
      if (DEBUG) CHORD_LOG ("Sending PING_REQ to Node: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << pingMessage << " transactionId: " << transactionId);
      Ptr<PingRequest> pingRequest = Create<PingRequest> (transactionId, Simulator::Now(), destAddress, pingMessage);
      // Add to ping-tracker
      m_pingTracker.insert (std::make_pair (transactionId, pingRequest));
      Ptr<Packet> packet = Create<Packet> ();
      PennChordMessage message = PennChordMessage (PennChordMessage::PING_REQ, transactionId);
      message.SetPingReq (pingMessage);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
      
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, pingMessage);
    }
}

void
PennChord::RecvMessage (Ptr<Socket> socket)
{
  Address sourceAddr;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
  Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
  uint16_t sourcePort = inetSocketAddr.GetPort ();
  PennChordMessage message;
  packet->RemoveHeader (message);

  switch (message.GetMessageType ())
    {
      case PennChordMessage::PING_REQ:
        ProcessPingReq (message, sourceAddress, sourcePort);
        break;
      case PennChordMessage::PING_RSP:
        ProcessPingRsp (message, sourceAddress, sourcePort);
        break;
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

    uint32_t f = PennKeyHelper::CreateShaKey(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(m_fingerTable[i]))));
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
PennChord::ProcessPingReq (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
    // Use reverse lookup for ease of debug
    std::string fromNode = ReverseLookup (sourceAddress);
    if (DEBUG) CHORD_LOG ("Received PING_REQ, From Node: " << fromNode << ", Message: " << message.GetPingReq().pingMessage);
    // Send Ping Response
    PennChordMessage resp = PennChordMessage (PennChordMessage::PING_RSP, message.GetTransactionId());
    resp.SetPingRsp (message.GetPingReq().pingMessage);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
    // Send indication to application layer
    m_pingRecvFn (sourceAddress, message.GetPingReq().pingMessage);
}

void
PennChord::ProcessPingRsp (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // Remove from pingTracker
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  iter = m_pingTracker.find (message.GetTransactionId ());
  if (iter != m_pingTracker.end ())
    {
      std::string fromNode = ReverseLookup (sourceAddress);
      if (DEBUG) CHORD_LOG ("Received PING_RSP, From Node: " << fromNode << ", Message: " << message.GetPingRsp().pingMessage);
      m_pingTracker.erase (iter);
      // Send indication to application layer
      m_pingSuccessFn (sourceAddress, message.GetPingRsp().pingMessage);
    }
  else
    {
      DEBUG_LOG ("Received invalid PING_RSP!");
    }
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