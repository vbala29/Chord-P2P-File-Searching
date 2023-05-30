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

#include "penn-chord-message.h"
#include "assert.h"

using namespace ns3;

PennChordMessage::PennChordMessage ()
{
}

PennChordMessage::~PennChordMessage ()
{
}

PennChordMessage::PennChordMessage (PennChordMessage::MessageType messageType, uint32_t transactionId)
{
  m_messageType = messageType;
}


uint32_t
PennChordMessage::GetSerializedSize (void) const
{
  // size of messageType, transaction id
  uint32_t size = sizeof (uint8_t) + sizeof (uint32_t);
  switch (m_messageType)
    {
      case FIND_PRED_REQ:
        size += m_message.findPredReq.GetSerializedSize ();
        break;
      case FIND_PRED_RSP:
        size += m_message.findPredRsp.GetSerializedSize ();
        break;
      case NOTIFY:
        size += m_message.notify.GetSerializedSize ();
        break;
      case LEAVE_P:
        size += m_message.leaveP.GetSerializedSize ();
        break;
      case LEAVE_S:
        size += m_message.leaveS.GetSerializedSize ();
        break;
      case RING_STATE:
        size += m_message.ringState.GetSerializedSize ();
        break;
      case STABILIZE_REQ:
        size += m_message.stabilizeReq.GetSerializedSize ();
        break;
      case STABILIZE_RSP:
        size += m_message.stabilizeRsp.GetSerializedSize ();
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
PennChordMessage::Print (std::ostream &os) const
{
  os << "\n****PennChordMessage Dump****\n" ;
  os << "messageType: " << m_messageType << "\n";
  os << "PAYLOAD:: \n";
  
  switch (m_messageType)
    {
      case FIND_PRED_REQ:
        m_message.findPredReq.Print (os);
        break;
      case FIND_PRED_RSP:
        m_message.findPredRsp.Print (os);
        break;
      case NOTIFY:
        m_message.notify.Print (os);
        break;
      case LEAVE_P:
        m_message.leaveP.Print (os);
        break;
      case LEAVE_S:
        m_message.leaveS.Print (os);
        break;
      case RING_STATE:
        m_message.ringState.Print (os);
        break;
      case STABILIZE_REQ:
        m_message.stabilizeReq.Print (os);
        break;
      case STABILIZE_RSP:
        m_message.stabilizeRsp.Print(os);
        break;
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
PennChordMessage::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteHtonU32 (0); //The deprecated transaction ID

  switch (m_messageType)
    {
      case FIND_PRED_REQ:
        m_message.findPredReq.Serialize (i);
        break;
      case FIND_PRED_RSP:
        m_message.findPredRsp.Serialize (i);
        break;
      case NOTIFY:
        m_message.notify.Serialize (i);
        break;
      case LEAVE_P:
        m_message.leaveP.Serialize (i);
        break;
      case LEAVE_S:
        m_message.leaveS.Serialize (i);
        break;
      case RING_STATE:
        m_message.ringState.Serialize (i);
        break;
      case STABILIZE_REQ:
        m_message.stabilizeReq.Serialize (i);
        break;
      case STABILIZE_RSP:
        m_message.stabilizeRsp.Serialize (i);
        break;
      default:
        NS_ASSERT (false);   
    }
}

uint32_t 
PennChordMessage::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType = (MessageType) i.ReadU8 ();
  i.ReadNtohU32 (); //discard the old transaction ID

  size = sizeof (uint8_t) + sizeof (uint32_t);

  switch (m_messageType)
    {
      case FIND_PRED_REQ:
        size += m_message.findPredReq.Deserialize (i);
        break;
      case FIND_PRED_RSP:
        size += m_message.findPredRsp.Deserialize (i);
        break;
      case NOTIFY:
        size += m_message.notify.Deserialize (i);
        break;
      case LEAVE_P:
        size += m_message.leaveP.Deserialize (i);
        break;
      case LEAVE_S:
        size += m_message.leaveS.Deserialize (i);
        break;
      case RING_STATE:
        size += m_message.ringState.Deserialize (i);
        break;
      case STABILIZE_REQ:
        size += m_message.stabilizeReq.Deserialize (i);
        break;
      case STABILIZE_RSP:
        size += m_message.stabilizeRsp.Deserialize (i);
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}


/* FINDPREDREQ */

uint32_t 
PennChordMessage::FindPredReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + findPredMessage.length();
  return size;
}

void
PennChordMessage::FindPredReq::Print (std::ostream &os) const
{
  os << "FindPredReq:: Message: " << findPredMessage << "\n";
}

void
PennChordMessage::FindPredReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (findPredMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (findPredMessage.c_str())), findPredMessage.length());
}

uint32_t
PennChordMessage::FindPredReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  findPredMessage = std::string (str, length);
  free (str);
  return FindPredReq::GetSerializedSize ();
}

void
PennChordMessage::SetFindPredReq (std::string findPredMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = FIND_PRED_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == FIND_PRED_REQ);
    }
  m_message.findPredReq.findPredMessage = findPredMessage;
}

PennChordMessage::FindPredReq PennChordMessage::GetFindPredReq ()
{
  return m_message.findPredReq;
}

/* END OF FINDPREDREQ */

/* FINDPREDRSP */

uint32_t 
PennChordMessage::FindPredRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + findPredMessage.length();
  return size;
}

void
PennChordMessage::FindPredRsp::Print (std::ostream &os) const
{
  os << "FindPredRsp:: Message: " << findPredMessage << "\n";
}

void
PennChordMessage::FindPredRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (findPredMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (findPredMessage.c_str())), findPredMessage.length());
}

uint32_t
PennChordMessage::FindPredRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  findPredMessage = std::string (str, length);
  free (str);
  return FindPredRsp::GetSerializedSize ();
}

void
PennChordMessage::SetFindPredRsp (std::string findPredMessage)
{
  
  if (m_messageType == 0)
    {
      m_messageType = FIND_PRED_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == FIND_PRED_RSP);
    }
  m_message.findPredRsp.findPredMessage = findPredMessage;
}

PennChordMessage::FindPredRsp PennChordMessage::GetFindPredRsp ()
{
  return m_message.findPredRsp;
}

/* END OF FINDPREDRSP */


/* STABILIZEREQ */

uint32_t 
PennChordMessage::StabilizeReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + stabilizeMessage.length();
  return size;
}

void
PennChordMessage::StabilizeReq::Print (std::ostream &os) const
{
  os << "StabilizeReq:: Message: " << stabilizeMessage << "\n";
}

void
PennChordMessage::StabilizeReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (stabilizeMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (stabilizeMessage.c_str())), stabilizeMessage.length());
}

uint32_t
PennChordMessage::StabilizeReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  stabilizeMessage = std::string (str, length);
  free (str);
  return StabilizeReq::GetSerializedSize ();
}

void
PennChordMessage::SetStabilizeReq (std::string stabilizeMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = STABILIZE_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == STABILIZE_REQ);
    }
  m_message.stabilizeReq.stabilizeMessage = stabilizeMessage;
}

PennChordMessage::StabilizeReq PennChordMessage::GetStabilizeReq ()
{
  return m_message.stabilizeReq;
}

/* END OF STABILIZEREQ */


/* STABILIZERSP */

uint32_t 
PennChordMessage::StabilizeRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + stabilizeMessage.length();
  return size;
}

void
PennChordMessage::StabilizeRsp::Print (std::ostream &os) const
{
  os << "StabilizeRsp:: Message: " << stabilizeMessage << "\n";
}

void
PennChordMessage::StabilizeRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (stabilizeMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (stabilizeMessage.c_str())), stabilizeMessage.length());
}

uint32_t
PennChordMessage::StabilizeRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  stabilizeMessage = std::string (str, length);
  free (str);
  return StabilizeRsp::GetSerializedSize ();
}

void
PennChordMessage::SetStabilizeRsp (std::string stabilizeMessage)
{
  
  if (m_messageType == 0)
    {
      m_messageType = STABILIZE_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == STABILIZE_RSP);
    }
  m_message.stabilizeRsp.stabilizeMessage = stabilizeMessage;
}

PennChordMessage::StabilizeRsp PennChordMessage::GetStabilizeRsp ()
{
  return m_message.stabilizeRsp;
}

/* END OF STABILIZERSP */

/* NOTIFY */

uint32_t 
PennChordMessage::Notify::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + notifyMessage.length();
  return size;
}

void
PennChordMessage::Notify::Print (std::ostream &os) const
{
  os << "Notify:: Message: " << notifyMessage << "\n";
}

void
PennChordMessage::Notify::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (notifyMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (notifyMessage.c_str())), notifyMessage.length());
}

uint32_t
PennChordMessage::Notify::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  notifyMessage = std::string (str, length);
  free (str);
  return Notify::GetSerializedSize ();
}

void
PennChordMessage::SetNotify (std::string notifyMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = NOTIFY;
    }
  else
    {
      NS_ASSERT (m_messageType == NOTIFY);
    }
  m_message.notify.notifyMessage = notifyMessage;
}

PennChordMessage::Notify PennChordMessage::GetNotify ()
{
  return m_message.notify;
}

/* END OF NOTIFY */

/* LEAVEP */

uint32_t 
PennChordMessage::LeaveP::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + leavePMessage.length();
  return size;
}

void
PennChordMessage::LeaveP::Print (std::ostream &os) const
{
  os << "LeaveP:: Message: " << leavePMessage << "\n";
}

void
PennChordMessage::LeaveP::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (leavePMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (leavePMessage.c_str())), leavePMessage.length());
}

uint32_t
PennChordMessage::LeaveP::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  leavePMessage = std::string (str, length);
  free (str);
  return LeaveP::GetSerializedSize ();
}

void
PennChordMessage::SetLeaveP (std::string leavePMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = LEAVE_P;
    }
  else
    {
      NS_ASSERT (m_messageType == LEAVE_P);
    }
  m_message.leaveP.leavePMessage = leavePMessage;
}

PennChordMessage::LeaveP
PennChordMessage::GetLeaveP ()
{
  return m_message.leaveP;
}

/* END OF LEAVEP */

/* LEAVES */

uint32_t 
PennChordMessage::LeaveS::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + leaveSMessage.length();
  return size;
}

void
PennChordMessage::LeaveS::Print (std::ostream &os) const
{
  os << "LeaveS:: Message: " << leaveSMessage << "\n";
}

void
PennChordMessage::LeaveS::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (leaveSMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (leaveSMessage.c_str())), leaveSMessage.length());
}

uint32_t
PennChordMessage::LeaveS::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  leaveSMessage = std::string (str, length);
  free (str);
  return LeaveS::GetSerializedSize ();
}

void
PennChordMessage::SetLeaveS (std::string leaveSMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = LEAVE_S;
    }
  else
    {
      NS_ASSERT (m_messageType == LEAVE_S);
    }
  m_message.leaveS.leaveSMessage = leaveSMessage;
}

PennChordMessage::LeaveS
PennChordMessage::GetLeaveS ()
{
  return m_message.leaveS;
}

/* END OF LEAVES */

/* RINGSTATE */

uint32_t 
PennChordMessage::RingState::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + ringStateMessage.length();
  return size;
}

void
PennChordMessage::RingState::Print (std::ostream &os) const
{
  os << "RingState:: Message: " << ringStateMessage << "\n";
}

void
PennChordMessage::RingState::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (ringStateMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (ringStateMessage.c_str())), ringStateMessage.length());
}

uint32_t
PennChordMessage::RingState::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  ringStateMessage = std::string (str, length);
  free (str);
  return RingState::GetSerializedSize ();
}

void
PennChordMessage::SetRingState (std::string ringStateMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = RING_STATE;
    }
  else
    {
      NS_ASSERT (m_messageType == RING_STATE);
    }
  m_message.ringState.ringStateMessage = ringStateMessage;
}

PennChordMessage::RingState
PennChordMessage::GetRingState ()
{
  return m_message.ringState;
}

/* END OF RINGSTATE */

void
PennChordMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

PennChordMessage::MessageType
PennChordMessage::GetMessageType () const
{
  return m_messageType;
}

