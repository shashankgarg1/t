/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Biljana Bojovic <bbojovic@cttc.es> and Tom Henderson <tomh@tomh.org>
 */

#include <ns3/laa-wifi-coexistence-helper.h>
#include <ns3/log.h>

#include <ns3/lte-enb-net-device.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/lte-module.h>
#include <ns3/wifi-module.h>
#include <ns3/spectrum-module.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>
#include <ns3/propagation-module.h>
#include <ns3/config-store-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/lbt-access-manager.h>
#include <ns3/basic-lbt-access-manager.h>
#include <ns3/duty-cycle-access-manager.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LaaWifiCoexistenceHelper");

NS_OBJECT_ENSURE_REGISTERED (LaaWifiCoexistenceHelper);

LaaWifiCoexistenceHelper::LaaWifiCoexistenceHelper ()
{
  NS_LOG_FUNCTION (this);
}

LaaWifiCoexistenceHelper::~LaaWifiCoexistenceHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId 
LaaWifiCoexistenceHelper::GetTypeId (void)
{

  static TypeId tid = TypeId ("ns3::LaaWifiCoexistenceHelper")
    .SetParent<Object> ()
    .SetGroupName ("laa-wifi-coexistence")
    .AddAttribute ("ChannelAccessManagerType",
                   "The type of channel access manager to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::ChannelAccessManager. Default value"
                   "is ns3::ChannelAccessManager",
                   StringValue ("ns3::ChannelAccessManager"),
                   MakeStringAccessor (&LaaWifiCoexistenceHelper::SetChannelAccessManagerType,
                                       &LaaWifiCoexistenceHelper::GetChannelAccessManagerType),
                   MakeStringChecker ());
  return tid;

}

void
LaaWifiCoexistenceHelper::SetChannelAccessManagerType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_channelAccessManagerFactory = ObjectFactory ();
  m_channelAccessManagerFactory.SetTypeId (type);
}

std::string
LaaWifiCoexistenceHelper::GetChannelAccessManagerType () const
{
  return m_channelAccessManagerFactory.GetTypeId ().GetName ();
}

void 
LaaWifiCoexistenceHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void 
LaaWifiCoexistenceHelper::ConfigureEnbDevicesForLbt (NetDeviceContainer enbDevices, struct PhyParams phyParams)
{

  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (enbDevices.GetN () > 0, "empty enb device container");
  NetDeviceContainer monitorDevices;

  for (NetDeviceContainer::Iterator i = enbDevices.Begin (); i != enbDevices.End (); ++i)
    {
      Ptr<Node> node = (*i)->GetNode ();
      // we need a spectrum channel in order to install wifi device on the same instance of spectrum channel
      Ptr<LteEnbNetDevice> lteEnbNetDevice = (*i)->GetObject<LteEnbNetDevice> ();
      Ptr<SpectrumChannel> downlinkSpectrumChannel = lteEnbNetDevice->GetPhy ()->GetDownlinkSpectrumPhy ()->GetChannel ();
      SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
      spectrumPhy.SetChannel (downlinkSpectrumChannel);

      WifiHelper wifi;
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
      WifiMacHelper mac;
      spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (false));
      spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
      spectrumPhy.Set ("TxGain", DoubleValue (phyParams.m_ueTxGain));
      spectrumPhy.Set ("RxGain", DoubleValue (phyParams.m_ueRxGain));
      spectrumPhy.Set ("TxPowerStart", DoubleValue (phyParams.m_ueTxPower));
      spectrumPhy.Set ("TxPowerEnd", DoubleValue (phyParams.m_ueTxPower));
      spectrumPhy.Set ("RxNoiseFigure", DoubleValue (phyParams.m_ueNoiseFigure));
      spectrumPhy.Set ("Receivers", UintegerValue (2));
      spectrumPhy.Set ("Transmitters", UintegerValue (2));

      wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
      //which implements a Wi-Fi MAC that does not perform any kind of beacon generation, probing, or association
      mac.SetType ("ns3::AdhocWifiMac");

      //uint32_t channelNumber = 36 + 4 * (i%4);
      uint32_t channelNumber = 36;
      spectrumPhy.SetChannelNumber (channelNumber);

      // wifi device that is doing monitoring
      Ptr<NetDevice> monitor = (wifi.Install (spectrumPhy, mac, node)).Get (0);
      Ptr<WifiPhy> wifiPhy = monitor->GetObject<WifiNetDevice> ()->GetPhy ();
      Ptr<SpectrumWifiPhy> spectrumWifiPhy = DynamicCast<SpectrumWifiPhy> (wifiPhy);
      //Ptr<MacLow> macLow = monitor->GetObject<WifiNetDevice>()->GetMac();

      Ptr<LteEnbPhy> ltePhy = (*i)->GetObject<LteEnbNetDevice> ()->GetPhy ();
      Ptr<LteEnbMac> lteMac = (*i)->GetObject<LteEnbNetDevice> ()->GetMac ();

      if (m_channelAccessManagerFactory.GetTypeId ().GetName () == "ns3::BasicLbtAccessManager")
        {
          Ptr<BasicLbtAccessManager> basicLbtAccessManager = m_channelAccessManagerFactory.Create<BasicLbtAccessManager> ();
          basicLbtAccessManager->SetWifiPhy (spectrumWifiPhy);
          // set channel access manager to lteEnbPhy
          ltePhy->SetChannelAccessManager (basicLbtAccessManager);
        }
      else if (m_channelAccessManagerFactory.GetTypeId ().GetName () == "ns3::LbtAccessManager")
        {
          Ptr<LbtAccessManager> lbtAccessManager = m_channelAccessManagerFactory.Create<LbtAccessManager> ();
          lbtAccessManager->SetWifiPhy (spectrumWifiPhy);
          lbtAccessManager->SetLteEnbMac(lteMac);
          lbtAccessManager->SetLteEnbPhy(ltePhy);
          //lbtAccessManager->SetupLowListener(macLow);
          ltePhy->SetChannelAccessManager (lbtAccessManager);
        }
      else if (m_channelAccessManagerFactory.GetTypeId ().GetName () == "ns3::DutyCycleAccessManager")
      {
         Ptr<DutyCycleAccessManager> dutyCycleAccessManager = m_channelAccessManagerFactory.Create<DutyCycleAccessManager> ();
         ltePhy->SetChannelAccessManager (dutyCycleAccessManager);
      }
      // set callbacks
      //wifiPhy->TraceConnectWithoutContext ("PhyRxBegin", MakeCallback (&ns3::LaaWifiCoexistenceHelper::WifiRxBegin, this));
    }
}

void LaaWifiCoexistenceHelper::WifiRxBegin (Ptr< const Packet > packet)
{
  NS_LOG_DEBUG ("Packet:" << packet->GetUid ());
}

} // namespace ns3
