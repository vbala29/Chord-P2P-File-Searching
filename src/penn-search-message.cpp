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

#include "../include/penn-search-message.h"
#include "../include/penn-log.h"


PennSearchMessage::PennSearchMessage ()
{
}

PennSearchMessage::~PennSearchMessage ()
{
}

PennSearchMessage::PennSearchMessage (PennSearchMessage::MessageType messageType)
{
  m_messageType = messageType;
}


uint32_t
PennSearchMessage::GetSerializedSize (void) const
{
  // size of messageType, transaction id
  uint32_t size = sizeof (uint8_t) + sizeof (uint32_t);
  switch (m_messageType)
    {
      case PUBLISH:
        size += m_message.publish.GetSerializedSize();
        break;
      case SEARCH_REQ:
        size += m_message.searchReq.GetSerializedSize();
        break;
      case SEARCH_RSP:
        size += m_message.searchRsp.GetSerializedSize();
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
PennSearchMessage::Print (std::ostream &os) const
{
  os << "\n****PennSearchMessage Dump****\n" ;
  os << "messageType: " << m_messageType << "\n";
  os << "PAYLOAD:: \n";
  
  switch (m_messageType)
    {
      case PUBLISH:
        m_message.publish.Print (os);
        break;
      case SEARCH_REQ:
        m_message.searchReq.Print(os);
        break;
      case SEARCH_RSP:
        m_message.searchRsp.Print(os);
        break;
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
PennSearchMessage::Serialize (BufferV2& i) const
{
  i.WriteU8 (m_messageType);
  i.WriteHtonU32 (0); //Dummy transaction ID

  switch (m_messageType)
    {
      case PUBLISH:
        m_message.publish.Serialize (i);
        break;
      case SEARCH_REQ:
        m_message.searchReq.Serialize (i);
        break;
      case SEARCH_RSP:
        m_message.searchRsp.Serialize (i);
        break;
      default:
        NS_ASSERT (false);   
    }
}

uint32_t 
PennSearchMessage::Deserialize (BufferV2& i)
{
  uint32_t size;
  m_messageType = (MessageType) i.ReadU8 ();
  i.ReadNtohU32 (); //read transaction ID

  size = sizeof (uint8_t) + sizeof (uint32_t);

  switch (m_messageType)
    {
      case PUBLISH:
        size += m_message.publish.Deserialize (i);
        break;
      case SEARCH_REQ:
        size += m_message.searchReq.Deserialize (i);
        break;
      case SEARCH_RSP:
        size += m_message.searchRsp.Deserialize (i);
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

/* PUBLISH */

uint32_t 
PennSearchMessage::Publish::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + publishMessage.length();
  return size;
}

void
PennSearchMessage::Publish::Print (std::ostream &os) const
{
  os << "Publish:: Message: " << publishMessage << "\n";
}

void
PennSearchMessage::Publish::Serialize (BufferV2 &start) const
{
  start.WriteU16 (publishMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (publishMessage.c_str())), publishMessage.length());
}

uint32_t
PennSearchMessage::Publish::Deserialize (BufferV2 &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  publishMessage = std::string (str, length);
  free (str);
  return Publish::GetSerializedSize ();
}

void
PennSearchMessage::SetPublish (std::string publishMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PUBLISH;
    }
  else
    {
      NS_ASSERT (m_messageType == PUBLISH);
    }
  m_message.publish.publishMessage = publishMessage;
}

PennSearchMessage::Publish
PennSearchMessage::GetPublish ()
{
  return m_message.publish;
}

/* END OF PUBLISH */


/* SEARCH_REQ */

uint32_t 
PennSearchMessage::SearchReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t)*4 + queryId.length() + invertedList.length() + pendingTerms.length() + initialReq.length();
  return size;
}

void
PennSearchMessage::SearchReq::Print (std::ostream &os) const
{
  os << "SearchReq:: QueryId: " << queryId << ", InvertedList: {" << invertedList << "}, PendingTerms: {" << pendingTerms << "}\n";
}

void
PennSearchMessage::SearchReq::Serialize (BufferV2 &start) const
{
  start.WriteU16 (queryId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (queryId.c_str())), queryId.length());
  start.WriteU16 (invertedList.length ());
  start.Write ((uint8_t *) (const_cast<char*> (invertedList.c_str())), invertedList.length());
  start.WriteU16 (pendingTerms.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pendingTerms.c_str())), pendingTerms.length());
  start.WriteU16 (initialReq.length ());
  start.Write ((uint8_t *) (const_cast<char*> (initialReq.c_str())), initialReq.length());
}

uint32_t
PennSearchMessage::SearchReq::Deserialize (BufferV2 &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  queryId = std::string (str, length);
  free (str);

  uint16_t length1 = start.ReadU16 ();
  char* str1 = (char*) malloc (length1);
  start.Read ((uint8_t*)str1, length1);
  invertedList = std::string (str1, length1);
  free (str1);

  uint16_t length2 = start.ReadU16 ();
  char* str2 = (char*) malloc (length2);
  start.Read ((uint8_t*)str2, length2);
  pendingTerms = std::string (str2, length2);
  free (str2);

  uint16_t length3 = start.ReadU16 ();
  char* str3 = (char*) malloc (length3);
  start.Read ((uint8_t*)str3, length3);
  initialReq = std::string (str3, length3);
  free (str3);

  return SearchReq::GetSerializedSize ();
}

void
PennSearchMessage::SetSearchReq (std::string queryId, std::string invertedList, std::string pendingTerms, bool initialReq)
{
  if (m_messageType == 0)
    {
      m_messageType = SEARCH_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == SEARCH_REQ);
    }
  m_message.searchReq.queryId = queryId;
  m_message.searchReq.invertedList = invertedList;
  m_message.searchReq.pendingTerms = pendingTerms;
  m_message.searchReq.initialReq = initialReq ? "1" : "0";
}

PennSearchMessage::SearchReq
PennSearchMessage::GetSearchReq ()
{
  return m_message.searchReq;
}

/* END OF SEARCH_REQ */


/* SEARCH_RSP */

uint32_t 
PennSearchMessage::SearchRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t)*2 + queryId.length() + invertedList.length();
  return size;
}

void
PennSearchMessage::SearchRsp::Print (std::ostream &os) const
{
  os << "SearchReq:: QueryId: " << queryId << ", InvertedList: {" << invertedList << "}\n";
}

void
PennSearchMessage::SearchRsp::Serialize (BufferV2 &start) const
{
  start.WriteU16 (queryId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (queryId.c_str())), queryId.length());
  start.WriteU16 (invertedList.length ());
  start.Write ((uint8_t *) (const_cast<char*> (invertedList.c_str())), invertedList.length());
}

uint32_t
PennSearchMessage::SearchRsp::Deserialize (BufferV2 &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  queryId = std::string (str, length);
  free (str);

  uint16_t length1 = start.ReadU16 ();
  char* str1 = (char*) malloc (length1);
  start.Read ((uint8_t*)str1, length1);
  invertedList = std::string (str1, length1);
  free (str1);

  return SearchRsp::GetSerializedSize ();
}

void
PennSearchMessage::SetSearchRsp (std::string queryId, std::string invertedList)
{
  if (m_messageType == 0)
    {
      m_messageType = SEARCH_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == SEARCH_RSP);
    }
  m_message.searchRsp.queryId = queryId;
  m_message.searchRsp.invertedList = invertedList;
}

PennSearchMessage::SearchRsp
PennSearchMessage::GetSearchRsp ()
{
  return m_message.searchRsp;
}

/* END OF SEARCH_RSP */



//
//
//

void
PennSearchMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

PennSearchMessage::MessageType
PennSearchMessage::GetMessageType () const
{
  return m_messageType;
}
