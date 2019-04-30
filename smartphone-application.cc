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

#include "global-defines.h"
#include "smartphone-application.h"
#include "my-onoff-application.h"
#include "global-defines.h"

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
                          .AddAttribute("App",
                                        "Smartphone application",
                                        StringValue("NONE"),
                                        MakeStringAccessor(&SmartphoneApplication::m_app),
                                        MakeStringChecker())
                          .AddAttribute("Port",
                                        "Communication port number",
                                        UintegerValue(9090),
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
  m_stopSendingB = true;
  m_uavPeer = Ipv4Address(); // Para nao ocorrer conflito com a comparacao no newlease
  m_onoff = 0;
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
  #ifdef TCP_CLI
    m_socketUav = Socket::CreateSocket(GetNode(), SocketFactory::GetTypeId());
  #else
    m_socketUav = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  #endif
  m_running = true;
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

  if (m_connected && !m_uavPeer.IsEqual(Ipv4Address())) { // caso, tenha um ip valido para o servidor
    if (m_socketUav && !m_socketUav->Connect (InetSocketAddress (m_uavPeer, m_port))) {
      std::ostringstream msg;
      Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
      msg << "CLIENT " << pos.x << " " << pos.y << " " << m_login << " MSG" << '\0';
      uint16_t packetSize = msg.str().length() + 1;
      Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
      // NS_LOG_DEBUG("SmartphoneApplication::SendPacketUaV @" << Simulator::Now().GetSeconds() <<  " " << m_id << " conectado enviando pacote.");
      if (m_socketUav && m_socketUav->Send(packet, 0) == packetSize)
      {
        msg.str("");
        msg << "CLIENT\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAV";
        m_packetTrace(msg.str());
        NS_LOG_INFO ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - UAV");
        NS_LOG_DEBUG("SmartphoneApplication::SendPacketUav @" << Simulator::Now().GetSeconds() <<  " " << m_id << " " << m_ip << " enviado.");

        #ifdef PACKET
          std::ostringstream os;
          os << "./scratch/client/data/output/" << m_pathData << "/client/" << m_ip << ".txt";
          std::ofstream file;
          file.open(os.str(), std::ofstream::out | std::ofstream::app);
          file << Simulator::Now().GetSeconds() << " ENVIADO UAV" << std::endl; // ENVIADO
          file.close();
        #endif
      }
      else
      {
        NS_LOG_DEBUG("SmartphoneApplication::SendPacketUav @" << Simulator::Now().GetSeconds() <<  " " << m_id << " " << m_ip << " erro ao enviar.");
        NS_LOG_ERROR("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - UAV NAO");
        #ifdef PACKET
          std::ostringstream os;
          os << "./scratch/client/data/output/" << m_pathData << "/client/" << m_ip << ".txt";
          std::ofstream file;
          file.open(os.str(), std::ofstream::out | std::ofstream::app);
          file << Simulator::Now().GetSeconds() << " FALHA UAV" << std::endl; // FALHA
          file.close();
        #endif
        if (m_connected) {
          m_sendEventUav = Simulator::Schedule(Seconds(1.0), &SmartphoneApplication::SendPacketUav, this);
        }
        return;
      }
      NS_LOG_INFO("SmartphoneApplication::SendPacketUav " << packet->GetReferenceCount());
    } else {
      NS_LOG_DEBUG("SmartphoneApplication::SendPacketUav @" << Simulator::Now().GetSeconds() << " " << m_id << " " << m_ip << " erro ao conectar com o servidor.");
      NS_LOG_INFO ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " erro ao conectar socket com servidor " << m_uavPeer);
      #ifdef PACKET
        std::ostringstream os;
        os << "./scratch/client/data/output/" << m_pathData << "/client/" << m_ip << ".txt";
        std::ofstream file;
        file.open(os.str(), std::ofstream::out | std::ofstream::app);
        file << Simulator::Now().GetSeconds() << " NAO_CONECTADO UAV" << std::endl; // NAO CONECTADO
        file.close();
      #endif
      return;
    }
    m_sendEventUav = Simulator::Schedule(Seconds(1.0), &SmartphoneApplication::SendPacketUav, this);
  }  
}

void
SmartphoneApplication::CourseChange(Ptr<const MobilityModel> mobility)
{
  // NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  Vector actual = mobility->GetPosition();
  double distance = std::sqrt(std::pow(m_lastPosition.x - actual.x, 2) + std::pow(m_lastPosition.y - actual.y, 2));

  if (distance >= m_changePosition*2 && m_connected)
  {
    m_lastPosition = actual;
    Simulator::Remove(m_sendEventUav);
    m_sendEventUav = Simulator::ScheduleNow(&SmartphoneApplication::SendPacketUav, this);
  }
}

void
SmartphoneApplication::TracedCallbackRxApp (Ptr<const Packet> packet, const Address & address)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds()  << packet << address);
  if (m_running) {
    uint8_t *buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::string msg = std::string(buffer, buffer + packet->GetSize());
    std::istringstream iss(msg);
    std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>());

    NS_LOG_DEBUG ("\tSmartphoneApplication::TracedCallbackRxApp " << m_login << " :: " << msg << " @" << Simulator::Now().GetSeconds());

    if (results.at(0).compare("CLIENTLOC") == 0) {
      Simulator::Remove(m_sendEventUav);
      NS_LOG_DEBUG("\tCLIENT #" << m_id << " recebeu CLIENTLOC");
      m_sendEventUav = Simulator::ScheduleNow(&SmartphoneApplication::SendPacketUav, this);
      if (m_stopSendingB) {
        m_stopSending = Simulator::Schedule(Seconds(5.0), &SmartphoneApplication::StopSendingPosition, this);
        m_stopSendingB = false;
      }
    } else if (results.at(0).compare("CLIENTOK") == 0) {
      Simulator::Remove(m_sendEventUav);
      Simulator::Remove(m_stopSending);
      m_stopSendingB = true;
      NS_LOG_DEBUG("\tCLIENT #" << m_id << " recebeu CLIENTOK");
    }
  }
}

void
SmartphoneApplication::StopSendingPosition ()
{
  Simulator::Remove(m_sendEventUav);
  Simulator::Remove(m_stopSending);
  m_stopSendingB = true;
}

void
SmartphoneApplication::TracedCallbackTxApp (Ptr<const Packet> packet, const Address &source, const Address &dest)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  // NS_LOG_DEBUG ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - ENVIANDO APP PARA UAV ");
  std::ostringstream os;
  os << "./scratch/client/data/output/" << m_pathData << "/client/" << m_ip << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << " ENVIADO " << m_appOnoff << " " << packet->GetSize () << " " << m_login << std::endl; // ENVIADO por um cliente
  file.close();
}

void SmartphoneApplication::TracedCallbackExpiryLease (const Ipv4Address& ip)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );  
  m_ip = Ipv4Address();
  #ifdef DHCP
    std::ostringstream os;
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/client_lease_" << m_id << ".txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " EXPIRYLEASE " << ip << std::endl;
    file.close();
    os.str("");
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/all_expirylease.txt";
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " " << m_id << " "<< ip << " " << m_uavPeer << std::endl;
    file.close();
  #endif
  NS_LOG_DEBUG ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [[ perdeu IP ]]");
}

void SmartphoneApplication::TracedCallbackNewLease (const Ipv4Address& ip)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  m_ip = ip;
  #ifdef COM_SERVER
    if (m_connected && !DynamicCast<DhcpClient>(GetNode()->GetApplication(m_idDHCP))->GetDhcpServer().IsEqual(m_uavPeer))
    { // caso o servidor tenha sido alterado
      // criar uma aplicação onoff com base na aplicação que o usuario esta configurado m_app
      if (m_onoff) {
        m_onoff->SetStopTime(Simulator::Now()); // na roxima avaliacao do onoff será finalizada a aplicacao!
      }
      m_uavPeer = DynamicCast<DhcpClient>(GetNode()->GetApplication(m_idDHCP))->GetDhcpServer();
      ConfigureApplication(m_uavPeer); // cria nova aplicacao para envio ao novo servidor!
    }  
  #endif
  #ifdef DHCP
    std::ostringstream os;
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/client_lease_" << m_id << ".txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " NEWLEASE " << ip << " " << m_uavPeer << std::endl;
    file.close();
    os.str("");
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/all_newlease.txt";
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " " << m_id << " "<< ip << " " << m_uavPeer << std::endl;
    file.close();
  #endif
  NS_LOG_DEBUG ("CLIENTE [" << m_id << "] @" << Simulator::Now().GetSeconds() << " novo IP " << ip << " do servidor " << m_uavPeer);  
}

// void
// SmartphoneApplication::PhyOkTrace (std::string context, Ptr<const Packet> packet, double snr, WifiMode mode, enum WifiPreamble preamble)
// {
//   NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
//   NS_LOG_INFO("CLIENT - PHYOK mode=" << mode << " snr=" << snr << " " << *packet);
// }

// void
// SmartphoneApplication::PhyRxErrorTrace (std::string context, Ptr<const Packet> packet, double snr)
// {
//   NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
//   NS_LOG_INFO("CLIENT - PHYRXERROR snr=" << snr << " " << *packet);
// }

// void
// SmartphoneApplication::PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower)
// {
//   NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
//   NS_LOG_INFO("CLIENT - PHYTX mode=" << mode << " " << *packet);
// }

void
SmartphoneApplication::TracedCallbackAssocLogger (Mac48Address mac)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_INFO ("CLIENT [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - associated to " << mac);

  #ifdef DHCP
    std::ostringstream os;
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/client_" << m_id << ".txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " ASSOC " << mac << std::endl;
    file.close();

    os.str("");
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/all_soc.txt";
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " ASSOC " << m_id << " " << mac << std::endl;
    file.close();
  #endif
  m_connected = true;
}

void
SmartphoneApplication::TracedCallbackDeAssocLogger (Mac48Address mac)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  #ifdef DHCP
    std::ostringstream os;
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/client_" << m_id << ".txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " DEASSOC " << mac << std::endl;
    file.close();

    os.str("");
    os << "./scratch/client/data/output/" << m_pathData << "/dhcp/all_soc.txt";
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " DEASSOC " << m_id << " " << mac << std::endl;
    file.close();
  #endif
  m_connected = false;
  Simulator::Remove(m_sendEventUav);
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

void SmartphoneApplication::SetNode(Ptr<Node> node) 
{
  m_node = node;
}

void SmartphoneApplication::ConfigureApplication (const Ipv4Address& ip)
{
  std::ofstream cliLogin;
  std::ostringstream ss;
  ss.str("");
  ss << "./scratch/flynetwork/data/output/" << m_pathData << "/client/client_" << m_id << ".txt";
  cliLogin.open(ss.str().c_str(), std::ofstream::out | std::ofstream::app);
  cliLogin << Simulator::Now().GetSeconds() << " CONFIGURE " << m_login;
  // configure OnOff application para server    
  int port = 0;
  ObjectFactory onoffFac;
  m_onoff = 0;
  m_appOnoff = m_app; // atualizando a aplicacao que esta sendo configurado o onoff!
  if (m_app.compare ("VOICE") != 0) { // VOICE
      onoffFac.SetTypeId ("ns3::MyOnOffApplication");
      onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
      onoffFac.Set ("PacketSize", UintegerValue (50));
      onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
      onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      // P.S.: offTime + DataRate/PacketSize = next packet time
      onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.024Mbps")));
      port = 5060;
      onoffFac.Set ("Remote", AddressValue (InetSocketAddress (ip, port)));
      m_onoff = onoffFac.Create<Application> ();
      m_onoff->SetStartTime(Seconds(1));
      m_onoff->SetStopTime(Seconds(ETAPA)); // considerando 111 minutos mensal, 3.7 diario - http://www.teleco.com.br/comentario/com631.asp
      m_onoff->TraceConnectWithoutContext ("TxWithAddresses", MakeCallback (&SmartphoneApplication::TracedCallbackTxApp, this));
      m_node->AddApplication (m_onoff);
      cliLogin << " VOICE" << std::endl;
  } else if (m_app.compare ("VIDEO") != 0) { // VIDEO
      onoffFac.SetTypeId ("ns3::MyOnOffApplication");
      #ifdef TCP_CLI
        onoffFac.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
      #else
        onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
      #endif
      onoffFac.Set ("PacketSize", UintegerValue (429));
      onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
      onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      // P.S.: offTime + DataRate/PacketSize = next packet time
      onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.128Mbps")));
      port = 5070;
      onoffFac.Set ("Remote", AddressValue (InetSocketAddress (ip, port)));
      m_onoff = onoffFac.Create<Application> ();
      m_onoff->SetStartTime(Seconds(1.0));
      m_onoff->SetStopTime(Seconds(ETAPA)); 
      m_onoff->TraceConnectWithoutContext ("TxWithAddresses", MakeCallback (&SmartphoneApplication::TracedCallbackTxApp, this));
      m_node->AddApplication (m_onoff);
      cliLogin << " VIDEO" << std::endl;
  } else if (m_app.compare ("WWW") != 0) { // WWW
      onoffFac.SetTypeId ("ns3::MyOnOffApplication");
      #ifdef TCP_CLI
        onoffFac.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
      #else
        onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
      #endif
      onoffFac.Set ("PacketSize", UintegerValue (429));
      onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
      onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.04]"));
      // P.S.: offTime + DataRate/PacketSize = next packet time
      onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.128Mbps")));
      port = 8080;
      onoffFac.Set ("Remote", AddressValue (InetSocketAddress (ip, port)));
      m_onoff = onoffFac.Create<Application> ();
      m_onoff->SetStartTime(Seconds(1.0));
      m_onoff->SetStopTime(Seconds(ETAPA));
      m_onoff->TraceConnectWithoutContext ("TxWithAddresses", MakeCallback (&SmartphoneApplication::TracedCallbackTxApp, this));
      m_node->AddApplication (m_onoff);
      cliLogin << " WWW" << std::endl;
  } else if (m_app.compare ("NOTHING") != 0) { // NOTHING
      cliLogin << " NOTHING" << std::endl;
  } else
    NS_FATAL_ERROR ("UavNetwork .. application error");
}

} // namespace ns3
