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
  void EnergyAskUavCallback();

  void CourseChange (Ptr<const MobilityModel>);

  void TracedCallbackRxApp (Ptr<const Packet> packet, const Address & address);

  void Start(double);
  void Stop();

  void SetUavDevice (Ptr<UavDeviceEnergyModel> dev);
  Ptr<UavDeviceEnergyModel> GetUavDevice () {
    return m_uavDevice;
  }

  void ClientConsumption (double time, double px, double py, uint32_t id);

private:
  void DoDispose();
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  double CalculateDistance(const std::vector<double> pos1, const std::vector<double> pos2);

  void ReplyServer ();

  // void ScheduleTx(void);
  void SendPacket(void);
  void SendCliData ();
  void SendPacketNewUav(void);

  uint32_t m_id;
  uint16_t m_serverPort;
  std::vector<double> m_goto;
  std::vector<double> m_central;
  Ipv4Address m_addressAdhoc;
  double m_updateTime;
  Ipv4Address m_peer;
  DataRate m_dataRate;
  Ptr<Socket> m_sendSck; // sending socket
  EventId m_sendEvent; 
  EventId m_sendCliDataEvent;
  EventId m_packetAskUav;
  bool m_running;
  TracedCallback<std::string> m_packetTrace;
  Ptr<UavDeviceEnergyModel> m_uavDevice;

  ClientModelContainer m_clientContainer;  

  EventId m_programDepletion; // utilizado para programar o depletion de acordo com o tempo para o UAV novo chegar até a posicao onde o UAV que solicitou está.

  bool m_depletion;// para identificar estado de emergencia

  // std::map<Ipv4Address, Ptr<ClientModel> > m_mapClient;

  std::string m_pathData;

};

} // namespace ns3

#endif /* UAV_APPLICATION */
