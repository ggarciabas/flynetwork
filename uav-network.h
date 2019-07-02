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

#include <limits>

namespace ns3
{

#define m_sup(v,v_sup) ((v+50.0>v_sup)?v_sup:v+50.0)
#define m_low(v,v_low) ((v-50.0<v_low)?v_low:v-50.0)

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

  void NewUav (int total, int update);
  void RemoveUav(int id, int step);

  /**
   * TracedCallback signature.
   */
  typedef void (*RemoveUavTrace) (int id, int step);
  typedef void (*NewUavTrace)(int total, int update);
  typedef void (*ClientPositionTrace)(string name);

  void PrintUavEnergy (int i);

  void PrintFinalUavEnergy();
private:
  void Configure();
  void ConfigureUav(int);
  void ConfigureCli();
  void ConfigurePalcos();
  void ConfigureServer();

  void ClientBehaviour (int posCli);
  void ClientConsumption (int posCli);

private:

  void DoDispose();
  /*
    Input members
  */

  double m_simulationTime;
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
  std::string m_PathData;
  std::string m_pathData;
  std::string m_scenarioName;
  double m_uavTimingNext;

  NodeContainer m_clientNode;
  vector<EventId> m_cliEvent; // eventos programados para os clientes
  Ptr<UniformRandomVariable> m_randApp;
  double m_clientUpdateCons;

  vector<double> m_palcoPos; // posicao dos palcos para nao dar conflito no arquivo de conferencia
  UavNodeContainer m_uavNodeActive;
  UavNodeContainer m_uavNode;
  NodeContainer m_serverNode;
  YansWifiChannelHelper m_channelHelper;
  YansWifiPhyHelper m_phyHelper;
  WifiMacHelper m_macAdHocHelper;
  WifiHelper m_adhocHelper;
  uint32_t m_protocol;
  uint32_t m_custo;
  uint32_t m_seed;
  std::string m_protocolName;
  Ipv4ListRoutingHelper m_list;
  Ipv4InterfaceContainer m_serverAddress;
  double m_iniX, m_iniY; // posicao inicial do UAV para nao interferir nos resultados!
  double m_scheduleServer;
  map<int, int> m_descTime;

  EventId m_newApp; // controla as aplicacoes dos usuarios

  // AthstatsHelper m_athstats;

  Ipv4AddressHelper m_addressHelper;

  Ptr<ServerApplication> m_serverApp;
  Ipv4StaticRoutingHelper m_ipv4RoutingHelper;
  InternetStackHelper m_stack;

  UavApplicationContainer   m_uavAppContainer;
  map<uint32_t, uint32_t>   m_nodeUavApp; // id, m_uavAppContainer pos
  Ptr< PositionAllocator >  m_positionAlloc; // utilizado para distanciar os UAVs da central

  std::ofstream m_file;
};

} // namespace ns3

#endif /* UAV_NETWORK */
