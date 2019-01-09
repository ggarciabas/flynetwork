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

#ifndef SMARTPHONE_APPLICATION
#define SMARTPHONE_APPLICATION

#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "uav-model-container.h"
#include "server-application.h"
#include "ns3/wifi-module.h"
#include "uav-model.h"
#include <string>

using namespace std;
using namespace ns3;

namespace ns3
{

class UavModel;

/**
 * SmartphoneApplication
 *	Application used to send information about the actual location.
 */
class SmartphoneApplication : public Application
{
public:
  static TypeId GetTypeId(void);
  SmartphoneApplication();
  virtual ~SmartphoneApplication();

  void SetLogin(std::string);
  std::string GetLogin();

  void CourseChange(Ptr<const MobilityModel> mobility);

  void TracedCallbackTxApp (Ptr<const Packet> packet, const Address &source, const Address &dest);

  void TracedCallbackAssocLogger (Mac48Address mac);
  void TracedCallbackDeAssocLogger (Mac48Address mac);

  void PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower);
  void PhyRxErrorTrace (std::string context, Ptr<const Packet> packet, double snr);
  void PhyRxOkTrace (std::string context, Ptr<const Packet> packet, double snr, WifiMode mode, enum WifiPreamble preamble);

  void TracedCallbackNewLease (const Ipv4Address& ip);
  void TracedCallbackExpiryLease (const Ipv4Address& ip);

  void SetApp (std::string a);
  std::string GetApp ();
private:
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  void SendPacketUav(void);

  void DoDispose();

  uint32_t m_id;
  uint16_t m_port;
  uint32_t m_idDHCP;
  bool m_connected;//
  double m_start;
  Ipv4Address m_appPeer; // endereco do servidor de aplicacao
  Ipv4Address m_uavPeer;
  DataRate m_dataRate;
  Ptr<Socket> m_socketUav;
  EventId m_sendEventUav;
  bool m_running;
  std::string m_login;
  std::string m_app;
  double m_changePosition; // máximo de movimentação para notificar o servidor
  Vector m_lastPosition;   // posicao inicial para notificacao ao servidor
  TracedCallback<std::string> m_packetTrace;
  Ipv4StaticRoutingHelper m_routingHelper;
  std::string m_pathData;

  Ptr<PacketSink> m_sink;
  Ptr<OnOffApplication> m_serverApp; // application sent to server
};

} // namespace ns3

#endif /* SMARTPHONE_APPLICATION */
