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
 *
 * Ported from LTE lte-spectrum-value-helper.h
 */

#ifndef WIFI_SPECTRUM_HELPER_H
#define WIFI_SPECTRUM_HELPER_H

#include <ns3/object.h>
#include <ns3/spectrum-value.h>
#include "wifi-mode.h"

namespace ns3 {

/**
 * \ingroup wifi
 * \brief This class defines all functions to create spectrum model for Wi-Fi
 */
class WifiSpectrumHelper
{
public:
  /*
   * @param channel the IEEE 802.11 channel number
   *
   * @return the center frequency, in MHz, of the specified channel
   */
  static uint16_t GetFreqMHzForChannel (uint16_t channel);

  /*
   * @param mode the WifiMode of interest
   *
   * @return Ptr to the SpectrumModel
   */
  static Ptr<SpectrumModel> GetSpectrumModel (WifiMode mode);

  /*
   *
   * @param txPower the total TX power in W
   * @param channel the number of the channel
   *
   * @return a Ptr to a newly created SpectrumValue instance which
   * represents the TX Power Spectral Density  of a wifi device
   * corresponding to the provided parameters
   */
  static Ptr<SpectrumValue> CreateTxPowerSpectralDensity (double txPower, WifiMode mode, uint32_t channel);

  /*
   *
   * @param noiseFigure the noise figure in dB w.r.t. a reference temperature of 290K
   * @param channel the number of the channel
   *
   * @return a pointer to a newly allocated SpectrumValue representing the noise Power Spectral Density in W/Hz for each Band
   */
  static Ptr<SpectrumValue> CreateNoisePowerSpectralDensity (double noiseFigure, uint32_t channel);

  /*
   *
   * @param channel the number of the channel
   *
   * @return a Ptr to a newly created SpectrumValue instance which
   * represents the frequency response of the RF filter which is used
   * by a wifi device to receive signals when tuned to a particular channel
   */
  static Ptr<SpectrumValue> CreateRfFilter (uint32_t channel);

};

} // namespace ns3

#endif /*  WIFI_SPECTRUM_VALUE_HELPER_H */
