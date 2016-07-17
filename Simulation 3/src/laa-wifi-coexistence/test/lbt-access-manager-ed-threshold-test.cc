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
 */

#include "ns3/test.h"
#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/log.h"
#include "ns3/packet-burst.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/wifi-spectrum-helper.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/interference-helper.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac-trailer.h"
#include "ns3/wifi-phy-tag.h"
#include "ns3/wifi-phy-standard.h"
#include "ns3/wifi-spectrum-signal-parameters.h"
#include "ns3/lbt-access-manager.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LbtAccessManagerEdThresholdTest");

static const uint16_t CHANNEL_NUMBER = 36;
static const uint32_t FREQUENCY = 5000; // MHz

class LbtAccessManagerEdThresholdTest : public TestCase
{
public:
  LbtAccessManagerEdThresholdTest ();
  LbtAccessManagerEdThresholdTest (std::string name);
  virtual ~LbtAccessManagerEdThresholdTest ();
protected:
  virtual void DoSetup (void);
  Ptr<SpectrumWifiPhy> m_phy;
  Ptr<LbtAccessManager> m_lbt;
  Ptr<SpectrumSignalParameters> MakeSignal (double txPowerWatts);
  void SendSignal (double txPowerWatts);
  void SpectrumWifiPhyReceiver (bool rxSucceeded);
private:
  virtual void DoRun (void);
  void ReceiveAccessGranted (Time duration);
  void RequestAccess (void);
  void SetEdThreshold (double edThreshold);
  void CheckState (LbtAccessManager::LbtState state);
};

LbtAccessManagerEdThresholdTest::LbtAccessManagerEdThresholdTest ()
  : TestCase ("LbtAccessManager testing of ED threshold")
{
}

LbtAccessManagerEdThresholdTest::LbtAccessManagerEdThresholdTest (std::string name)
  : TestCase (name)
{
}

// Make a Wi-Fi signal to inject directly to the StartRx() method
Ptr<SpectrumSignalParameters> 
LbtAccessManagerEdThresholdTest::MakeSignal (double txPowerWatts)
{
  WifiPreamble preamble;
  preamble = WIFI_PREAMBLE_LONG;
  WifiMode mode = WifiPhy::GetHtMcs0 ();
  WifiTxVector txVector = WifiTxVector (mode, 0, 0, false, 1, 0, 20000000, false, false);
  enum mpduType mpdutype = NORMAL_MPDU;

  Ptr<Packet> pkt = Create<Packet> (1000);
  WifiMacHeader hdr;
  WifiMacTrailer trailer;

  hdr.SetType (WIFI_MAC_QOSDATA);
  hdr.SetQosTid (0);
  uint32_t size = pkt->GetSize () + hdr.GetSize () + trailer.GetSerializedSize ();
  Time txDuration = m_phy->CalculateTxDuration (size, txVector, preamble, m_phy->GetFrequency(), mpdutype, 0);
  hdr.SetDuration (txDuration);

  pkt->AddHeader (hdr);
  pkt->AddTrailer (trailer);
  WifiPhyTag tag (txVector, preamble, mpdutype);
  pkt->AddPacketTag (tag);
  Ptr<SpectrumValue> txPowerSpectrum = WifiSpectrumHelper::CreateTxPowerSpectralDensity (txPowerWatts, txVector.GetMode (), CHANNEL_NUMBER);
  Ptr<PacketBurst> pb = Create<PacketBurst> ();
  pb->AddPacket (pkt);
  Ptr<WifiSpectrumSignalParameters> txParams = Create<WifiSpectrumSignalParameters> ();
  txParams->psd = txPowerSpectrum;
  txParams->txPhy = 0;
  txParams->duration = txDuration;
  txParams->packetBurst = pb;
  return txParams;
}

// Make a Wi-Fi signal to inject directly to the StartRx() method
void
LbtAccessManagerEdThresholdTest::SendSignal (double txPowerWatts)
{
  m_phy->StartRx (MakeSignal (txPowerWatts));
}

void
LbtAccessManagerEdThresholdTest::SpectrumWifiPhyReceiver (bool rxSucceeded)
{
  NS_FATAL_ERROR ("Should be unreachable; Wi-Fi rx reception disabled");
}

void
LbtAccessManagerEdThresholdTest::SetEdThreshold (double edThreshold)
{
  bool ok = m_lbt->SetAttributeFailSafe ("EnergyDetectionThreshold", DoubleValue (edThreshold));
  NS_TEST_ASSERT_MSG_EQ (ok, true, "Could not set attribute");
  // change the underlying SpectrumWifiPhy attribute via the LbtAccessManager
  m_lbt->SetWifiPhy (m_phy);  
}

LbtAccessManagerEdThresholdTest::~LbtAccessManagerEdThresholdTest ()
{
}

void
LbtAccessManagerEdThresholdTest::RequestAccess ()
{
  m_lbt->RequestAccess ();
}

void
LbtAccessManagerEdThresholdTest::ReceiveAccessGranted (Time duration)
{
  NS_FATAL_ERROR ("Should be unreachable; LTE not yet part of this test");
}

// Create necessary objects, and inject signals.  Test that the expected
// number of packet receptions occur.
void
LbtAccessManagerEdThresholdTest::DoSetup (void)
{
  m_phy = CreateObject<SpectrumWifiPhy> ();
  m_phy->ConfigureStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  Ptr<ErrorRateModel> error = CreateObject<NistErrorRateModel> ();
  m_phy->SetErrorRateModel (error);
  m_phy->SetChannelNumber (CHANNEL_NUMBER);
  m_phy->SetChannelModel (WifiPhy::AWGN);
  m_phy->SetFrequency (FREQUENCY);
  m_phy->SetNoisePowerSpectralDensity (WifiSpectrumHelper::CreateNoisePowerSpectralDensity (0, CHANNEL_NUMBER));
  m_phy->SetPacketReceivedCallback (MakeCallback (&LbtAccessManagerEdThresholdTest::SpectrumWifiPhyReceiver, this));
  m_phy->SetAttribute ("DisableWifiReception", BooleanValue (true));
  m_phy->SetAttribute ("RxGain", DoubleValue (0.0));

  m_lbt = CreateObject<LbtAccessManager> ();
  m_lbt->SetWifiPhy (m_phy);
  m_lbt->SetAccessGrantedCallback (MakeCallback (&LbtAccessManagerEdThresholdTest::ReceiveAccessGranted, this));
}

void
LbtAccessManagerEdThresholdTest::CheckState (LbtAccessManager::LbtState state)
{
  NS_TEST_ASSERT_MSG_EQ (state, m_lbt->GetLbtState (), "Failed at time " << Simulator::Now());
}

void
LbtAccessManagerEdThresholdTest::DoRun (void)
{
  double txPowerWatts = 1e-9;  // -60 dBm
  // Signal duration is 1292 microseconds
  // Check that state starts in IDLE 
  Simulator::Schedule (Seconds (0.5), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Send a packet above threshold and see if it causes Lbt state to go BUSY
  // just for the duration of the packet, then return to IDLE
  m_phy->SetAttribute ("CcaMode1Threshold", DoubleValue (-62.0));
  Simulator::Schedule (Seconds (1), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (1001291), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  Simulator::Schedule (MicroSeconds (1001293), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Move below ED threshold of -62 dBm.  LbtAccessManager shouldn't hear it
  txPowerWatts = 5e-10;  // -63 dBm
  Simulator::Schedule (Seconds (2), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (2001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Hit almost exactly the ED threshold with one packet
  txPowerWatts = 6.32e-10;  // -62 dBm
  Simulator::Schedule (Seconds (3), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (3001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  // Double check state at the same time
  Simulator::Schedule (MicroSeconds (3001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);

  // Hit exactly the ED threshold with two packets sent 1 us apart
  txPowerWatts = 5e-10;  // -63 dBm
  Simulator::Schedule (MicroSeconds (4000000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (4000001), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (4001000), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);

  // Two packets sent 700 us apart.  Before 700 us into the first packet, the
  // state should be IDLE.  After second packet arrives, the state should be
  // busy for 596 us more, then should return to IDLE.
  Simulator::Schedule (MicroSeconds (5000000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (5000700), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  // Check around the boundaries
  Simulator::Schedule (MicroSeconds (5000699), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);
  Simulator::Schedule (MicroSeconds (5000701), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  Simulator::Schedule (MicroSeconds (5001291), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  Simulator::Schedule (MicroSeconds (5001293), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  // Change the LBT ED threshold attribute to -72 dBm.  
  Simulator::Schedule (MicroSeconds (6000000), &LbtAccessManagerEdThresholdTest::SetEdThreshold, this, -72.0);
  // Check that signal above -72 dBm but below -62 dBm triggers a BUSY
  txPowerWatts = 1.58e-10;  // -68 dBm
  Simulator::Schedule (MicroSeconds (6001000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (6001050), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::BUSY);
  txPowerWatts = 5e-11;  // -73 dBm
  Simulator::Schedule (MicroSeconds (7001000), &LbtAccessManagerEdThresholdTest::SendSignal, this, txPowerWatts); 
  Simulator::Schedule (MicroSeconds (7001050), &LbtAccessManagerEdThresholdTest::CheckState, this, LbtAccessManager::IDLE);

  Simulator::Stop (Seconds (10));
  Simulator::Run ();
  Simulator::Destroy ();
}

class TestPhyListener : public ns3::WifiPhyListener
{
public:
  /**
   * Create a test PhyListener
   *
   */
  TestPhyListener (void) :
    m_notifyRxStart (0),
    m_notifyRxEndOk (0),
    m_notifyRxEndError (0),
    m_notifyMaybeCcaBusyStart (0)
  {
  }
  virtual ~TestPhyListener ()
  {
  }
  virtual void NotifyRxStart (Time duration)
  {
    ++m_notifyRxStart;
  }
  virtual void NotifyRxEndOk (void)
  {
    ++m_notifyRxEndOk;
  }
  virtual void NotifyRxEndError (void)
  {
    ++m_notifyRxEndError;
  }
  virtual void NotifyTxStart (Time duration, double txPowerDbm)
  {
  }
  virtual void NotifyMaybeCcaBusyStart (Time duration)
  {
    ++m_notifyMaybeCcaBusyStart;
  }
  virtual void NotifySwitchingStart (Time duration)
  {
  }
  virtual void NotifySleep (void)
  {
  }
  virtual void NotifyWakeup (void)
  {
  }
  uint32_t m_notifyRxStart;
  uint32_t m_notifyRxEndOk;
  uint32_t m_notifyRxEndError;
  uint32_t m_notifyMaybeCcaBusyStart;
private:
};

class LbtAccessManagerEdThresholdTestSuite : public TestSuite
{
public:
  LbtAccessManagerEdThresholdTestSuite ();
};

LbtAccessManagerEdThresholdTestSuite::LbtAccessManagerEdThresholdTestSuite ()
  : TestSuite ("lbt-access-manager-ed-threshold", UNIT)
{
  AddTestCase (new LbtAccessManagerEdThresholdTest, TestCase::QUICK);
}

static LbtAccessManagerEdThresholdTestSuite lbtAccessManagerTestSuite;

