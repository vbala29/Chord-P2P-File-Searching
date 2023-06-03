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


#include "penn-search.h"
#include "penn-key-helper.h"


#define DEBUG 0
#define TEMP_DEBUG 0

void* PennSearchReceiveThread(void* args) {
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  uint8_t buff[4096];
  struct sockaddr_in my_addr, cli_addr;
  int n;


  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("ERROR opening socket in PennSearchReceiveThread()");
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
  portno = SEARCH_APP_PORT;

  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY; //monitor all interfaces (Aka IPs this host associated with)
  my_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
    perror("ERROR on binding in PennSearchReceiveThread()");
    exit(1);
  }

  if (listen(sockfd, 100) < 0) { // Allow up to 100 pending TCP SYN connections 
    perror("Error on listen()");
    exit(1);
  } 

  while(true) {
    clilen = sizeof(cli_addr);
    // std::cout << "Waiting to accept connection" << std::endl << std::flush;
    
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
      close(newsockfd);
      perror("ERROR on accept in PennSearchReceiveThread()");
      continue;
    }
    
    // fprintf(stderr, "established TCP connection from %s port %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    n = read(newsockfd, buff, 4095);
    if (n == -1) {
      perror("Error on read() in PennSearchReceiveThread()");
      close(newsockfd);
      continue;
    }

    close(newsockfd);

    BufferV2 b{};
    b.Write(buff, n);

    // std::cout << "Size of message rxed: " << n << std::endl << std::flush;
    // for (int i = 0; i < n; i++) {
    //     std::cout << buff[i] << ", ";
    // } 
    // std::cout << std::endl << std::flush;

    PennSearchMessage psm;
    psm.Deserialize(b);

    //Add in thread pool once this actually works TODO
    unsigned int* addrPtr = (unsigned int*) malloc(sizeof(unsigned int));
    ipStringToNumber(inet_ntoa(cli_addr.sin_addr), addrPtr);
    static_cast<PennSearch*>(args)->RecvMessage(psm, Ipv4Address(static_cast<uint32_t>(*addrPtr)));
  }

  return NULL;
}


PennSearch::PennSearch (PennChord* pc)
{
  m_chord = pc;
}

PennSearch::~PennSearch ()
{
  std::cout << "AvgHopCount<" << g_nodeId << ", " << m_chord->totalLookups << ", " << m_chord->totalHops << ">" << std::endl;
}


std::map<std::string, pthread_t>
PennSearch::StartApplication (std::map<uint32_t, Ipv4Address> m_nodeAddressMap, std::map<Ipv4Address, uint32_t> m_addressNodeMap,  Ipv4Address m_local, std::string nodeId)
{
  std::cout << "PennSearch::StartApplication()!!!!!" << std::endl;

  SetNodeAddressMap(m_nodeAddressMap);
  SetAddressNodeMap(m_addressNodeMap);
  SetLocalAddress(m_local);
  g_nodeId = nodeId;
  m_appPort = SEARCH_APP_PORT;

  // Configure Callbacks with Chord
  m_chord->SetPublishCallback((&PennSearch::HandlePublish));
  m_chord->SetSearchCallback(&PennSearch::HandleSearch);
  m_chord->SetLeaveCallback(&PennSearch::TransferFiles);
  m_chord->SetRehashKeysCallback(&PennSearch::HandleRehashKeys);

  m_chord->SetModuleName ("CHORD");

  SetSearchVerbose(true);
  SetErrorVerbose(true);
  SetChordVerbose(true);

  std::map<std::string, pthread_t> threadMap;

  pthread_t penn_search_receive_thread;
  pthread_create(&penn_search_receive_thread, NULL, PennSearchReceiveThread, this);
  threadMap.insert({"penn_search_receive_thread", penn_search_receive_thread});

  return threadMap;
}

void
PennSearch::StopApplication (void)
{
  std::cout << std::flush << std::endl;
}

void
PennSearch::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;

  if (command == "CHORD")
    { 
      // Send to Chord Sub-Layer
      tokens.erase (iterator);
      m_chord->ProcessCommand (tokens);
    } 
  else if (command == "PUBLISH") {
    // for (std::string& s : tokens) {
    //   std::cout << " " << s;
    // }

    // std::cout << std::endl << std::flush;

    if (tokens.size() != 2) {
      std::cerr << "Insufficient Parameters for PUBLISH!" << std::endl << std::flush;
    }

    ParsePublish(tokens.at(1)); //Pass in file name
    
  } else if (command == "SEARCH") {
    // for (std::string& s : tokens) {
    //   std::cout << " " << s;
    // }

    
    // std::cout << std::endl << std::flush;

    if (tokens.size() < 3) {
      std::cerr << "Insufficient Parameters for SEARCH!" << std::endl << std::flush;
    }

    ParseSearch(tokens);
  } else if (command == "PrintIvList") {
    PrintInvertedLists();
  } else {
    std::cerr << "Invalid command" << std::endl << std::flush;
  }
  
}


void PennSearch::TransferFiles(Ipv4Address destAddress, std::string message) {
  for (std::pair<std::string, std::vector<std::string>> mapping : m_storedFiles) {

    for (std::string s : mapping.second) {
      SEARCH_LOG("Publish<" << mapping.first << ", " << s << ">");
    }

    std::string msg = mapping.first;
    for (std::string s : mapping.second) {
      msg += ",";
      msg += s;
    }
    
    // parsing of the message 
    PennSearchMessage publish = PennSearchMessage(PennSearchMessage::MessageType::PUBLISH);
    publish.SetPublish(msg);
    sendTo (publish, m_appPort, m_nodeAddressMap.at(std::stoi(m_chord->getSuccessorNode())));
  }

  m_storedFiles.clear(); // All files been transferred so don't store any anymore
}

void PennSearch::ParsePublish(std::string filename) {

  std::cerr << "In (ParsePublish), file = " << filename << std::endl;
  std::ifstream f(filename, std::ios_base::in);

  if (f.fail()) {
    std::cerr << "ERROR READING FILE" << std::endl;
  }

  std::string line;
  invertedLists.clear();
  hashInvertedLists.clear();

  while(std::getline(f, line)) {
    std::istringstream s(line);
    std::string tok;
    int i = 0;
    std::string document;

    while (std::getline(s, tok, ' ')) {
      if (i == 0) {
        //Document name
        document = tok;

      } else {
        //token is keyword
        
        if (invertedLists.find(tok) != invertedLists.end()) {
          //Already have a vector in map
          
          invertedLists.at(tok).push_back(document);
          
        } else {
          std::vector<std::string> v;
          v.push_back(document);
          invertedLists[tok] = v;
        }
      SEARCH_LOG("Publish<" << tok << ", " << document << ">");
      }
      
      i++;
    }
  }

  for (std::pair<std::string, std::vector<std::string>> p : invertedLists) {
    hashInvertedLists[PennKeyHelper::CreateShaKey(p.first)] = p;
  }

  // Publishing inverted lists to the proper node
  for (std::pair<std::string, std::vector<std::string>> mapping : invertedLists) {
    // std::cerr << "In for loop, mapping " << mapping.first << std::endl;
    uint32_t hash = PennKeyHelper::CreateShaKey(mapping.first);
    // std::cout << "term: [" << mapping.first << "] hashes to " << PennKeyHelper::KeyToHexString(hash) << std::endl;  
    m_chord->Lookup(hash);
  }

  
  // SEARCH_LOG("Publish<" << std::to_string(m_chord->currHash) << ", " << PennKeyHelper::CreateShaKey(fromNode) << ">");

}

void PennSearch::ParseSearch(std::vector<std::string> tokens) {
  std::deque<std::string> pendingTerms;
  std::string nodeToSearchFrom = tokens.at(1);
  std::string queryId = g_nodeId; // Signifies that this is the initial request to node in ring.

  for (size_t i = 2; i < tokens.size(); i++) {
    pendingTerms.push_back(tokens.at(i));
  }

  std::string pendingTermsString;
  for (std::string s : pendingTerms) {
    pendingTermsString += s + ",";
  }

  if (TEMP_DEBUG) std::cout << "Node: " << g_nodeId << " created Search Query with pendingTermsString: " << pendingTermsString << std::endl;
  SEARCH_LOG("Search<" << pendingTermsString.substr(0, pendingTermsString.length() - 1) << ">");

  // Send request to node to search from
  PennSearchMessage message = PennSearchMessage(PennSearchMessage::SEARCH_REQ);
  message.SetSearchReq (queryId, "", pendingTermsString, true);
  sendTo (message, m_appPort, m_nodeAddressMap[static_cast<uint32_t>(std::stoi(nodeToSearchFrom))]);
}

void
PennSearch::RecvMessage (PennSearchMessage message, Ipv4Address sourceAddress)
{
  uint16_t sourcePort = m_appPort;
 // std::cout << "PennSearch Received message of type " << message.GetMessageType() << ", from: " << sourceAddress.Ipv4ToString() << std::endl << std::flush;

  switch (message.GetMessageType ())
    {
      case PennSearchMessage::PUBLISH:
        ProcessPublish(message, sourceAddress, sourcePort);
        break;
      case PennSearchMessage::SEARCH_REQ:
        ProcessSearchReq(message, sourceAddress, sourcePort);
        break;
      case PennSearchMessage::SEARCH_RSP:
        ProcessSearchRsp(message, sourceAddress, sourcePort);
        break;
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}

std::string PennSearch::FormatStringForOutput(std::string input) {

  if (input == "\'Empty List\'") return input;

  std::string output;
  output += "{";

   std::stringstream ss(input);

   while(ss.good()) {
    std::string substr;
    std::getline(ss, substr, ',');
    output += substr + ", ";
   }

   output = output.substr(0, output.length() - 2);
   output += "}";

   return output;
}

void PennSearch::ProcessSearchRsp(PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  if (TEMP_DEBUG) SEARCH_LOG("Received SEARCH_RSP, From Node: " << fromNode << ", Inverted List: " << message.GetSearchRsp().invertedList);
  std::string list = message.GetSearchRsp().invertedList;
  SEARCH_LOG("SearchResults<" << m_nodeAddressMap[std::stoi(g_nodeId)] << ", " << FormatStringForOutput(list.substr(0, list.length() - 1)) << ">");
}

void PennSearch::ProcessSearchReq(PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);


  //Inverted List Building
  std::list<std::string> invertedList; //Allows for easy removal via iterator
  std::stringstream ss(message.GetSearchReq().invertedList);

  while (ss.good()) {
    std::string substr;
    std::getline(ss, substr, ',');
    invertedList.push_back(substr);
  }
  invertedList.pop_back(); //Pop extra comma

  //Pending Terms List Building
  std::stringstream ss2 (message.GetSearchReq().pendingTerms);
  std::deque<std::string> pendingTerms;

  while (ss2.good()) {
    std::string substr;
    std::getline(ss2, substr, ',');
    pendingTerms.push_back(substr);
  }
  pendingTerms.pop_back(); //Pop extra comma
  //End list building code



  //Case 1: this is initial request to node in ring.
  if (message.GetSearchReq().initialReq == "1") { 
    if (TEMP_DEBUG) SEARCH_LOG("Received INITIAL SEARCH_REQ, From Node: " << fromNode << ", Inverted List = [" << message.GetSearchReq().invertedList 
                                      << "], pendingTerms = {" << message.GetSearchReq().pendingTerms << "}, queryId: " << message.GetSearchReq().queryId);


    uint32_t hash = PennKeyHelper::CreateShaKey(pendingTerms.front());
    auto tup = std::make_tuple(invertedList, pendingTerms, message.GetSearchReq().queryId);
    queryMap[hash] = tup; // For callback to reference

    m_chord->makingSearchQuery = true; //Such that lookup sets the correct fields in the FIND_PRED_REQ
    m_chord->Lookup(hash);
    m_chord->makingSearchQuery = false;
    return;
  }

  if (TEMP_DEBUG) SEARCH_LOG("Received SEARCH_REQ, From Node: " << fromNode << ", Inverted List = [" << message.GetSearchReq().invertedList 
                                      << "], pendingTerms = {" << message.GetSearchReq().pendingTerms << "}, queryId: " << message.GetSearchReq().queryId);


  //Case 2: This is not initial request to node in ring, so current termToLook for should be here
  std::string termToLookFor = pendingTerms.front();
  pendingTerms.pop_front(); //Decrease size by 1 
  std::vector<std::string> candidateDocuments;

  if (m_storedFiles.find(termToLookFor) != m_storedFiles.end()) {
    candidateDocuments = m_storedFiles.at(termToLookFor);
  } // else candidate documents has size 0 so invertedLists will get cleared.

  std::string candidateDocumentsString;
  for (std::string s : candidateDocuments) {
    candidateDocumentsString += s + ",";
  }

  if (candidateDocuments.size() == 0) {
     SEARCH_LOG("InvertedListShip<" << termToLookFor << ", " << "\'Empty List\'" << ">");
  } else SEARCH_LOG("InvertedListShip<" << termToLookFor << ", " 
              << FormatStringForOutput(candidateDocumentsString.substr(0, candidateDocumentsString.length() - 1)) << ">");

  if (invertedList.size() == 0) {
    //This is the first search query so NO "AND" comparison needs to happen. Just append candidateDocuments to invertedList
    for (std::string s : candidateDocuments) {
      invertedList.push_back(s);
    }
  } else {
    //Comparison needs to happen.
      std::set<std::string> s (candidateDocuments.begin(), candidateDocuments.end());

    //Iterator through doubly linked list for fast removal via .erase()
    auto it = invertedList.begin();
    while(it != invertedList.end()) {
      if (s.find(*it) == s.end()) {
        //Must remove from invertedList
        it = invertedList.erase(it);
      } else {
        ++it;
      }
    }

  }
  
  //Now invertedLists is in the updated state.
  if (invertedList.size() == 0) {
    if (TEMP_DEBUG) SEARCH_LOG("SENDING SEARCH_RSP, From Node: " << fromNode << ", Returning EMPTY LIST, queryId: " <<  message.GetSearchReq().queryId);

    //Send back a no result call

    PennSearchMessage rspMessage = PennSearchMessage(PennSearchMessage::SEARCH_RSP);
    rspMessage.SetSearchRsp(rspMessage.GetSearchReq().queryId, "\'Empty List\',"); //The comma is scuffed lol but it is because I do a substr on the response in processSearchRsp due to extra commas in non empty list returns.
    sendTo (rspMessage, m_appPort, m_nodeAddressMap[static_cast<uint32_t>(std::stoi(message.GetSearchReq().queryId))]); //Query id is original requester

    //SEARCH_LOG("SearchResults<" << m_nodeAddressMap[std::stoi(g_nodeId)] << ", \'Empty List\'>");
  } else if (pendingTerms.size() == 0) {
    //Send back a search result SEARCH_RSP since there's no more pending terms
    PennSearchMessage rspMessage = PennSearchMessage(PennSearchMessage::SEARCH_RSP);

    std::string invertedListString;
    for (std::string s : invertedList) {
      invertedListString += s + ",";
    }

   // SEARCH_LOG("SearchResults<" << m_nodeAddressMap[std::stoi(g_nodeId)] << ", " << FormatStringForOutput(invertedListString.substr(0, invertedListString.length() - 1)) << ">");

    rspMessage.SetSearchRsp(message.GetSearchReq().queryId, invertedListString);
    sendTo (rspMessage, m_appPort, m_nodeAddressMap[static_cast<uint32_t>(std::stoi(message.GetSearchReq().queryId))]); //Query id is original requester

  } else {
    if (TEMP_DEBUG) SEARCH_LOG("Forwarding SEARCH_REQ, From Node: " << fromNode << ", queryId: " << message.GetSearchReq().queryId);

    //lookup for next search Id
    uint32_t hash = PennKeyHelper::CreateShaKey(pendingTerms.front());
    auto tup = std::make_tuple(invertedList, pendingTerms, message.GetSearchReq().queryId);
    queryMap[hash] = tup ; // For callback to reference

    m_chord->makingSearchQuery = true; //Such that lookup sets the correct fields in the FIND_PRED_REQ
    m_chord->Lookup(hash);
    m_chord->makingSearchQuery = false;
  }


}


void PennSearch::ProcessPublish(PennSearchMessage message, Ipv4Address sourceAddress, uint16_t sourcePort) {
  std::string fromNode = ReverseLookup (sourceAddress);
  if (DEBUG) SEARCH_LOG("Received PUBLISH, From Node: " << fromNode << ", Message: " << message.GetPublish().publishMessage);

  // Parse input msg
  std::string key;
  std::vector<std::string> tokens;
  
  std::stringstream ss(message.GetPublish().publishMessage);

  // Extract key
  std::getline(ss, key, ',');

  // Extract documents
  while (ss.good()) {
      std::string substr;
      std::getline(ss, substr, ',');
      tokens.push_back(substr);
  }

  if (m_storedFiles.find(key) == m_storedFiles.end()) {
    // New document to add to map
    std::vector<std::string> v;
    m_storedFiles[key] = v;        
  }


  //Set of all items in the vector we are editing. Use this to make sure we don't add duplicates to the inverted list.
  std::set<std::string> s (m_storedFiles.at(key).begin(), m_storedFiles.at(key).end());
  
  for (std::string tok : tokens) {
    if (s.find(tok) == s.end()) {
      SEARCH_LOG("Store<" << key << ", " << tok << ">");
      m_storedFiles.at(key).push_back(tok);
    }
  }

}

void PennSearch::PrintInvertedLists() {
  //Print out m_storedFiles
  for (auto & p : m_storedFiles) {
    std::cerr << "Node: " << g_nodeId << ", Keyword: " << p.first << "-->  ";
    for (std::string& s : p.second) {
      std::cerr << s << ", ";
    }
    std::cerr << std::endl;
  }
  std::cerr << std::endl << std::endl << std::endl;
}

// Handle Chord Callbacks


void
PennSearch::HandleRehashKeys (Ipv4Address destAddress, std::string message) 
{
  std::vector<std::string> toDelete;
  for (auto p : m_storedFiles) {
    uint32_t hash = PennKeyHelper::CreateShaKey(p.first);
    uint32_t currHash = PennKeyHelper::CreateShaKey(g_nodeId);
    uint32_t predecessorHash = m_chord->getPredecessorHash();

    if (!((currHash > predecessorHash && hash > predecessorHash && hash <= currHash) || (currHash <= predecessorHash && (hash > predecessorHash || hash <= currHash)))) {
      toDelete.push_back(p.first);
    
      for (std::string s : p.second) {
        std::string msg;
        msg += p.first;
        msg += "," + s;

        SEARCH_LOG("PublishRehash<" << p.first << ", " << s << ">");
        PennSearchMessage publish = PennSearchMessage(PennSearchMessage::MessageType::PUBLISH);
        publish.SetPublish(msg);

        // PRINT_LOG("DEBUG MESSAGE: " << "Node we want to send to: "  <<  m_chord->getSuccessorNode() << ", IP to send to: " << m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(m_chord->getSuccessorNode()))) 
        //                       << ", Reverse Lookup: " << ReverseLookup(m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(m_chord->getSuccessorNode())))));
        sendTo (publish, m_appPort, m_nodeAddressMap.at(static_cast<uint32_t>(std::stoi(m_chord->getPredecessorNode()))));
      } 
    }
  }
  
  // Erase all files that were sent to the successor
  for (std::string& s : toDelete) {
    m_storedFiles.erase(s);
  }
}

void
PennSearch::HandleSearch (Ipv4Address destAddress, std::string message) 
{
  if (DEBUG) SEARCH_LOG ("SEARCH Handler Called! Source nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);

  std::vector<std::string> v;

  std::stringstream ss(message);

  while (ss.good()) {
      std::string substr;
      std::getline(ss, substr, ',');
      v.push_back(substr);
  }

  std::string nodeToSendTo = v.at(1);

  std::string queryHash {}; //The original query to PennChord::lookup()

  if (v.size() >= 5) {
    queryHash = v.at(4);
  } else {
    ERROR_LOG ("SEARCH CALLBACK DID NOT HAVE ENOUGH ARGUMENTS");
  }

  auto tup = queryMap.at(static_cast<uint32_t>(std::stoul(queryHash)));
  auto invertedList = std::get<0>(tup);
  auto pendingTerms = std::get<1>(tup);
  auto queryId = std::get<2>(tup);

  std::string invertedListString;
  for (std::string s : invertedList) {
    invertedListString += s + ",";
  }

  std::string pendingTermsString;
  for (std::string s : pendingTerms) {
    pendingTermsString += s + ",";
  }

  // Send request to node to search from
  PennSearchMessage msg = PennSearchMessage(PennSearchMessage::SEARCH_REQ);
  msg.SetSearchReq (queryId, invertedListString, pendingTermsString, false);
  sendTo (msg, m_appPort, m_nodeAddressMap[static_cast<uint32_t>(std::stoi(nodeToSendTo))]);
}

void
PennSearch::HandlePublish (Ipv4Address destAddress, std::string message)
{
  if (DEBUG) SEARCH_LOG ("PUBLISH Handler Called! Source nodeId: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << message);

  std::vector<std::string> v;

  std::stringstream ss(message);

  while (ss.good()) {
      std::string substr;
      std::getline(ss, substr, ',');
      v.push_back(substr);
  }
  std::string nodeToSendTo = v.at(1);
  std::string queryHash {}; //The original query to PennChord::lookup()

  if (v.size() >= 5) {
    queryHash = v.at(4);
  } else {
    ERROR_LOG ("PUBLISH CALLBACK DID NOT HAVE ENOUGH ARGUMENTS");
  }

  if (DEBUG) std::cout << "Node to send to: " << nodeToSendTo << ", queryHash: " << queryHash << std::endl;

  auto p = hashInvertedLists.at(static_cast<uint32_t>(std::stoul(queryHash)));

  std::string msg = p.first;
  for (std::string s : p.second) {
    msg += ",";
    msg += s;
  }
  
  // parsing of the message 
  PennSearchMessage publish = PennSearchMessage(PennSearchMessage::MessageType::PUBLISH);

  publish.SetPublish(msg);
  sendTo (publish, m_appPort, m_nodeAddressMap.at(std::stoi(nodeToSendTo)));
}

// Override PennLog

void
PennSearch::SetTrafficVerbose (bool on)
{ 
  m_chord->SetTrafficVerbose (on);
  g_trafficVerbose = on;
}

void
PennSearch::SetErrorVerbose (bool on)
{ 
  m_chord->SetErrorVerbose (on);
  g_errorVerbose = on;
}

void
PennSearch::SetDebugVerbose (bool on)
{
  m_chord->SetDebugVerbose (on);
  g_debugVerbose = on;
}

void
PennSearch::SetStatusVerbose (bool on)
{
  m_chord->SetStatusVerbose (on);
  g_statusVerbose = on;
}

void
PennSearch::SetChordVerbose (bool on)
{
  m_chord->SetChordVerbose (on);
  g_chordVerbose = on;
}

void
PennSearch::SetSearchVerbose (bool on)
{
  m_chord->SetSearchVerbose (on);
  g_searchVerbose = on;
}
