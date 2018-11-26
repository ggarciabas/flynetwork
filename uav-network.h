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

#ifndef UAV_NETWORK
#define UAV_NETWORK

#include "server-application.h"
#include "uav-model.h"
#include "uav-mobility-model.h"
#include "uav-application.h"
#include "uav-application-container.h"
#include "uav-node-container.h"

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/olsr-module.h"

namespace ns3
{

/**
 * UavNetwork
 *
 */
class UavNetwork : public Object
{
public:
  static TypeId GetTypeId(void);
  UavNetwork();
  virtual ~UavNetwork();

  void Run();
  void ClientPosition (string name);

  void NewUav (int total, bool update);
  void RemoveUav(int id);

  /**
   * TracedCallback signature.
   */
  typedef void (*RemoveUavTrace) (int id);
  typedef void (*NewUavTrace)(int total, bool update);
  typedef void (*ClientPositionTrace)(string name);


  /**
   * TracedCallback signature.
   */
  typedef void (*PacketTraceServer)(std::string msg);
  typedef void (*PacketTraceUav)(std::string msg);
  typedef void (*PacketTraceClient)(std::string msg);

  void PacketUav(std::string);
  void PacketServer(std::string);
  void PacketClient(std::string);

private:
  void Configure();
  void ConfigureUav(int);
  void ConfigureCli();
  void ConfigurePalcos();
  void ConfigureServer();

private:

  void DoDispose();
  /*
    Input members
  */

  double m_simulationTime;
  double m_updateTimeCli;
  double m_updateTimeUav;
  uint16_t m_serverPort;
  uint16_t m_cliPort;
  uint32_t m_totalCli;
  uint32_t m_environment;
  uint32_t m_scenario;
  double m_zValue;
  double m_xmax, m_ymax, m_xmin, m_ymin, m_cx, m_cy; // m_c* posicao da central
  double m_speedUav;
  double m_txGain;
  double m_rxGain;
  double m_powerLevel;
  double m_frequency;
  std::string m_scenarioName;

  NodeContainer m_clientNode;
  vector<double> m_palcoPos; // posicao dos palcos para nao dar conflito no arquivo de conferencia
  UavNodeContainer m_uavNodeActive;
  UavNodeContainer m_uavNode;
  NodeContainer m_serverNode;
  YansWifiChannelHelper m_channelHelper;
  YansWifiChannelHelper m_channelHelperCli;
  YansWifiPhyHelper m_phyHelper;
  YansWifiPhyHelper m_phyHelperCli;
  WifiMacHelper m_macAdHocHelper;
  WifiMacHelper m_macWifiHelper;
  WifiMacHelper m_macWifiHelperCli;
  WifiHelper m_adhocHelper;
  WifiHelper m_wifiHelper;
  uint32_t m_protocol;
  uint32_t m_custo;
  std::string m_protocolName;
  Ipv4ListRoutingHelper m_list;
  Ipv4InterfaceContainer m_serverAddress;

  // AthstatsHelper m_athstats;

  std::string m_propagationLossCli;

  Ipv4AddressHelper m_addressHelperCli; // modificar estratégia de conexão do cliente para com os UAVs
  Ipv4AddressHelper m_addressHelper;

  Ptr<ServerApplication> m_serverApp;
  std::ostringstream m_ssgnuPalcos;
  // AnimationInterface*         m_animation;
  // Ptr<OutputStreamWrapper> m_routintable;
  // Ptr<OutputStreamWrapper> m_routintableUav;
  // Ptr<OutputStreamWrapper> m_routintableUser;
  Ipv4StaticRoutingHelper m_ipv4RoutingHelper;
  std::ofstream m_filePacketServer;
  std::ofstream m_filePacketUav;
  std::ofstream m_filePacketClient;
  InternetStackHelper m_stack;

  UavApplicationContainer   m_uavAppContainer;
  Ptr< PositionAllocator >    m_positionAlloc; // utilizado para distanciar os UAVs da central
};

} // namespace ns3

#endif /* UAV_NETWORK */
