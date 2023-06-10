/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#ifndef PENN_SEARCH_MESSAGE_H
#define PENN_SEARCH_MESSAGE_H

#include "BufferV2.h"
#include "ipv4.hpp"
#include "assert.h"


#define IPV4_ADDRESS_SIZE 4

class PennSearchMessage
{
  public:
    PennSearchMessage ();
    virtual ~PennSearchMessage ();


    enum MessageType
      {
        PUBLISH = 3,
        SEARCH_REQ = 4,
        SEARCH_RSP = 5,

      };

    PennSearchMessage (PennSearchMessage::MessageType messageType);

    /**
    *  \brief Sets message type
    *  \param messageType message type
    */
    void SetMessageType (MessageType messageType);

    /**
     *  \returns message type
     */
    MessageType GetMessageType () const;

  private:
    /**
     *  \cond
     */
    MessageType m_messageType;
    /**
     *  \endcond
     */
  public:
    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (BufferV2& i) const;
    uint32_t Deserialize (BufferV2& i);


    struct Publish
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (BufferV2 &start) const;
        uint32_t Deserialize (BufferV2 &start);
        // Payload
        std::string publishMessage;
      };
 
    struct SearchReq
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (BufferV2 &start) const;
        uint32_t Deserialize (BufferV2 &start);

        // Payload
        std::string queryId; //The id (not the hash) of the node that made the query
        std::string invertedList;
        std::string pendingTerms; 
        std::string initialReq; //Indicates that this is initial request to node in ring. (E.g. if 2 SEARCH 0), the request from 2 to 0 is an initial request. "1" = true, "0" = false.
      };
 
    struct SearchRsp
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (BufferV2 &start) const;
        uint32_t Deserialize (BufferV2 &start);
        
        // Payload
        std::string queryId; //The hash of the node that made the query
        std::string invertedList;

      };
 

  private:
    struct
      {
        Publish publish;
        SearchReq searchReq;
        SearchRsp searchRsp;
      } m_message;
    
  public:
    /**
     * \returns Publish Struct
     */
    Publish GetPublish ();
    /**
     *  \brief Sets Publish message params
     *  \param message Payload String
     */
    void SetPublish (std::string message);


    /**
     * \returns SearchReq Struct
     */
    SearchReq GetSearchReq ();
    /**
     *  \brief Sets SearchReq message params
     *  \param message Payload String
     */
    void SetSearchReq (std::string queryId, std::string invertedList, std::string pendingTerms, bool initialReq);

    /**
     * \returns SearchRsp Struct
     */
    SearchRsp GetSearchRsp ();
    /**
     *  \brief Sets SearchRsp message params
     *  \param message Payload String
     */
    void SetSearchRsp (std::string queryId, std::string invertedList);


}; // class PennSearchMessage

static inline std::ostream& operator<< (std::ostream& os, const PennSearchMessage& message)
{
  message.Print (os);
  return os;
}

#endif
