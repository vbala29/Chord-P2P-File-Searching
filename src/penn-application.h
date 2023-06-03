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

#ifndef PENN_APPLICATION_H
#define PENN_APPLICATION_H

#include <map>
#include <sstream>
#include <vector>
#include <pthread.h>
#include "penn-log.h"
#include "ipv4.hpp"

class PennApplication : public PennLog
{
public:
  PennApplication ();
  virtual ~PennApplication ();
  virtual std::map<std::string, pthread_t> StartApplication (std::map<uint32_t, Ipv4Address> m_nodeAddressMap, std::map<Ipv4Address, uint32_t> m_addressNodeMap,  Ipv4Address m_local, std::string nodeId) {
    return std::map<std::string, pthread_t>();
  };

  // Interface for PennApplication(s)
  virtual void ProcessCommand (std::vector<std::string> tokens) = 0;
  virtual void SetNodeAddressMap (std::map<uint32_t, Ipv4Address> nodeAddressMap);
  virtual void SetAddressNodeMap (std::map<Ipv4Address, uint32_t> addressNodeMap);
  void SetLocalAddress (Ipv4Address local);
  Ipv4Address GetLocalAddress();
  virtual Ipv4Address ResolveNodeIpAddress (std::string nodeId);
  virtual std::string ReverseLookup (Ipv4Address ipv4Address); 

protected:

  Ipv4Address m_local;
  std::map<uint32_t, Ipv4Address> m_nodeAddressMap;
  std::map<Ipv4Address, uint32_t> m_addressNodeMap;
private:
  virtual void StopApplication (void) = 0;
};

#endif
