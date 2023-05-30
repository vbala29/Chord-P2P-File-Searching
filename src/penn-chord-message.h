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

#ifndef PENN_CHORD_MESSAGE_H
#define PENN_CHORD_MESSAGE_H

#include <stdint.h>
#include <string>
#include "buffer.h"

using namespace ns3; //For the Buffer class

#define IPV4_ADDRESS_SIZE 4


class PennChordMessage
{
  public:
    PennChordMessage ();
    virtual ~PennChordMessage ();

    enum MessageType
    {
      FIND_PRED_REQ = 3,
      FIND_PRED_RSP = 4,
      NOTIFY = 5,
      LEAVE_P = 6,
      LEAVE_S = 7,
      RING_STATE = 8,
      STABILIZE_REQ = 9,
      STABILIZE_RSP = 10,

    };

    PennChordMessage (PennChordMessage::MessageType messageType, uint32_t transactionId);

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
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);

    struct FindPredReq
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string findPredMessage; //(sender node), (node whose predecessor we want), ("1" if NOT HASHED node whose predecessor we want, "0" otherwise), "1" == SEARCHREQUEST or "2"== PUBLISHREQUEST (optional), hop count (optional)
      };

    struct FindPredRsp
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string findPredMessage; //Returns node,successor,predecessor, "1" == SEARCHREQUEST or "2"==PUBLISHREQUEST (optional), hash of value whose predecessor we wanted (optional), hop count (optional)
      };

    struct StabilizeReq
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string stabilizeMessage; //Empty
      };

    struct StabilizeRsp
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string stabilizeMessage; //Returns successor.predecessor
      };

    struct Notify
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string notifyMessage;
      };

    struct LeaveP
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string leavePMessage;
      };

    struct LeaveS
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string leaveSMessage;
      };

    struct RingState
      {
        void Print (std::ostream &os) const;
        uint32_t GetSerializedSize (void) const;
        void Serialize (Buffer::Iterator &start) const;
        uint32_t Deserialize (Buffer::Iterator &start);
        // Payload
        std::string ringStateMessage;
      };
      
  private:
    struct
      {
        FindPredReq findPredReq;
        FindPredRsp findPredRsp;
        Notify notify;
        LeaveP leaveP;
        LeaveS leaveS;
        RingState ringState;
        StabilizeReq stabilizeReq;
        StabilizeRsp stabilizeRsp;
      } m_message;
    
  public:

    /**
     * \returns FindPredReq Struct
     */
    FindPredReq GetFindPredReq ();
    /**
     *  \brief Sets FindPredReq message params
     *  \param message Payload String
     */
    void SetFindPredReq (std::string message);

    /**
     * \returns FindPredRsp Struct
     */
    FindPredRsp GetFindPredRsp ();
    /**
     *  \brief Sets FindPredRsp message params
     *  \param message Payload String
     */
    void SetFindPredRsp (std::string message);

     /**
     * \returns StabilizeReq Struct
     */
    StabilizeReq GetStabilizeReq ();
    /**
     *  \brief Sets StabilizeReq message params
     *  \param message Payload String
     */
    void SetStabilizeReq(std::string message);

    /**
     * \returns StabilizeReq Struct
     */
    StabilizeRsp GetStabilizeRsp ();
    /**
     *  \brief Sets StabilizeRsp message params
     *  \param message Payload String
     */
    void SetStabilizeRsp(std::string message);



    /**
     * \returns PingRsp Struct
     */
    Notify GetNotify ();
    /**
     *  \brief Sets PingRsp message params
     *  \param message Payload String
     */
    void SetNotify (std::string message);

    /**
     * \returns PingRsp Struct
     */
    LeaveP GetLeaveP ();
    /**
     *  \brief Sets PingRsp message params
     *  \param message Payload String
     */
    void SetLeaveP (std::string message);

    /**
     * \returns PingRsp Struct
     */
    LeaveS GetLeaveS ();
    /**
     *  \brief Sets PingRsp message params
     *  \param message Payload String
     */
    void SetLeaveS (std::string message);

    /**
     * \returns PingRsp Struct
     */
    RingState GetRingState ();
    /**
     *  \brief Sets PingRsp message params
     *  \param message Payload String
     */
    void SetRingState (std::string message);



}; // class PennChordMessage

static inline std::ostream& operator<< (std::ostream& os, const PennChordMessage& message)
{
  message.Print (os);
  return os;
}

#endif
