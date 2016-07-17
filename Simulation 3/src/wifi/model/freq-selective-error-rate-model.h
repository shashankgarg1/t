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

#ifndef FREQ_SELECTIVE_ERROR_RATE_MODEL_H
#define FREQ_SELECTIVE_ERROR_RATE_MODEL_H

#include <map>
#include "wifi-mode.h"
#include "error-rate-model.h"

namespace ns3
{

/**
 * \ingroup wifi
 *
 * A frequency selective error rate model based on curve-fitted link simulation
 * results for different MCSs and TX/RX antenna configurations for TGn channel
 * model D
 */
class FreqSelectiveErrorRateModel : public ErrorRateModel
{
public:
  static TypeId GetTypeId (void);

  FreqSelectiveErrorRateModel (); //!< Default constructor
  virtual ~FreqSelectiveErrorRateModel (); //!< Destructor

  virtual double GetChunkSuccessRate (WifiMode mode, WifiTxVector txVector, double snr, uint32_t nbits) const;

  /** 
   * Set number of RX antennas for correct error rates to be looked up
   *
   * @param numRxAntennas the number of RX antennas
   */
  void SetNumRxAntennas (uint32_t numRxAntennas);

private:
  /**
  * Get the bit error rate for the specified SNR (in dB) and MCS index based on
  * the number of RX antennas configured.
  *
  * @param snrDb the signal to noise ratio in dB
  * @param mode the modulation and coding scheme mode
  *
  * @return the retrieved bit error rate
  */
  double GetBitErrorRate (double snrDb, WifiMode mode) const;

  uint32_t m_numRxAntennas; //!< Number of RX antennas for the node's PHY

  Ptr<ErrorRateModel> m_backupErrorRateModel;
};

} // namespace ns3

#endif /* FREQ_SELECTIVE_ERROR_RATE_MODEL_H*/
