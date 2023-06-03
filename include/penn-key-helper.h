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

#ifndef PENN_KEY_HELPER_H
#define PENN_KEY_HELPER_H

#include "ipv4.hpp"
#include <map>
#include <string>
#include <ios>
#include <iomanip>
#include <openssl/sha.h>


#define DIGEST_LENGTH 20

class PennKeyHelper
{
public:
    /**
     * @brief Create a 32-bit hash key from a string term.
     *
     * @param term
     * @return uint32_t
     */
    static uint32_t CreateShaKey(const std::string &term)
    {
        unsigned char md[DIGEST_LENGTH];
        SHA1((const unsigned char *)term.c_str(), term.length(), md);
        uint32_t key = md[0] | (md[1] << 8) | (md[2] << 16) | (md[3] << 24);
        return key;
    }

    /**
     * @brief Create a 32-bit hash key from a node's IP address.
     *
     * @param ip
     * @return uint32_t
     */
    static uint32_t CreateShaKey(const Ipv4Address &ip, std::map<Ipv4Address, uint32_t> m_addressNodeMap)
    {
        std::stringstream ss;
        ss << m_addressNodeMap.at(ip);
        return CreateShaKey(ss.str());
    }

    /**
     * @brief Convert the 32-bit hash key to a hex string.
     * Use for printing ringstate.
     *
     * @param key
     * @return std::string
     */
    static std::string KeyToHexString(const uint32_t key)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(8) << std::hex << key;
        return ss.str();
    }
};

#endif