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

#ifndef PENN_SEARCH_H
#define PENN_SEARCH_H

#include "penn-application.h"
#include "penn-search-message.h"
#include "transmit.h"
#include "penn-chord.h"

#include "ipv4.hpp"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <list>
#include <deque>

#define SEARCH_APP_PORT 3001

class PennChord;

class PennSearch : public PennApplication
{
  public:
    PennSearch (PennChord* pc);
    virtual ~PennSearch ();
    virtual std::map<std::string, pthread_t> StartApplication (std::map<uint32_t, Ipv4Address> m_nodeAddressMap, std::map<Ipv4Address, uint32_t> m_addressNodeMap,  Ipv4Address m_local, std::string nodeId);
    virtual void StopApplication (void);

    void RecvMessage (PennSearchMessage message, Ipv4Address sourceAddress);
    void ProcessPublish(PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessSearchReq(PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessSearchRsp(PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    

    // Chord Callbacks
    void HandlePublish (Ipv4Address destAddress, std::string message);
    void HandleSearch (Ipv4Address destAddress, std::string message);
    void TransferFiles(Ipv4Address destAddress, std::string message);
    void HandleRehashKeys(Ipv4Address destAddress, std::string message);

    // From PennApplication
    virtual void ProcessCommand (std::vector<std::string> tokens);
    void ParsePublish(std::string filename);
    void ParseSearch(std::vector<std::string> tokens);
    std::string FormatStringForOutput(std::string intput);

    // From PennLog
    virtual void SetTrafficVerbose (bool on);
    virtual void SetErrorVerbose (bool on);
    virtual void SetDebugVerbose (bool on);
    virtual void SetStatusVerbose (bool on);
    virtual void SetChordVerbose (bool on);
    virtual void SetSearchVerbose (bool on);

    
  private:
    void PrintInvertedLists();

    PennChord* m_chord;
    uint32_t m_currentTransactionId;
    uint16_t m_appPort, m_chordPort;

    
    std::map<std::string, std::vector<std::string>> invertedLists; //Search term to list of documents used for PUBLISH
    std::map<uint32_t, std::pair<std::string, std::vector<std::string>>> hashInvertedLists; //Search term hash to pairs of <search term, list of documents> used for PUBLISH

    std::map<std::string, std::vector<std::string>> m_storedFiles; //Search term to list of documents that this node owns

    std::map<uint32_t, std::tuple<std::list<std::string>, std::deque<std::string>, std::string>> queryMap; //HASH passed into lookup mapped to pairs of invertedList, pendingTerms, queryId


};

#endif


