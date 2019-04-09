/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 * Authors: Giovanna Garcia <ggarciabas@gmail.com>
 */

#ifndef UAV_APPLICATION
#define UAV_APPLICATION

#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/log.h"
#include "ns3/packet-sink.h"
#include "ns3/simulator.h"
#include "ns3/internet-module.h"
#include "ns3/energy-module.h"
#include "uav-energy-source.h"
#include "uav-device-energy-model.h"
#include "client-device-energy-model.h"

#include "client-model-container.h"
#include "client-model.h"

using namespace std;

namespace ns3
{

/**
 * UavApplication
 *	Application used to send notifications to the
 * server.
 * 		Information about the actual location and
 * messages of control.
 *		Update location based on server notifications.
 */
class UavApplication : public Application
{
public:
  // typedef Callback<void> OffWifiPhyCallback;

  static TypeId GetTypeId(void);
  UavApplication();
  virtual ~UavApplication();

  void SetId(uint32_t);
  uint32_t GetId();

  void EnergyDepletionCallback();
  void EnergyRechargedCallback();

  void Reset();

  void CourseChange (Ptr<const MobilityModel>);

  void TracedCallbackRxApp (Ptr<const Packet> packet, const Address & address);
  void TracedCallbackRxAppInfra (Ptr<const Packet> packet, const Address & address);
  // void TurnOffWifiPhy ();
  // void SetTurnOffWifiPhyCallback(OffWifiPhyCallback adhoc, OffWifiPhyCallback infra);

  void TotalEnergyConsumptionTrace (double oldV, double newV);
  void TracedCallbackNewLease (const Ipv4Address& ip);
  void TracedCallbackExpiryLease (const Ipv4Address& ip);

  void Start(double);
  void Stop();

  void SetWifiDevice (Ptr<WifiRadioEnergyModel> dev);
  Ptr<WifiRadioEnergyModel> GetWifiDevice() {
    return m_wifiDevice;
  }
  void SetUavDevice (Ptr<UavDeviceEnergyModel> dev);
  Ptr<UavDeviceEnergyModel> GetUavDevice () {
    return m_uavDevice;
  }
  void SetCliDevice (Ptr<ClientDeviceEnergyModel> dev);
  Ptr<ClientDeviceEnergyModel> GetCliDevice () {
    return m_cliDevice;
  }

private:
  void DoDispose();
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  double CalculateDistance(const std::vector<double> pos1, const std::vector<double> pos2);

  void ReplyServer ();

  // void ScheduleTx(void);
  void SendPacket(void);
  void SendCliData ();
  void SendPacketDepletion(void);
  void AskCliPosition();

  uint32_t m_id;
  uint16_t m_serverPort;
  uint16_t m_cliPort;
  std::vector<double> m_goto;
  std::vector<double> m_central;
  double m_meanConsumption; // consumo medio do UAV
  Ipv4Address m_addressAdhoc;
  double m_updateTime;
  double m_cliUpdateTime;
  Ipv4Address m_peer;
  DataRate m_dataRate;
  Ptr<Socket> m_sendSck; // sending socket
  Ptr<Socket> m_socketClient;
  EventId m_sendEvent; 
  EventId m_sendCliDataEvent;
  EventId m_packetDepletion;
  EventId m_askCliPos;
  bool m_running;
  TracedCallback<std::string> m_packetTrace;
  Callback<void> m_setOffWifiPhyInfra; // turn off wifiphy
  Callback<void> m_setOffWifiPhyAdhoc; // turn off wifiphy
  Ptr<WifiRadioEnergyModel> m_wifiDevice;
  Ptr<UavDeviceEnergyModel> m_uavDevice;
  Ptr<ClientDeviceEnergyModel> m_cliDevice;

  bool m_depletion;// para identificar estado de emergencia

  std::map<Ipv4Address, Ptr<ClientModel> > m_mapClient;

  Ptr<WifiRadioEnergyModel> m_wifiRadioEnergyModel;
  Ptr<UavEnergySource> m_uavEnergySource;

  std::string m_pathData;

  int m_totalLeased;

};

} // namespace ns3

#endif /* UAV_APPLICATION */
