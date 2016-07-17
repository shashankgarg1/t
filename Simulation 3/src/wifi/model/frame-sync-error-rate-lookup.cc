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
#include "ns3/enum.h"
#include <cmath>
#include "frame-sync-error-rate-lookup.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("FrameSyncErrorRateLookup");

NS_OBJECT_ENSURE_REGISTERED (FrameSyncErrorRateLookup);


// Data for signal to noise ratio (SNR) (in dB) to delayed autocorrelation frame
// synchronization error rate (SER) mapping for Additive White Gaussian
// Noise (AWGN) only channel and IEEE TGn Channel Model D (with AWGN).

// AWGN channel
static const unsigned int g_channelAwgnNumSnrSerDatapoints = 28;
static const double g_channelAwgnSnrs[28] = {-10, -9, -8, -7, -6, -5, -4, -3,
                                             -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
                                             9, 10, 11, 12, 13, 14, 15, 16, 17};
static const double g_channelAwgnSers[28] = {1.00, 0.99, 0.99, 0.99, 0.98, 0.96,
                                             0.95, 0.90, 0.81, 0.65, 0.47, 0.29,
                                             0.13, 0.04, 0.01, 0.00, 0.00, 0.00,
                                             0.00, 0.00, 0.00, 0.00, 0.00, 0.00,
                                             0.00, 0.00, 0.00, 0.00};
static uint8_t g_channelAwgnNumSnrDecPlaces = 0;
static const double g_channelAwgnSnrSpacing = 1;
static const double g_channelAwgnSnrMin = -10;
static const double g_channelAwgnSnrMax = 17;

// IEEE TGn Channel Model D
static const unsigned int g_channelDNumSnrSerDatapoints = 28;
static const double g_channelDSnrs[28] = {-10, -9, -8, -7, -6, -5, -4, -3, -2,
                                          -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                          11, 12, 13, 14, 15, 16, 17};
static const double g_channelDSers[28] = {1.00, 0.99, 0.99, 0.98, 0.98, 0.96,
                                          0.92, 0.87, 0.78, 0.67, 0.54, 0.39,
                                          0.27, 0.19, 0.10, 0.06, 0.03, 0.01,
                                          0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
                                          0.01, 0.00, 0.00, 0.00};
static uint8_t g_channelDNumSnrDecPlaces = 0;
static const double g_channelDSnrSpacing = 1;
static const double g_channelDSnrMin = -10;
static const double g_channelDSnrMax = 17;


TypeId
FrameSyncErrorRateLookup::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::FrameSyncErrorRateLookup")
    .SetParent<Object> ()
    .AddConstructor<FrameSyncErrorRateLookup> ()
    .AddAttribute ("ChannelModel",
                   "The channel model to use for frame sync error rates.",
                   EnumValue (WifiPhy::AWGN),
                   MakeEnumAccessor (&FrameSyncErrorRateLookup::m_channelModel),
                   MakeEnumChecker (WifiPhy::AWGN, "awgn",
                                    WifiPhy::D, "d",
                                    WifiPhy::USER_DEFINED, "user"))
  ;
  return tid;
}


FrameSyncErrorRateLookup::FrameSyncErrorRateLookup ()
  : m_channelModel (WifiPhy::AWGN),
    m_errorRateTableIsLoaded (false),
    m_numSnrDecPlaces (0),
    m_numSnrDecPlacesIsSet (false),
    m_snrSpacing (1),
    m_snrSpacingIsSet (false),
    m_snrMin (-10),
    m_snrMax (17),
    m_snrOffset (0),
    m_snrOffsetIsSet (false)
{
  NS_LOG_FUNCTION (this);
}


FrameSyncErrorRateLookup::~FrameSyncErrorRateLookup ()
{
  NS_LOG_FUNCTION (this);
}


void
FrameSyncErrorRateLookup::SetNumSnrDatapointDecPlaces (double numDecimalPlaces)
{
  NS_LOG_FUNCTION (this << numDecimalPlaces);
  NS_ASSERT_MSG (m_channelModel == WifiPhy::USER_DEFINED,
                 "ChannelModel attribute must be set to 'user'"
                 " to manually add the number of SNR decimal places");
  m_numSnrDecPlaces = numDecimalPlaces;
  m_numSnrDecPlacesIsSet = true;
}


void
FrameSyncErrorRateLookup::SetSnrDatapointSpacing (double snrDatapointSpacing)
{
  NS_LOG_FUNCTION (this << snrDatapointSpacing);
  NS_ASSERT_MSG (m_channelModel == WifiPhy::USER_DEFINED,
                 "ChannelModel attribute must be set to 'user'"
                 " to manually add the SNR datapoint spacing");
  m_snrSpacing = snrDatapointSpacing;
  m_snrSpacingIsSet = true;
}


void
FrameSyncErrorRateLookup::AddSnrSerDatapoint (double snr, double ser)
{
  NS_LOG_FUNCTION (this << snr << ser);
  NS_ASSERT_MSG (m_channelModel == WifiPhy::USER_DEFINED,
                 "ChannelModel attribute must be set to 'user'"
                 " to manually add datapoints");
  NS_ASSERT_MSG (m_numSnrDecPlacesIsSet, "number of decimal places of SNR"
                                         " datapoints must be set before"
                                         " manually adding datapoints");
  if (m_frameSyncErrorRateTable.empty ())
    {
      m_snrMin = snr;
      m_snrMax = snr;
    }
  else if (snr < m_snrMin)
    {
      m_snrMin = snr;
    }
  else if (snr > m_snrMax)
    {
      m_snrMax = snr;
    }
  std::pair<std::map<int, double>::iterator, bool> ret;
  int snrInt = DoubleToHashKeyInt (snr); //SNR converted to an integer to use as hash key
  ret = m_frameSyncErrorRateTable.insert (std::pair<int, double> (snrInt, ser)); //add datapoint to hash map
  NS_ASSERT_MSG (ret.second, "element with snr hash of " << snrInt <<
                             " already exists in frame sync error rate hash map with value of "
                             << ret.first->second);
  m_errorRateTableIsLoaded = true;
}


double
FrameSyncErrorRateLookup::GetFrameSyncErrorRate (double snr)
{
  NS_LOG_FUNCTION (this << snr);
  if (!m_errorRateTableIsLoaded && m_channelModel != WifiPhy::USER_DEFINED)
    {
      LoadErrorRateDataToTable (); //load error rates the first time this method is called
    }
  else if (m_channelModel == WifiPhy::USER_DEFINED)
    {
      NS_ASSERT_MSG (m_errorRateTableIsLoaded,
                     "At least one datapoint must be added using AddSnrSerDatapoint()"
                     " when ChannelModel is set to 'user'");
      NS_ASSERT_MSG (m_numSnrDecPlacesIsSet,
                     "Number of SNR datapoint decimal places must be manually set"
                     " when ChannelModel is set to 'user'");
      NS_ASSERT_MSG (m_snrSpacingIsSet,
                     "SNR datapoint spacing must be manually set"
                     " when ChannelModel is set to 'user'");
    }
  if (!m_snrOffsetIsSet)
    {
      DetermineSnrOffset ();
    }
  bool snrAtBound = false;
  if (snr <= m_snrMin)
    {
      NS_LOG_DEBUG ("SNR is lower than minimum frame sync error rate datapoint;"
                    " setting SNR to " << m_snrMin << " (min SNR)");
      snr = m_snrMin;
      snrAtBound = true;
    }
  else if (snr >= m_snrMax)
    {
      NS_LOG_DEBUG ("SNR is higher than maximum frame sync error rate datapoint;"
                    " setting SNR to " << m_snrMax << " (max SNR)");
      snr = m_snrMax;
      snrAtBound = true;
    }
  InterpolParams iParams;
  double ser; //sync error rate to return
  iParams = FindDatapointBounds (snr, snrAtBound);
  ser = InterpolateAndRetrieveData (snr, iParams.snrLoBound, iParams.snrHiBound);
  return ser;
}


int
FrameSyncErrorRateLookup::RoundDoubleToInt (double val)
{
  NS_LOG_FUNCTION (this << val);
  if (val > 0.0)
    {
      return val + 0.5;
    }
  else
    {
      return val - 0.5;
    }
}


int
FrameSyncErrorRateLookup::DoubleToHashKeyInt (double val)
{
  NS_LOG_FUNCTION (this << val);
  for (uint8_t i = 0; i < m_numSnrDecPlaces; i++)
    {
      val = val * 10;
    }
  return (int)(round (val));
}


void
FrameSyncErrorRateLookup::LoadErrorRateDataToTable ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_channelModel != WifiPhy::USER_DEFINED,
                 "ChannelModel attribute cannot be set to 'user'"
                 " if loading from default error rate models");
  NS_ASSERT_MSG (!m_errorRateTableIsLoaded, "frame sync error rate table has"
                                            " already been loaded");
  std::pair<std::map<int, double>::iterator, bool> ret;
  int snrInt; //SNR converted to an integer to use as hash key
  if (m_channelModel == WifiPhy::AWGN)
    {
      m_numSnrDecPlaces = g_channelAwgnNumSnrDecPlaces;
      m_numSnrDecPlacesIsSet = true;
      m_snrSpacing = g_channelAwgnSnrSpacing;
      m_snrSpacingIsSet = true;
      m_snrMin = g_channelAwgnSnrMin;
      m_snrMax = g_channelAwgnSnrMax;
      for (unsigned int i = 0; i < g_channelAwgnNumSnrSerDatapoints; i++)
        {
          snrInt = DoubleToHashKeyInt (g_channelAwgnSnrs[i]);
          ret = m_frameSyncErrorRateTable.insert (std::pair<int, double> (snrInt, g_channelAwgnSers[i])); //add datapoint to hash map
          NS_ASSERT_MSG (ret.second, "element with snr hash of " << snrInt <<
                                     " already exists in frame sync error rate hash map with value of "
                                     << ret.first->second);
        }
    }
  else if (m_channelModel == WifiPhy::D)
    {
      m_numSnrDecPlaces = g_channelDNumSnrDecPlaces;
      m_numSnrDecPlacesIsSet = true;
      m_snrSpacing = g_channelDSnrSpacing;
      m_snrSpacingIsSet = true;
      m_snrMin = g_channelDSnrMin;
      m_snrMax = g_channelDSnrMax;
      for (unsigned int i = 0; i < g_channelDNumSnrSerDatapoints; i++)
        {
          snrInt = DoubleToHashKeyInt (g_channelDSnrs[i]);
          ret = m_frameSyncErrorRateTable.insert (std::pair<int, double> (snrInt, g_channelDSers[i])); //add datapoint to hash map
          NS_ASSERT_MSG (ret.second, "element with snr hash of " << snrInt <<
                                     " already exists in frame sync error rate hash map with value of "
                                     << ret.first->second);
        }
    }
  else
    {
      NS_FATAL_ERROR ("specified ChannelModel is unsupported");
    }
  m_errorRateTableIsLoaded = true;
}


void
FrameSyncErrorRateLookup::DetermineSnrOffset ()
{
  NS_LOG_FUNCTION (this);
  double snr = m_snrMin;
  double toleranceForAssumingZero = 0.0001;
  for (int i = 0; i < m_numSnrDecPlaces; i++)
    {
      toleranceForAssumingZero /= 10;
    }
  if (m_snrMin < 0)
    {
      for (; snr <= 2 * m_snrSpacing; snr += m_snrSpacing)
        {
          if (snr >= 0)
            {
              m_snrOffset = snr;
              break;
            }
        }
      if (std::abs (snr) <= toleranceForAssumingZero) //if less than tolerance assume 0
        {
          m_snrOffset = 0;
        }
    }
  else if (m_snrMin > 0)
    {
      for (; snr >= -2 * m_snrSpacing; snr -= m_snrSpacing)
        {
          if (snr <= 0)
            {
              m_snrOffset = snr;
              break;
            }
        }
      if (std::abs (snr) <= toleranceForAssumingZero) //if less than tolerance assume 0
        {
          m_snrOffset = 0;
        }
    }
  else //if m_snrMin == 0
    {
      m_snrOffset = 0;
    }
  m_snrOffsetIsSet = true;
}


double
FrameSyncErrorRateLookup::RoundToNearestDataMidpoint (double value, double dataSpacing, double offset)
{
  NS_LOG_FUNCTION (this << value << dataSpacing << offset);
  double newOffset = (dataSpacing / 2) + offset;
  return (RoundDoubleToInt ((value - newOffset) / dataSpacing) * dataSpacing) + newOffset;
}


FrameSyncErrorRateLookup::InterpolParams
FrameSyncErrorRateLookup::FindDatapointBounds (double snr, bool snrAtBound)
{
  NS_LOG_FUNCTION (this << snr << snrAtBound);
  InterpolParams iParams;
  if (snrAtBound)
    {
      iParams.snrLoBound = snr;
      iParams.snrHiBound = snr;
    }
  else
    {
      double snrRatio = snr / m_snrSpacing;
      double snrLoBoundNoOffset = floor (snrRatio) * m_snrSpacing;
      double snrHiBoundNoOffset = ceil (snrRatio) * m_snrSpacing;
      double nearestMidSnrPoint = RoundToNearestDataMidpoint (snr, m_snrSpacing, m_snrOffset);
      if (snr > nearestMidSnrPoint)
        {
          iParams.snrLoBound = snrLoBoundNoOffset - m_snrOffset;
          iParams.snrHiBound = snrHiBoundNoOffset - m_snrOffset;
        }
      else
        {
          iParams.snrLoBound = snrLoBoundNoOffset + m_snrOffset;
          iParams.snrHiBound = snrHiBoundNoOffset + m_snrOffset;
        }
    }
  return iParams;
}


double
FrameSyncErrorRateLookup::InterpolateAndRetrieveData (double xd, double x1d, double x2d)
{
  NS_LOG_FUNCTION (this << xd << x1d << x2d);
  int x = DoubleToHashKeyInt (xd);
  int x1 = DoubleToHashKeyInt (x1d);
  int x2 = DoubleToHashKeyInt (x2d);
  std::map<int, double>::iterator elemIter;
  double fp; //retrieved value after any interpolation
  if (x1 != x2) //1D linear interpolation (x)
    {
      NS_LOG_DEBUG ("performing linear interpolation on snr for frame sync error rate lookup");
      double fq1;
      double fq2;
      elemIter = m_frameSyncErrorRateTable.find (x1);
      if (elemIter == m_frameSyncErrorRateTable.end ())
        {
          NS_FATAL_ERROR ("No frame sync error rate data stored for snr key = " << x1);
        }
      else
        {
          fq1 = (*elemIter).second;
        }
      elemIter = m_frameSyncErrorRateTable.find (x2);
      if (elemIter == m_frameSyncErrorRateTable.end ())
        {
          NS_FATAL_ERROR ("No frame sync error rate data stored for snr key = " << x2);
        }
      else
        {
          fq2 = (*elemIter).second;
        }
      fp = (((x2d - xd) / (x2d - x1d)) * fq1) + (((xd - x1d) / (x2d - x1d)) * fq2);
    }
  else //x1 == x2; no interpolation needed
    {
      NS_LOG_DEBUG ("no interpolation needed--direct table lookup for frame sync error rate");
      elemIter = m_frameSyncErrorRateTable.find (x);
      if (elemIter == m_frameSyncErrorRateTable.end ())
        {
          NS_FATAL_ERROR ("No frame sync error rate data stored for snr key = " << x);
        }
      else
        {
          fp = (*elemIter).second;
        }
    }
  return fp;
}

} // namespace ns3
