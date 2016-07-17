/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Dean Armstrong
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
 * Author: Dean Armstrong <deanarm@gmail.com>
 */
#include "wifi-spectrum-error-model.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "wifi-phy-tag.h"

NS_LOG_COMPONENT_DEFINE ("WifiSpectrumErrorModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WifiSpectrumErrorModel);

TypeId
WifiSpectrumErrorModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::WifiSpectrumErrorModel")
    .SetParent<SpectrumErrorModel> ()
    .SetGroupName ("Wifi")
    ;
  return tid;
}

void
WifiSpectrumErrorModel::StartRx (Ptr<const Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  WifiPhyTag tag;
  p->PeekPacketTag (tag);

  m_rxMode = tag.GetWifiTxVector ().GetMode ();
  m_rxPreamble = tag.GetWifiPreamble ();
  m_rxStart = Simulator::Now ();

  m_rxError = false;
}

void
WifiSpectrumErrorModel::SetRxMask (Ptr<SpectrumValue> rxMask)
{
  m_rxMask = rxMask;
}

void
WifiSpectrumErrorModel::EvaluateChunk (const SpectrumValue &sinr, Time duration)
{
  NS_LOG_FUNCTION (this << sinr << duration);

  if (Sum ((*m_rxMask) * sinr) < 1)
    {
      m_rxError = true;
    }
}

bool
WifiSpectrumErrorModel::IsRxCorrect ()
{
  NS_LOG_FUNCTION (this);
  return !m_rxError;
}

} // namespace ns3
