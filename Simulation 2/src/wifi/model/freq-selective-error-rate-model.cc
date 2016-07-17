/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Benjamin Cizdziel <ben.cizdziel@gmail.com>
 */

#include "ns3/log.h"
#include <cmath>
#include "freq-selective-error-rate-model.h"
#include "nist-error-rate-model.h"
#include "wifi-phy.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("FreqSelectiveErrorRateModel");

NS_OBJECT_ENSURE_REGISTERED (FreqSelectiveErrorRateModel);


TypeId
FreqSelectiveErrorRateModel::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::FreqSelectiveErrorRateModel")
    .SetParent<ErrorRateModel> ()
    .SetGroupName ("Wifi")
    .AddConstructor<FreqSelectiveErrorRateModel> ()
  ;
  return tid;
}


FreqSelectiveErrorRateModel::FreqSelectiveErrorRateModel ()
  : m_numRxAntennas (1)
{
  NS_LOG_FUNCTION (this);
  m_backupErrorRateModel = CreateObject<NistErrorRateModel> ();
}


FreqSelectiveErrorRateModel::~FreqSelectiveErrorRateModel ()
{
  NS_LOG_FUNCTION (this);
}


double
FreqSelectiveErrorRateModel::GetChunkSuccessRate (WifiMode mode, WifiTxVector txVector, double snr, uint32_t nbits) const
{
  NS_LOG_FUNCTION (this << mode << snr << nbits);
  NS_ASSERT (nbits > 0);
  if (mode.GetModulationClass () != WIFI_MOD_CLASS_HT)
    {
      return m_backupErrorRateModel->GetChunkSuccessRate (mode, txVector, snr, nbits);
    }
  NS_ASSERT_MSG (txVector.GetChannelWidth () == 20, "only 20 MHz bandwidth channels currently supported");
  NS_ASSERT_MSG (snr > 0, "SNR ratio must be greater than 0");
  snr = 10.0 * std::log10 (snr); //convert SNR from ratio to dB
  uint32_t ss = txVector.GetNss ();
  if (ss > 2)
    {
      NS_FATAL_ERROR ("No support for " << ss << " spatial streams");
    }
  double ber = GetBitErrorRate (snr, mode);
  return std::pow (1 - ber, nbits);
}


void
FreqSelectiveErrorRateModel::SetNumRxAntennas (uint32_t numRxAntennas)
{
  NS_LOG_FUNCTION (this << numRxAntennas);
  m_numRxAntennas = numRxAntennas;
}


double
FreqSelectiveErrorRateModel::GetBitErrorRate (double snrDb, WifiMode mode) const
{
  NS_ASSERT_MSG (m_numRxAntennas <= 2, "only up to 2 RX antennas currently supported");
  double snrDbMin = 0;
  double p1 = 0;
  double p2 = 0;
  if (m_numRxAntennas == 2)
    {
      //1 spatial stream, 2 RX antennas, from UW link sim
      if (mode == WifiPhy::GetHtMcs0 ())
        {
            snrDbMin = -5.1;
            p1 = -0.300827;
            p2 = -3.756085;
        }
      else if (mode == WifiPhy::GetHtMcs1 ())
        {
            snrDbMin = -1.6;
            p1 = -0.309238;
            p2 = -2.965011;
        }
      else if (mode == WifiPhy::GetHtMcs2 ())
        {
            snrDbMin = 1.4;
            p1 = -0.237281;
            p2 = -2.405734;
        }
      else if (mode == WifiPhy::GetHtMcs3 ())
        {
            snrDbMin = 3.3;
            p1 = -0.359498;
            p2 = -1.821525;
        }
      else if (mode == WifiPhy::GetHtMcs4 ())
        {
            snrDbMin = 7.3;
            p1 = -0.246839;
            p2 = -1.573179;
        }
      else if (mode == WifiPhy::GetHtMcs5 ())
        {
            snrDbMin = 11.3;
            p1 = -0.239672;
            p2 = -0.664214;
        }
      else if (mode == WifiPhy::GetHtMcs6 ())
        {
            snrDbMin = 12.6;
            p1 = -0.249975;
            p2 = -0.241742;
        }
      else if (mode == WifiPhy::GetHtMcs7 ())
        {
            snrDbMin = 14.8;
            p1 = -0.250718;
            p2 = 0.350541;
        }
          //2 spatial streams, 2 RX antennas, based on Figure 10 of
          //S. A. Mujtaba, "TGnSync proposal PHY results," IEEE, Tech. Rep., May 2005.
      else if (mode == WifiPhy::GetHtMcs8 ())
        {
            snrDbMin = 6.0;
            p1 = -0.331433;
            p2 = -1.707058;
        }
      else if (mode == WifiPhy::GetHtMcs9 ())
        {
            snrDbMin = 9.0;
            p1 = -0.327923;
            p2 = -0.512829;
        }
      else if (mode == WifiPhy::GetHtMcs10 ())
        {
            snrDbMin = 12.0;
            p1 = -0.250134;
            p2 = -0.312513;
        }
      else if (mode == WifiPhy::GetHtMcs11 ())
        {
            snrDbMin = 15.0;
            p1 = -0.293255;
            p2 = 1.055878;
        }
      else if (mode == WifiPhy::GetHtMcs12 ())
        {
            snrDbMin = 18.0;
            p1 = -0.225802;
            p2 = 0.830230;
        }
      else if (mode == WifiPhy::GetHtMcs13 ())
        {
            snrDbMin = 24.0;
            p1 = -0.265136;
            p2 = 2.807394;
        }
      else if (mode == WifiPhy::GetHtMcs14 ())
        {
            snrDbMin = 24.0;
            p1 = -0.229463;
            p2 = 2.290292;
        }
      else if (mode == WifiPhy::GetHtMcs15 ())
        {
            snrDbMin = 27.0;
            p1 = -0.159535;
            p2 = 0.688334;
        }
      else
        {
            NS_FATAL_ERROR ("unsupported MCS for bit error rate lookup");
        }
    }
  else //m_numRxAntennas == 1
    {
      if (mode == WifiPhy::GetHtMcs0 ())
        {
          //1 spatial stream, 1 RX antenna, from UW link sim. TODO: run more linksim iterations for 1x1
            snrDbMin = -1.0;
            p1 = -0.187098;
            p2 = -2.378001;
        }
      else if (mode == WifiPhy::GetHtMcs1 ())
        {
            snrDbMin = 3.0;
            p1 = -0.184590;
            p2 = -1.973042;
        }
      else if (mode == WifiPhy::GetHtMcs2 ())
        {
            snrDbMin = 6.0;
            p1 = -0.137791;
            p2 = -1.841526;
        }
      else if (mode == WifiPhy::GetHtMcs3 ())
        {
            snrDbMin = 7.0;
            p1 = -0.223005;
            p2 = -1.841909;
        }
      else if (mode == WifiPhy::GetHtMcs4 ())
        {
            snrDbMin = 11.0;
            p1 = -0.164868;
            p2 = -1.540567;
        }
      else if (mode == WifiPhy::GetHtMcs5 ())
        {
            snrDbMin = 15.0;
            p1 = -0.152820;
            p2 = -1.347143;
        }
      else if (mode == WifiPhy::GetHtMcs6 ())
        {
            snrDbMin = 17.0;
            p1 = -0.151264;
            p2 = -1.188952;
        }
      else if (mode == WifiPhy::GetHtMcs7 ())
        {
            snrDbMin = 19.0;
            p1 = -0.161418;
            p2 = -0.353079;
        }
      else 
        {
            NS_FATAL_ERROR ("unsupported MCS for bit error rate lookup");
        }
    }
  double ber;
  if (snrDb < snrDbMin)
    {
      ber = 1; //bit error rate = 1 if SNR is below minimum
    }
  else
    {
      ber = std::pow (10, p1 * snrDb + p2); //log linear curve (ber = 10 ^ (p1*snrDb + p2))
    }
  return ber;
}


} // namespace ns3
