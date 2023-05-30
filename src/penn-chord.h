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

#ifndef PENN_CHORD_H
#define PENN_CHORD_H


#include "ipv4.hpp"
#include "buffer.h"
#include "penn-application.h"
#include "penn-chord-message.h"
#include "penn-key-helper.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <array>

#define HASHED_FIELD "0"
#define NOT_HASHED_FIELD "1"
#define SEARCH_QUERY "1"
#define PUBLISH_QUERY "2"

using namespace ns3;

class PennChord : public PennApplication
{
  public:
    typedef void (*Callback) (Ipv4Address, std::string);

    PennChord ();
    virtual ~PennChord ();
    void StartApplication (std::map<uint32_t, Ipv4Address> m_nodeAddressMap, std::map<Ipv4Address, uint32_t> m_addressNodeMap,  Ipv4Address m_local, std::string nodeId);

    void RecvMessage (Ptr<Socket> socket);
    void ProcessPingReq (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessPingRsp (PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void StopChord ();

    // Callback with Application Layer (add more when required)
    void SetPublishCallback (Callback publishFn);
    void SetSearchCallback (Callback searchFn);
    void SetLeaveCallback (Callback leaveFn);
    void SetRehashKeysCallback (Callback rehashKeysFn);

    // From PennApplication
    virtual void ProcessCommand (std::vector<std::string> tokens);

    // Public Chord API function
    void Lookup(uint32_t id_hash);
    uint32_t getSuccessorHash();
    uint32_t getPredecessorHash();
    std::string getSuccessorNode();
    std::string getPredecessorNode();


  bool makingSearchQuery = false;
  uint32_t totalLookups = 0;
  uint32_t totalHops = 0;

    
  private:
    virtual void StopApplication (void);

    //Commands
    void RingState();
    void Leave();
    void CreateRing(std::string& currNode);
    void Join(std::string& nodeContained, std::string& currNum);
    void Notify();
    void Stabilize();
    void FixFingers();
    void FindSuccessor(uint32_t hashOfNode);
    
    //Processing
    void ProcessFindPredRsp(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessFindPredReq(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessLeaveP(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessLeaveS(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessRingState(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessNotify(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessStabilizeRsp(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessStabilizeReq(PennChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);

    // Finger Table Method
    std::string ClosestPrecedingNode(uint32_t id);


    uint32_t successorHash; // -1 = Nil
    uint32_t predecessorHash; // -1 = Nil
    uint32_t currHash; // -1 = Nil
    std::string successorNumber; // "-1" = Nil
    std::string predecessorNumber; // "-1" = Nil
    std::string currNumber; // "-1" = Nil
    Ipv4Address successorIP;
    Ipv4Address predecessorIP;
    Ipv4Address currIP;
    uint32_t nextFinger;

    bool tryingToJoin = false;
    bool fixingFingers = false;

    bool haveNotifiedOnce = false;
    bool inRing = false;
    bool isSingleton = false; //Ring only has one node

    uint16_t m_appPort;
    
    // Timers
    Timer m_stabilizeTimer;
    Timer m_fixFingersTimer;
    
    // Callbacks
    Callback m_pingSuccessFn;
    Callback m_pingFailureFn;
    Callback m_pingRecvFn;
    Callback m_publishFn;
    Callback m_searchFn;
    Callback m_leaveFn;
    Callback m_rehashKeys;

    // Finger Table
    std::array<std::string, 32> m_fingerTable; //stores node numbers not hash numbers
    


};

#endif


