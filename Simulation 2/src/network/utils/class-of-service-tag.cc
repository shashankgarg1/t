/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
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
 *
 */

#include "class-of-service-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ClassOfServiceTag);

TypeId
ClassOfServiceTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ClassOfServiceTag")
    .SetParent<Tag> ()
    .SetGroupName ("Network")
    .AddConstructor<ClassOfServiceTag> ()
    .AddAttribute ("Class", "The class of service",
                   UintegerValue (0),
                   MakeUintegerAccessor (&ClassOfServiceTag::GetClassOfService),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
ClassOfServiceTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

ClassOfServiceTag::ClassOfServiceTag ()
  : m_cos (0)
{
}
ClassOfServiceTag::ClassOfServiceTag (uint8_t cos)
  : m_cos (cos)
{
}

void
ClassOfServiceTag::SetClassOfService (uint8_t cos)
{
  m_cos = cos;
}

uint32_t
ClassOfServiceTag::GetSerializedSize (void) const
{
  return 1;
}

void
ClassOfServiceTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_cos);
}

void
ClassOfServiceTag::Deserialize (TagBuffer i)
{
  m_cos = (uint8_t) i.ReadU8 ();
}

uint8_t
ClassOfServiceTag::GetClassOfService () const
{
  return m_cos;
}

void
ClassOfServiceTag::Print (std::ostream &os) const
{
  os << "CoS=" << m_cos;
}

} // namespace ns3
