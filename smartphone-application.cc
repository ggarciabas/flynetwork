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

#include "smartphone-application.h"

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include <cmath>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SmartphoneApplication");

NS_OBJECT_ENSURE_REGISTERED(SmartphoneApplication);

TypeId
SmartphoneApplication::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::SmartphoneApplication")
                          .SetParent<Application>()
                          .SetGroupName("Flynetwork-Application")
                          .AddConstructor<SmartphoneApplication>()
                          .AddAttribute("Id",
                                        "Id",
                                        UintegerValue(-1),
                                        MakeUintegerAccessor(&SmartphoneApplication::m_id),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Login",
                                        "Smartphone login",
                                        StringValue("login-"),
                                        MakeStringAccessor(&SmartphoneApplication::m_login),
                                        MakeStringChecker())
                          .AddAttribute("Port",
                                        "Communication port number",
                                        UintegerValue(8080),
                                        MakeUintegerAccessor(&SmartphoneApplication::m_port),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("IdDhcp",
                                        "DHCP application number",
                                        UintegerValue(0),
                                        MakeUintegerAccessor(&SmartphoneApplication::m_idDHCP),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("ChangePosition",
                                        "Movimentação máxima para notificar o servidor",
                                        DoubleValue(5.0),
                                        MakeDoubleAccessor(&SmartphoneApplication::m_changePosition),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("Start",
                                        "Time to start to send data to the UAV.",
                                        DoubleValue(0.5),
                                        MakeDoubleAccessor(&SmartphoneApplication::m_start),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("RemoteAPP", "The address of the application server",
                                        Ipv4AddressValue(),
                                        MakeIpv4AddressAccessor(&SmartphoneApplication::m_appPeer),
                                        MakeIpv4AddressChecker())
                          .AddAttribute ("PacketSink", "The sink pointer",
                                        PointerValue(),
                                        MakePointerAccessor(&SmartphoneApplication::m_sink),
                                        MakePointerChecker<PacketSink>())
                          .AddAttribute ("OnOffServerApp", "The application pointer that send packet to server",
                                        PointerValue(),
                                        MakePointerAccessor(&SmartphoneApplication::m_serverApp),
                                        MakePointerChecker<PacketSink>())
                          .AddAttribute("PathData",
                                        "Name of scenario",
                                        StringValue("none"),
                                        MakeStringAccessor(&SmartphoneApplication::m_pathData),
                                        MakeStringChecker())
                          .AddAttribute("DataRate", "Data rate of the communication.",
                                        DataRateValue(),
                                        MakeDataRateAccessor(&SmartphoneApplication::m_dataRate),
                                        MakeDataRateChecker())
                          .AddTraceSource("PacketTrace",
                                          "Packet trace",
                                          MakeTraceSourceAccessor(&SmartphoneApplication::m_packetTrace),
                                          "ns3::UavNetwork::PacketTrace")
                          ;
  return tid;
}

SmartphoneApplication::SmartphoneApplication()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("SmartphoneApplication::SmartphoneApplication @" << Simulator::Now().GetSeconds());
  m_running = false;
  m_connected = false;
}

SmartphoneApplication::~SmartphoneApplication()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  std::cout << "SmartphoneApplication::~SmartphoneApplication @" << Simulator::Now().GetSeconds() << "\n";
}

void SmartphoneApplication::SetLogin(std::string login)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds()  << login);
  m_login = login;
}

std::string
SmartphoneApplication::GetLogin()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  return m_login;
}

void SmartphoneApplication::StartApplication(void)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  m_lastPosition = GetNode()->GetObject<MobilityModel>()->GetPosition();

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
  Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (ipv4);
  for (uint32_t i = 1; i < staticRouting->GetNRoutes (); i++)
  {
    staticRouting->RemoveRoute (i); // removendo rotas padrão do static
  }

  m_socketUav = Socket::CreateSocket (GetNode(), UdpSocketFactory::GetTypeId ());
}

void SmartphoneApplication::StopApplication(void)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  m_running = false;

  Simulator::Remove(m_sendEventUav);

  if (m_socketUav)
  {
    m_socketUav->Close();
  }
}

void SmartphoneApplication::SendPacketUav(void) // envia posicionamento atual para o UAV
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_sendEventUav);

  if (m_socketUav && !m_socketUav->Connect (InetSocketAddress (m_uavPeer, m_port))) {
    std::ostringstream msg;
    Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
    msg << "CLIENT " << pos.x << " " << pos.y << " " << m_login << " MSG" << '\0';
    uint16_t packetSize = msg.str().length() + 1;
    Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);

    if (m_socketUav && m_socketUav->Send(packet, 0) == packetSize)
    {
      msg.str("");
      msg << "CLIENT\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAV";
      m_packetTrace(msg.str());
      NS_LOG_INFO ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - UAV");
    }
    else
    {
      NS_LOG_ERROR("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - UAV NAO");
      if (m_connected) {
        m_sendEventUav = Simulator::Schedule(Seconds(10.0), &SmartphoneApplication::SendPacketUav, this);
      }
      return;
    }
    NS_LOG_INFO("SmartphoneApplication::SendPacketUav " << packet->GetReferenceCount());
  } else {
    NS_LOG_INFO ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " erro ao conectar socket com servidor " << m_uavPeer);
    if (m_connected) {
      m_sendEventUav = Simulator::Schedule(Seconds(10.0), &SmartphoneApplication::SendPacketUav, this);
    }
    return;
  }

  m_sendEventUav = Simulator::Schedule(Seconds(120.0), &SmartphoneApplication::SendPacketUav, this);
}

void
SmartphoneApplication::CourseChange(Ptr<const MobilityModel> mobility)
{
  // NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  Vector actual = mobility->GetPosition();
  double distance = std::sqrt(std::pow(m_lastPosition.x - actual.x, 2) + std::pow(m_lastPosition.y - actual.y, 2));

  if (distance >= m_changePosition && m_connected)
  {
    Simulator::Remove(m_sendEventUav);
    m_lastPosition = actual;
    // TODO: enviar pacote informando novo posicionamento!
    SendPacketUav();
  }
}

void
SmartphoneApplication::TracedCallbackTxApp (Ptr<const Packet> packet, const Address &source, const Address &dest)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - SERVER ");
}

void SmartphoneApplication::TracedCallbackExpiryLease (const Ipv4Address& ip)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_sendEventUav);
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/client_" << m_id << "_expirylease" << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << ip << std::endl;
  file.close();
  os.str();
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/all_expirylease" << ".txt";
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << m_id << " "<< ip << std::endl;
  file.close();
  NS_LOG_DEBUG ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [[ perdeu IP ]]");
}

void SmartphoneApplication::TracedCallbackNewLease (const Ipv4Address& ip)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_sendEventUav);
  m_uavPeer = DynamicCast<DhcpClient>(GetNode()->GetApplication(m_idDHCP))->GetDhcpServer();
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/client_" << m_id << "_newlease" << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << ip << " " << m_uavPeer << std::endl;
  file.close();
  os.str();
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/all_newlease" << ".txt";
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << m_id << " "<< ip << " " << m_uavPeer << std::endl;
  file.close();
  NS_LOG_DEBUG ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " novo IP " << ip << " do servidor " << m_uavPeer);

  SendPacketUav();
}

void
SmartphoneApplication::PhyRxOkTrace (std::string context, Ptr<const Packet> packet, double snr, WifiMode mode, enum WifiPreamble preamble)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("CLIENT - PHYRXOK mode=" << mode << " snr=" << snr << " " << *packet);
}

void
SmartphoneApplication::PhyRxErrorTrace (std::string context, Ptr<const Packet> packet, double snr)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("CLIENT - PHYRXERROR snr=" << snr << " " << *packet);
}

void
SmartphoneApplication::PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("CLIENT - PHYTX mode=" << mode << " " << *packet);
}

void
SmartphoneApplication::TracedCallbackAssocLogger (Mac48Address mac)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_sendEventUav);
  NS_LOG_INFO ("CLIENT [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - associated to " << mac);

  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/client_" << m_id << "_assoc" << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << mac << std::endl;
  file.close();

  os.str();
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/all_assoc" << ".txt";
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << m_id << " " << mac << std::endl;
  file.close();
  m_sendEventUav = Simulator::Schedule(Seconds(20.0), &SmartphoneApplication::SendPacketUav, this);

  m_connected = true;
}

void
SmartphoneApplication::TracedCallbackDeAssocLogger (Mac48Address mac)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_sendEventUav);
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/client_" << m_id << "_deassoc" << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << mac << std::endl;
  file.close();

  os.str();
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/dhcp/all_deassoc" << ".txt";
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " " << m_id << " " << mac << std::endl;
  file.close();
  m_connected = false;
}

void SmartphoneApplication::DoDispose() {
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("SmartphoneApplication::DoDispose id " << m_id << " REF " << GetReferenceCount() << " SKT REF " << m_socketUav->GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
  Simulator::Remove(m_sendEventUav);
  m_socketUav->ShutdownRecv();
  m_socketUav->ShutdownSend();
  m_socketUav->Close();
  m_socketUav = 0;
  m_running = false;

}

void SmartphoneApplication::SetApp (std::string a)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  m_app = a;
}

std::string SmartphoneApplication::GetApp () {
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  return m_app;
}

} // namespace ns3
