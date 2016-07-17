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

#include <ns3/test.h>
#include <ns3/log.h>
#include <ns3/string.h>
#include <ns3/frame-sync-error-rate-lookup.h>

/**
 * This test verifies the successful operation of the FrameSyncErrorRateLookup
 * class by loading in known mapped signal to noise ratio (SNR) to frame
 * synchronization error rate (SER) datapoints and verifying correct lookup
 * values for exact SNR datapoints, above/below SNR datapoint bounds, and
 * interpolated values (lookup between SNR datapoints).
 */
NS_LOG_COMPONENT_DEFINE ("FrameSyncErrorRateLookupTest");

using namespace ns3;

const double TOLERANCE = 1e-15;
// Adjust floating point comparison epsilon based on inputs.
// Follows http://realtimecollisiondetection.net/blog/?p=89
static double epsilon = 0;


// Data for signal to noise ratio (SNR) (in dB) to delayed autocorrelation frame
// synchronization error rate (SER) mapping for Additive White Gaussian
// Noise (AWGN) only channel and IEEE TGn Channel Model D (with AWGN).

// AWGN channel
const unsigned int g_channelAwgnNumSnrSerDatapoints = 28;
const double g_channelAwgnSnrs[28] = {-10, -9, -8, -7, -6, -5, -4, -3,
                                             -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
                                             9, 10, 11, 12, 13, 14, 15, 16, 17};
const double g_channelAwgnSers[28] = {1.00, 0.99, 0.99, 0.99, 0.98, 0.96,
                                             0.95, 0.90, 0.81, 0.65, 0.47, 0.29,
                                             0.13, 0.04, 0.01, 0.00, 0.00, 0.00,
                                             0.00, 0.00, 0.00, 0.00, 0.00, 0.00,
                                             0.00, 0.00, 0.00, 0.00};
uint8_t g_channelAwgnNumSnrDecPlaces = 0;
const double g_channelAwgnSnrSpacing = 1;
const double g_channelAwgnSnrMin = -10;
const double g_channelAwgnSnrMax = 17;

// IEEE TGn Channel Model D
const unsigned int g_channelDNumSnrSerDatapoints = 28;
const double g_channelDSnrs[28] = {-10, -9, -8, -7, -6, -5, -4, -3, -2,
                                          -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                          11, 12, 13, 14, 15, 16, 17};
const double g_channelDSers[28] = {1.00, 0.99, 0.99, 0.98, 0.98, 0.96,
                                          0.92, 0.87, 0.78, 0.67, 0.54, 0.39,
                                          0.27, 0.19, 0.10, 0.06, 0.03, 0.01,
                                          0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
                                          0.01, 0.00, 0.00, 0.00};
uint8_t g_channelDNumSnrDecPlaces = 0;
const double g_channelDSnrSpacing = 1;
const double g_channelDSnrMin = -10;
const double g_channelDSnrMax = 17;


class FrameSyncErrorRateLookupTestCase : public TestCase
{
public:
  FrameSyncErrorRateLookupTestCase (std::string channelModel);
  virtual ~FrameSyncErrorRateLookupTestCase ();

private:
  virtual void DoRun (void);
  static std::string Name (std::string channelModel);

  std::string m_channelModel;
};


std::string 
FrameSyncErrorRateLookupTestCase::Name (std::string channelModel)
{
  std::ostringstream oss;
  oss << "channel model: " << channelModel;
  return oss.str();
}

FrameSyncErrorRateLookupTestCase::FrameSyncErrorRateLookupTestCase (std::string channelModel)
  : TestCase (Name (channelModel)),
    m_channelModel (channelModel)
{
}

FrameSyncErrorRateLookupTestCase::~FrameSyncErrorRateLookupTestCase ()
{
}

void
FrameSyncErrorRateLookupTestCase::DoRun (void)
{
  NS_LOG_FUNCTION (m_channelModel);

  double expectedSer;
  double retrievedSer;

  Ptr<FrameSyncErrorRateLookup> syncErrorLookup = CreateObject<FrameSyncErrorRateLookup> ();
  syncErrorLookup->SetAttribute ("ChannelModel", StringValue ("user"));

  if (m_channelModel.compare ("awgn") == 0)
    {
      syncErrorLookup->SetNumSnrDatapointDecPlaces (g_channelAwgnNumSnrDecPlaces);
      syncErrorLookup->SetSnrDatapointSpacing (g_channelAwgnSnrSpacing);
      // load lookup table
      for (unsigned int i = 0; i < g_channelAwgnNumSnrSerDatapoints; i++)
        {
          syncErrorLookup->AddSnrSerDatapoint (g_channelAwgnSnrs[i], g_channelAwgnSers[i]);
        }
      // test exact datapoint lookup
      for (unsigned int i = 0; i < g_channelAwgnNumSnrSerDatapoints; i++)
        {
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelAwgnSnrs[i]);
          expectedSer = g_channelAwgnSers[i];
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
        }
      // test above upper bound lookup
      retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelAwgnSnrMax + 1);
      expectedSer = g_channelAwgnSers[g_channelAwgnNumSnrSerDatapoints - 1];
      epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
      NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                 expectedSer,
                                 epsilon,
                                 "retrieved SER value (" << retrievedSer << ") is incorrect");
      // test below lower bound lookup
      retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelAwgnSnrMin - 1);
      expectedSer = g_channelAwgnSers[0];
      epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
      NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                 expectedSer,
                                 epsilon,
                                 "retrieved SER value (" << retrievedSer << ") is incorrect");
      // test interpolation lookup
      for (unsigned int i = 0; i < g_channelAwgnNumSnrSerDatapoints - 1; i++)
        {
          // test halfway between datapoints
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate ((g_channelAwgnSnrs[i] + g_channelAwgnSnrs[i + 1]) / 2);
          expectedSer = (g_channelAwgnSers[i] + g_channelAwgnSers[i + 1]) / 2;
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
          // test 1/4 between datapoints
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelAwgnSnrs[i] * 0.75 + g_channelAwgnSnrs[i + 1] * 0.25);
          expectedSer = g_channelAwgnSers[i] * 0.75 + g_channelAwgnSers[i + 1] * 0.25;
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
          // test 9/10 between datapoints
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelAwgnSnrs[i] * 0.1 + g_channelAwgnSnrs[i + 1] * 0.9);
          expectedSer = g_channelAwgnSers[i] * 0.1 + g_channelAwgnSers[i + 1] * 0.9;
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
        }
    }
  else if (m_channelModel.compare ("d") == 0)
    {
      syncErrorLookup->SetNumSnrDatapointDecPlaces (g_channelDNumSnrDecPlaces);
      syncErrorLookup->SetSnrDatapointSpacing (g_channelDSnrSpacing);
      for (unsigned int i = 0; i < g_channelDNumSnrSerDatapoints; i++)
        {
          syncErrorLookup->AddSnrSerDatapoint (g_channelDSnrs[i], g_channelDSers[i]);
        }
      // test exact datapoint lookup
      for (unsigned int i = 0; i < g_channelDNumSnrSerDatapoints; i++)
        {
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelDSnrs[i]);
          expectedSer = g_channelDSers[i];
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
        }
      // test above upper bound lookup
      retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelDSnrMax + 1);
      expectedSer = g_channelDSers[g_channelDNumSnrSerDatapoints - 1];
      epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
      NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                 expectedSer,
                                 epsilon,
                                 "retrieved SER value (" << retrievedSer << ") is incorrect");
      // test below lower bound lookup
      retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelDSnrMin - 1);
      expectedSer = g_channelDSers[0];
      epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
      NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                 expectedSer,
                                 epsilon,
                                 "retrieved SER value (" << retrievedSer << ") is incorrect");
      // test interpolation lookup
      for (unsigned int i = 0; i < g_channelDNumSnrSerDatapoints - 1; i++)
        {
          // test halfway between datapoints
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate ((g_channelDSnrs[i] + g_channelDSnrs[i + 1]) / 2);
          expectedSer = (g_channelDSers[i] + g_channelDSers[i + 1]) / 2;
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
          // test 1/4 between datapoints
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelDSnrs[i] * 0.75 + g_channelDSnrs[i + 1] * 0.25);
          expectedSer = g_channelDSers[i] * 0.75 + g_channelDSers[i + 1] * 0.25;
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
          // test 9/10 between datapoints
          retrievedSer = syncErrorLookup->GetFrameSyncErrorRate (g_channelDSnrs[i] * 0.1 + g_channelDSnrs[i + 1] * 0.9);
          expectedSer = g_channelDSers[i] * 0.1 + g_channelDSers[i + 1] * 0.9;
          epsilon = TOLERANCE * std::max (1.0, std::max (retrievedSer, expectedSer));
          NS_TEST_ASSERT_MSG_EQ_TOL (retrievedSer,
                                     expectedSer,
                                     epsilon,
                                     "retrieved SER value (" << retrievedSer << ") is incorrect");
        }
    }
  else // force test failure because of unsupported channel model for this test
    {
      NS_TEST_ASSERT_MSG_EQ (0, 1, "channel model " << m_channelModel << " is unsupported in this test suite");
    }
}


class FrameSyncErrorRateLookupTestSuite : public TestSuite
{
public:
  FrameSyncErrorRateLookupTestSuite ();
};

FrameSyncErrorRateLookupTestSuite::FrameSyncErrorRateLookupTestSuite ()
  : TestSuite ("frame-sync-error-rate-lookup", UNIT)
{
  NS_LOG_INFO ("creating FrameSyncErrorRateLookupTestSuite");
  AddTestCase (new FrameSyncErrorRateLookupTestCase ("awgn"), TestCase::QUICK);
  AddTestCase (new FrameSyncErrorRateLookupTestCase ("d"), TestCase::QUICK);
}

static FrameSyncErrorRateLookupTestSuite g_frameSyncErrorRateLookupTestSuite;
