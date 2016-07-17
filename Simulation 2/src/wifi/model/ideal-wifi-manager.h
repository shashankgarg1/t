/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef IDEAL_WIFI_MANAGER_H
#define IDEAL_WIFI_MANAGER_H

#include <stdint.h>
#include <vector>
#include "ns3/traced-value.h"
#include "wifi-mode.h"
#include "wifi-remote-station-manager.h"

namespace ns3 {

/**
 * \brief Ideal rate control algorithm
 * \ingroup wifi
 *
 * This class implements an 'ideal' rate control algorithm
 * similar to RBAR in spirit (see <i>A rate-adaptive MAC
 * protocol for multihop wireless networks</i> by G. Holland,
 * N. Vaidya, and P. Bahl.): every station keeps track of the
 * snr of every packet received and sends back this snr to the
 * original transmitter by an out-of-band mechanism. Each
 * transmitter keeps track of the last snr sent back by a receiver
 * and uses it to pick a transmission mode based on a set
 * of snr thresholds built from a target ber and transmission
 * mode-specific snr/ber curves.
 *
 * This model does not presently account for WifiPhy configuration 
 * changes that may occur during runtime (after object initialization time),
 * including changing the number of antennas, the guard interval, or the
 * channel width.
 */
class IdealWifiManager : public WifiRemoteStationManager
{
public:
  static TypeId GetTypeId (void);
  IdealWifiManager ();
  virtual ~IdealWifiManager ();

  // Inherited from WifiRemoteStationManager
  virtual void SetupPhy (Ptr<WifiPhy> phy);

  typedef void (*RateChangeTracedCallback)(const uint64_t rate, const Mac48Address remoteAddress);

private:
  //overriden from base class
  virtual void DoInitialize (void);
  virtual WifiRemoteStation* DoCreateStation (void) const;
  virtual void DoReportRxOk (WifiRemoteStation *station,
                             double rxSnr, WifiMode txMode);
  virtual void DoReportRtsFailed (WifiRemoteStation *station);
  virtual void DoReportDataFailed (WifiRemoteStation *station);
  virtual void DoReportRtsOk (WifiRemoteStation *station,
                              double ctsSnr, WifiMode ctsMode, double rtsSnr);
  virtual void DoReportDataOk (WifiRemoteStation *station,
                               double ackSnr, WifiMode ackMode, double dataSnr);
  virtual void DoReportAmpduTxStatus (WifiRemoteStation *station, uint32_t nSuccessfulMpdus, uint32_t nFailedMpdus, double rxSnr, double dataSnr);
  virtual void DoReportFinalRtsFailed (WifiRemoteStation *station);
  virtual void DoReportFinalDataFailed (WifiRemoteStation *station);
  virtual WifiTxVector DoGetDataTxVector (WifiRemoteStation *station);
  virtual WifiTxVector DoGetRtsTxVector (WifiRemoteStation *station);
  virtual bool IsLowLatency (void) const;

  /**
   * Return the minimum SNR needed to successfully transmit
   * data with this WifiTxVector at the specified BER.
   *
   * \param txVector WifiTxVector (containing valid mode, width, and nss)
   *
   * \return the minimum SNR for the given WifiTxVector
   */
  double GetSnrThreshold (WifiTxVector txVector) const;
  /**
   * Adds a pair of WifiTxVector and the minimum SNR for that given vector
   * to the list.
   *
   * \param txVector the WifiTxVector storing mode, channel width, and nss
   * \param snr the minimum SNR for the given txVector
   */
  void AddSnrThreshold (WifiTxVector txVector, double snr);

  /**
   * Return true or false depending on whether the txVector is supported 
   * for transmitting.  The list of supported modes stored in the 
   * WifiPhy may be larger than the list of usable modes for sending data 
   * because the number of transmit antennas and receive antennas may 
   * be unequal.
   *
   * Note that WifiPhy has a similar IsValidTxVector() that may be used
   * to check whether the standard allows the combination of channel width,
   * NSS, and MCS.
   *
   * \param txVector the txVector to check
   * \return true if the station can transmit using this txVector
   */
  bool IsSupportedTxVector (const WifiTxVector txVector) const;
  /**
   * Convenience function for selecting a channel width for legacy mode
   * \param non-(V)HT WifiMode
   * \return the channel width (MHz) for the selected mode
   */
  uint32_t GetChannelWidthForMode (WifiMode mode) const;

  /**
   * \param st Station under consideration
   * \return true if cached values may be reused
   */
  bool UseCachedDataTxVector (WifiRemoteStation* st) const;
  /**
   * \param st Station under consideration
   * \mode WifiMode value to update the cached value to
   * \mode nss value to update the cached value to
   */
  void UpdateCachedDataTxVector (WifiRemoteStation* st, WifiMode mode, uint8_t nss);

  /**
   * \param st Station under consideration
   * \return true if suitable VHT MCS is found
   */
  bool DoGetDataTxVectorVht (WifiRemoteStation* st);

  /**
   * \param st Station under consideration
   * \return true if suitable HT MCS is found
   */
  bool DoGetDataTxVectorHt (WifiRemoteStation* st);

  /**
   * \param st Station under consideration
   * \return true if suitable legacy mode is found
   */
  bool DoGetDataTxVectorLegacy (WifiRemoteStation* st);

  /**
   * A vector of <snr, WifiTxVector> pair holding the minimum SNR for the 
   * WifiTxVector
   */
  typedef std::vector<std::pair<double, WifiTxVector> > Thresholds;

  double m_ber;             //!< The maximum Bit Error Rate acceptable at any transmission mode
  Thresholds m_thresholds;  //!< List of WifiTxVector and the minimum SNR pair

  /**
   * The trace source fired when the transmission rate change.
   */
  TracedCallback<uint64_t, Mac48Address> m_rateChange;
};

} //namespace ns3

#endif /* IDEAL_WIFI_MANAGER_H */
