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

#ifndef FRAME_SYNC_ERROR_RATE_LOOKUP_H
#define FRAME_SYNC_ERROR_RATE_LOOKUP_H

#include "wifi-phy.h"

namespace ns3
{

/**
 * \ingroup wifi
 *
 * \brief Provides lookup for SNR to frame synchronization error rate mapping.
 *
 * This class provides a lookup table to obtain the Wi-Fi frame synchronization
 * error rate (SER) for a given signal to noise ratio (SNR) (in dB) using the
 * specified channel model. Default channel models (from link simulation results)
 * are included as options, as well as an API for users to add their own.
 */
class FrameSyncErrorRateLookup : public Object
{
public:
  FrameSyncErrorRateLookup (); //!< Default constructor
  virtual ~FrameSyncErrorRateLookup (); //!< Destructor

  /**
   * Register this type.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /** 
   * Manually set the maximum number of decimal places needed to fully represent
   * all the SNR datapoint values in the lookup table. For example, 1 decimal
   * place is required to represent the set of SNRs {0, 0.5, 1, 1.5, and 2}.
   * ChannelModel attribute must be set to 'user' before calling this method.
   *
   * @param numDecimalPlaces the number of decimal places for SNR values
   */
  void SetNumSnrDatapointDecPlaces (double numDecimalPlaces);

  /** 
   * Manually set the spacing (in dB) between SNR datapoints in the lookup table.
   * For example, the SNR set {0, 2, 4, 6, 8,...} has an SNR spacing of 2.
   * ChannelModel attribute must be set to 'user' before calling this method.
   *
   * @param snrDatapointSpacing the spacing (in dB) between SNR datapoints
   */
  void SetSnrDatapointSpacing (double snrDatapointSpacing);

  /** 
   * Manually add an (SNR, SER) datapoint to the lookup table. SNR stands for
   * signal to noise ratio and SER stands for frame synchronization error rate.
   * Duplicate SNR values cannot be added. ChannelModel attribute must be set to
   * 'user' and SetNumSnrDatapointDecPlaces() must be called before calling this
   * method.
   *
   * @param snr the signal to noise ratio (in dB) of the datapoint
   * @param ser the frame synchronization error rate/probability of the datapoint
   */
  void AddSnrSerDatapoint (double snr, double ser);

  /** 
   * Returns the frame synchronization error rate (SER) for the given signal to
   * noise ratio (SNR) input from the lookup table. If the input SNR lie in
   * between SNR datapoints, linear interpolation is performed to calculate the
   * SER value to be returned. If the input SNR falls below/above the
   * minimum/maximum SNR of the datapoints, the SER corresponding to the
   * minimum/maximum SNR datapoint is returned. If ChannelModel attribute is set
   * to 'user', SetNumSnrDatapointDecPlaces() and SetSnrDatapointSpacing() must
   * be called before calling this method, and AddSnrSerDatapoint() must have
   * been called at least once.
   *
   * @param snr the signal to noise ratio (in dB) corresponding to the SER to
   * look up
   *
   * @return the retrieved frame sync error rate corresponding to the input SNR
   */
  double GetFrameSyncErrorRate (double snr);

private:
  /**
   * A struct containing the lower and upper SNR datapoint bounds used in linear
   * interpolation.
   */
  struct InterpolParams
  {
    double snrLoBound;
    double snrHiBound;
  };

  /** 
   * Rounds input double value to the nearest integer.
   *
   * @param val the double value to round
   *
   * @return the input value rounded to the nearest integer
   */
  int RoundDoubleToInt (double val);

  /** 
   * Converts double value to integer key used for lookup table indexing.
   *
   * @param val the double value to convert to integer key
   *
   * @return the integer key corresponding to the input double value
   */
  int DoubleToHashKeyInt (double val);

  /** 
   * Loads one of the default frame sync error rate models to the lookup table
   * based on ChannelModel attribute selected.
   */
  void LoadErrorRateDataToTable (void);

  /** 
   * Determines offset from zero of SNR datapoints in lookup table.
   */
  void DetermineSnrOffset (void);

  /** 
   * Rounds input value to the nearest data midpoint with given data spacing
   * and offset from zero. For example, an SNR dataset with SNR datapoints at
   * {-5, -2, 1, 4, 7, 10,...} has offset=1 and dataSpacing=3. The midpoints
   * between these numbers would be {-3.5, -0.5, 2.5, 5.5, 8.5,...}. So for
   * value=3.5 for this dataset, this method will return 2.5.
   *
   * @param value the input value to round
   * @param dataSpacing the spacing between datapoints
   * @param offset the offset from zero of the dataset
   *
   * @return the nearest data midpoint value
   */
  double RoundToNearestDataMidpoint (double value, double dataSpacing, double offset);

  /** 
   * Finds upper and lower SNR datapoint bounds around input SNR. If input SNR
   * is at upper or lower bound, both bounds are set equal to input SNR.
   *
   * @param snr the input SNR (in dB)
   * @param snrAtBound a flag set to true if input SNR is at upper or lower
   * datapoint bound
   *
   * @return the struct containing the upper and lower SNR datapoint bounds
   */
  InterpolParams FindDatapointBounds (double snr, bool snrAtBound);

  /** 
   * Retrieves SER value from lookup table based on input SNR and its SNR
   * datapoint bounds, performing linear interpolation if necessary.
   *
   * @param xd the SNR (in dB) corresponding to the SER to look up
   * @param x1d the lower SNR datapoint bound (in dB) of xd
   * @param x2d the upper SNR datapoint bound (in dB) of xd
   *
   * @return the retrieved SER
   */
  double InterpolateAndRetrieveData (double xd, double x1d, double x2d);

  enum WifiPhy::ChannelModel m_channelModel; //!< Channel model to use for frame sync error rate results
  bool m_errorRateTableIsLoaded; //!< Indicates if frames sync error rate table has been loaded (true) or not (false)
  uint8_t m_numSnrDecPlaces; //!< Number of decimal places in SNR datapoints
  bool m_numSnrDecPlacesIsSet; //!< Indicates if number of SNR datapoint decimal places has been set (true) or not (false)
  double m_snrSpacing; //!< Spacing (in dB) between SNR datapoints
  bool m_snrSpacingIsSet; //!< Indicates if SNR datapoint spacing has been set (true) or not (false)
  double m_snrMin; //!< Minimum (in dB) SNR datapoint value
  double m_snrMax; //!< Maximum (in dB) SNR datapoint value
  double m_snrOffset; //!< Offset from zero (in dB) of SNR datapoints
  bool m_snrOffsetIsSet; //!< Indicates if SNR offset from zero has been set (true) or not (false)
  std::map<int, double> m_frameSyncErrorRateTable; //!< Stores frame synchronization error rate values indexed by integer SNR key
};

} // namespace ns3

#endif /* FRAME_SYNC_ERROR_RATE_LOOKUP_H */
