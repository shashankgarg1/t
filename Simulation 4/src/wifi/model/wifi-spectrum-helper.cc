/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Nicola Baldo <nbaldo@cttc.es>
 *
 * Ported/adapted from LTE lte-spectrum-value-helper.cc
 */

#include "ns3/log.h"
#include "wifi-spectrum-helper.h"
#include "wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiSpectrumHelper");

// convert this to map as in LTE in the future
static Ptr<SpectrumModel> g_wifiSpectrumModel = 0;

uint16_t 
WifiSpectrumHelper::GetFreqMHzForChannel (uint16_t channel)
{
  NS_LOG_FUNCTION_NOARGS ();
  if ((channel == 36) || (channel == 40) || (channel == 44) || (channel == 48))
    {
       return 5180 + 5 * (channel - 36);
    }
  else
    {
       NS_FATAL_ERROR ("Only channel 36, 40, 44, and 48 supported initially");
       return 5180;
    }
  // 2.4 GHz channels 1-14 (per IEEE Std. 802.11-2007, Table 18-9) 
  if (channel >= 1 && channel < 14)
    {
      return 2407 + (channel * 5);
    }
  // Channel 14 is a special case at 2484 MHz
  if (channel == 14)
    {
      return 2484;
    }
}

Ptr<SpectrumModel>
WifiSpectrumHelper::GetSpectrumModel (WifiMode mode)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (g_wifiSpectrumModel)
    {
      return g_wifiSpectrumModel;
    }
  Bands bands;
  // Channel 36 is 5180 MHz, channel 165 is 5825 Mhz.
  // Need 725 MHz to span this including 40 MHz guard bands
  double bandwidth = 725e6; // 150 MHz 5.180-5.825 GHz +/- 40 MHz
  double bandBandwidth = 5e6; // 5 Mhz resource blocks:  145 bands
  double startingFreq = 5140e6;
  for (uint16_t i = 0; i < bandwidth/bandBandwidth; i++)
    {
      BandInfo info;
      double f = startingFreq + (i * bandBandwidth);
      info.fl = f;
      f += 2500e3;
      info.fc = f;
      f += 2500e3;
      info.fh = f;
      bands.push_back (info);
    }
  g_wifiSpectrumModel = Create<SpectrumModel> (bands);
  // when more than one model supported, insert into map such as in LTE
  return g_wifiSpectrumModel;
}

Ptr<SpectrumValue> 
WifiSpectrumHelper::CreateTxPowerSpectralDensity (double txPower, WifiMode mode, uint32_t channel)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<SpectrumValue> c = Create <SpectrumValue> (GetSpectrumModel (WifiPhy::GetOfdmRate6Mbps ()));
  (*c) = 0;
  if ((channel != 36) && (channel != 40) && (channel != 44) && (channel != 48))
    {
      NS_FATAL_ERROR ("Only channels 36, 40, 44, and 48 supported initially");
    }
  // Spread txPower across 20 MHz centered on channel center frequency
  // For now, spread evenly across four 5MHz bands; skip past first six bands  
  double txPowerPerBand = txPower/4;
  Values::iterator vit = c->ValuesBegin ();
  Bands::const_iterator bit = c->ConstBandsBegin ();
  for (uint16_t i = 0; i < c->GetSpectrumModel ()->GetNumBands (); i++, vit++, bit++)
    {
      if (channel == 36)
        {
          if (i >= 6 && i <= 9)
            {
              *vit = txPowerPerBand / (bit->fh - bit->fl);
            }
        }
      else if (channel == 40)
        {
          if (i >= 10 && i <= 13)
            {
              *vit = txPowerPerBand / (bit->fh - bit->fl);
            }
        }
      else if (channel == 44)
        {
          if (i >= 14 && i <= 17)
            {
              *vit = txPowerPerBand / (bit->fh - bit->fl);
            }
        }
      else if (channel == 48)
        {
          if (i >= 18 && i <= 21)
            {
              *vit = txPowerPerBand / (bit->fh - bit->fl);
            }
        }
    }
  return c;
}

Ptr<SpectrumValue> 
WifiSpectrumHelper::CreateNoisePowerSpectralDensity (double noiseFigureDb, uint32_t channel)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<SpectrumValue> c = Create <SpectrumValue> (GetSpectrumModel (WifiPhy::GetOfdmRate6Mbps ()));
  // From LteSpectrumValueHelper
  const double kT_dBm_Hz = -174.0;  // dBm/Hz
  double kT_W_Hz = std::pow (10.0, (kT_dBm_Hz - 30) / 10.0);
  double noiseFigureLinear = std::pow (10.0, noiseFigureDb / 10.0);
  double noisePowerSpectralDensity =  kT_W_Hz * noiseFigureLinear;
  (*c) = noisePowerSpectralDensity;
  return c;
}

Ptr<SpectrumValue> 
WifiSpectrumHelper::CreateRfFilter (uint32_t channel)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<SpectrumValue> c = Create <SpectrumValue> (GetSpectrumModel (WifiPhy::GetOfdmRate6Mbps ()));
  (*c) = 0;
  if ((channel != 36) && (channel != 40) && (channel != 44) && (channel != 48))
    {
      NS_FATAL_ERROR ("Only channels 36, 40, 44, and 48 supported initially");
    }
  // For now, perfect 20Mhz-wide window
  Values::iterator vit = c->ValuesBegin ();
  for (uint16_t i = 0; i < c->GetSpectrumModel ()->GetNumBands (); i++, vit++)
    {
      if (channel == 36)
        {
          if (i >= 6 && i <= 9)
            {
              *vit = 1;
            }
        }
      else if (channel == 40)
        {
          if (i >= 10 && i <= 13)
            {
              *vit = 1;
            }
        }
      else if (channel == 44)
        {
          if (i >= 14 && i <= 17)
            {
              *vit = 1;
            }
        }
      else if (channel == 48)
        {
          if (i >= 18 && i <= 21)
            {
              *vit = 1;
            }
        }
    }
  return c;
}

} // namespace ns3
