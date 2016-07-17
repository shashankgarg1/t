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
#ifndef WIFI_SPECTRUM_ERROR_MODEL_H
#define WIFI_SPECTRUM_ERROR_MODEL_H

#include "ns3/spectrum-error-model.h"

#include "wifi-mode.h"
#include "wifi-preamble.h"

namespace ns3 {

class WifiSpectrumErrorModel : public SpectrumErrorModel
{
public:
  static TypeId GetTypeId ();
  // inherited from SpectrumErrorModel
  void StartRx (Ptr<const Packet> p);
  void EvaluateChunk (const SpectrumValue &sinr, Time duration);
  bool IsRxCorrect ();
  void SetRxMask (Ptr<SpectrumValue> rxMask);
private:
  WifiMode m_rxMode;
  WifiPreamble m_rxPreamble;
  Time m_rxStart;

  bool m_rxError;
  Ptr<SpectrumValue> m_rxMask;
};

} // namespace ns3

#endif /* WIFI_SPECTRUM_ERROR_MODEL_H */
