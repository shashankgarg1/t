/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
 * Copyright (c) 2015 University of Washington
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
 *
 * Author: Tom Henderson <tomhend@u.washington.edu>
 * adapted from Wi-Fi QosTag written by Mirko Banchi <mk.banchi@gmail.com>
 */

#ifndef CLASS_OF_SERVICE_TAG_H
#define CLASS_OF_SERVICE_TAG_H

#include "ns3/packet.h"

namespace ns3 {

class Tag;

/**
 * IEEE P802.1p definitions
 */
enum ClassOfService
{
  COS_BK = 1, /**< background  */
  COS_BE = 0, /**< best effort (default) */
  COS_EE = 2, /**< excellent effort  */
  COS_CA = 3, /**< critical applications */
  COS_VI = 4, /**< video, < 100ms latency and jitter */
  COS_VO = 5, /**< voice, < 10ms latency and jitter */
  COS_IC = 6, /**< internetwork control */
  COS_NC = 7  /**< network control */
};

/**
 * \ingroup network
 *
 * Allow network applications to specify a class of service associated
 * with a packet sent to a NetDevice
 */
class ClassOfServiceTag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Create a ClassOfServiceTag with the default class = 0 (best effort traffic)
   */
  ClassOfServiceTag ();

  /**
   * Create a ClassOfServiceTag with the given class of service
   *
   * \param cos the given class of service
   */
  ClassOfServiceTag (uint8_t cos);

  /**
   * Set the class of service to the given value. 
   *
   * \param cos the given class of service
   */
  void SetClassOfService (uint8_t cos);

  /**
   * Return the class of service
   *
   * \return class of service
   */
  uint8_t GetClassOfService (void) const;

  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual uint32_t GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_cos;  //!< class of service
};

} //namespace ns3

#endif /* QOS_TAG_H */
