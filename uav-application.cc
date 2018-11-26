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

#include "uav-application.h"

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "uav-device-energy-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavApplication");

NS_OBJECT_ENSURE_REGISTERED(UavApplication);

TypeId
UavApplication::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::UavApplication")
                          .SetParent<Application>()
                          .SetGroupName("Flynetwork-Application")
                          .AddConstructor<UavApplication>()
                          .AddAttribute("Id",
                                        "Uav model id",
                                        UintegerValue(-1),
                                        MakeUintegerAccessor(&UavApplication::m_id),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("UpdateTime",
                                        "Time to update location to the server.",
                                        DoubleValue(15.0),
                                        MakeDoubleAccessor(&UavApplication::m_updateTime),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("CliUpdateTime",
                                        "Time to update consumption of clients to server.",
                                        DoubleValue(55.0),
                                        MakeDoubleAccessor(&UavApplication::m_cliUpdateTime),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("Remote", "The address of the destination",
                                        Ipv4AddressValue(),
                                        MakeIpv4AddressAccessor(&UavApplication::m_peer),
                                        MakeIpv4AddressChecker())
                          .AddAttribute("DataRate", "Data rate of the communication.",
                                        DataRateValue(),
                                        MakeDataRateAccessor(&UavApplication::m_dataRate),
                                        MakeDataRateChecker())
                          .AddAttribute("ServerPort",
                                        "Communication port number",
                                        UintegerValue(8080),
                                        MakeUintegerAccessor(&UavApplication::m_serverPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("ClientPort",
                                        "Communication port number",
                                        UintegerValue(8080),
                                        MakeUintegerAccessor(&UavApplication::m_cliPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("AdhocAddress", "The address of the adhoc interface node",
                                        Ipv4AddressValue(),
                                        MakeIpv4AddressAccessor(&UavApplication::m_addressAdhoc),
                                        MakeIpv4AddressChecker())
                          .AddAttribute ("UavEnergySource", "Uav Energy Source",
                                        PointerValue(),
                                        MakePointerAccessor(&UavApplication::m_uavEnergySource),
                                        MakePointerChecker<UavEnergySource>())
                          .AddTraceSource("PacketTrace",
                                          "Packet trace",
                                          MakeTraceSourceAccessor(&UavApplication::m_packetTrace),
                                          "ns3::UavNetwork::PacketTrace");
  return tid;
}

UavApplication::UavApplication()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("UavApplication::UavApplication @" << Simulator::Now().GetSeconds());
  m_running = false;
  m_meanConsumption = 0.0;
}

UavApplication::~UavApplication()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("UavApplication::~UavApplication @" << Simulator::Now().GetSeconds());
}

void UavApplication::Start(double stoptime) {
  Simulator::Remove(m_startEvent);
  StartApplication();
  Simulator::Remove(m_stopEvent);
  m_stopEvent = Simulator::Schedule (Seconds(stoptime), &UavApplication::StopApplication, this);
}

void UavApplication::Stop() {
  m_meanConsumption = 0.0;
  Simulator::Remove(m_startEvent);
  Simulator::Remove(m_stopEvent);
  StopApplication();
}

// void UavApplication::SetTurnOffWifiPhyCallback(UavApplication::OffWifiPhyCallback adhoc, UavApplication::OffWifiPhyCallback infra)
// {
//   m_setOffWifiPhyAdhoc = adhoc;
//   m_setOffWifiPhyInfra = infra;
// }
//
// void UavApplication::TurnOffWifiPhy ()
// {
//   m_setOffWifiPhyAdhoc();
//   m_setOffWifiPhyInfra();
// }

void
UavApplication::CourseChange (Ptr<const MobilityModel> mob)
{
  // verificar UavMobility para tirar duvidas, este somente é executado quando chega ao destino!
  NS_LOG_INFO ("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " course changed!!! ---- ~~~~");
  Simulator::Remove(m_sendEvent);
  NS_LOG_INFO("UAV #" << m_id << " chegou ao seu posicionamento final.");
  SendPacket();

  Ptr<UavDeviceEnergyModel> dev = GetNode()->GetObject<UavDeviceEnergyModel>();
  dev->StartHover();
}

void
UavApplication::SetId(uint32_t id)
{
  m_id = id;
}

uint32_t
UavApplication::GetId()
{
  return m_id;
}

void
UavApplication::EnergyDepletionCallback()
{
  std::cout << "---- EnergyDepletionCallback ##$#$#$#$#$#$\n";
}

void
UavApplication::TracedCallbackRxApp (Ptr<const Packet> packet, const Address & address)
{
  if (m_running) {
    uint8_t *buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::string msg = std::string(buffer, buffer + packet->GetSize());
    std::istringstream iss(msg);
    std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>());
    std::string::size_type sz; // alias of size_t
    if (results.at(0).compare("SERVEROK") == 0) {
      Simulator::Remove(m_sendEvent);
      NS_LOG_INFO("UAV #" << m_id << " recebeu SERVEROK");
    } else if (results.at(0).compare("SERVER") == 0)
      {
        std::ostringstream mm;
        mm << "UAV\t" << m_id << "\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tSERVER";
        m_packetTrace(mm.str());
        double z = std::stod(results.at(3), &sz) - GetNode()->GetObject<MobilityModel>()->GetPosition().z;
        // mudar o posicionamento do UAV
        Ptr<UavDeviceEnergyModel> dev = GetNode()->GetObject<UavDeviceEnergyModel>();
        dev->StopHover();
        GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(std::stod(results.at(1), &sz), std::stod(results.at(2), &sz), (z > 0) ? z : 0.0)); // Verficar necessidade de subir em no eixo Z
        // repply to server
        ReplyServer();
      } else if (results.at(0).compare("SERVERDATA") == 0)
        {
          SendCliData();
        } else if (results.at(0).compare("DATAOK") == 0)
          {
            Simulator::Remove(m_sendCliDataEvent);
            m_client.Clear();
            m_meanConsumption = 0.0;
            NS_LOG_DEBUG("UAV #" << m_id << " recebeu DATAOK @" << Simulator::Now().GetSeconds());
          }

      results.clear();

      //packet->Unref();
  }
}

void
UavApplication::ReplyServer ()
{ // confirma recebimento do posicionamento para o servidor
  if (m_running) {
    std::ostringstream m;
    m << "UAVRECEIVED " << m_id << "\0";
    uint16_t packetSize = m.str().length() + 1;
    Ptr<Packet> packet = Create<Packet>((uint8_t *)m.str().c_str(), packetSize);
    if (m_sendSck && m_sendSck->Send(packet) == packetSize)
    {
      m.str("");
      m << "UAV\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAVRECEIVED";
      m_packetTrace(m.str());
      NS_LOG_INFO("UAV #" << m_id << " enviando UAVRECEIVED");
    }
    else
    {
      NS_LOG_ERROR("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [NÃO] UAVRECEIVED");
      Simulator::Schedule(Seconds(0.5), &UavApplication::ReplyServer, this);
    }
  }
}

void
UavApplication::TracedCallbackRxAppInfra (Ptr<const Packet> packet, const Address & address)
{
  if (m_running) {
    NS_LOG_INFO ("UavApplication::TracedCallbackRxAppInfra @" << Simulator::Now().GetSeconds());
    uint8_t *buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::string msg = std::string(buffer, buffer + packet->GetSize());
    std::istringstream iss(msg);
    std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>());

    NS_LOG_DEBUG ("UavApplication::TracedCallbackRxAppInfra :: " << msg << " @" << Simulator::Now().GetSeconds());

    if (results.at(0).compare("CLIENT") == 0) { // received a message from a client
      std::string::size_type sz; // alias of size_t
      std::vector<double> pos;
      pos.push_back(std::stod (results.at(1),&sz));
      pos.push_back(std::stod (results.at(2),&sz));
      Ptr<ClientModel> cli = NULL;
      cli = m_client.FindClientModel(results.at(3));
      if (cli == NULL) { // nao existe cadastro por addr
        ObjectFactory obj;
        obj.SetTypeId("ns3::ClientModel");
        obj.Set("Login", StringValue(results.at(3)));
        cli = obj.Create()->GetObject<ClientModel>();
        m_client.Add(cli);
      }
      cli->SetPosition(pos.at(0), pos.at(1));
      cli->SetUpdatePos (Simulator::Now());
    }
    results.clear();

  }
}

void UavApplication::StartApplication(void)
{
  m_running = true;
  // criando socket para enviar informacoes ao servidor
  m_sendSck = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
  if (m_sendSck->Connect(InetSocketAddress(m_peer, m_serverPort))) {
    NS_FATAL_ERROR ("UAV - $$ [NÃO] conseguiu conectar com Servidor!");
  }

  ScheduleTx();
}

void UavApplication::SendCliData ()
{
  if (m_running) {
    Simulator::Remove(m_sendCliDataEvent);
    std::ostringstream msg;
    msg << "DATA " << m_id << " " << GetNode()->GetObject<UavDeviceEnergyModel>()->GetEnergySource()->GetRemainingEnergy() << " 0";
    // msg << "CONSUMPTION " << m_id << " " << GetNode()->GetObject<UavDeviceEnergyModel>()->GetEnergySource()->GetRemainingEnergy() << " " << m_meanConsumption;
    for (ClientModelContainer::Iterator it = m_client.Begin(); it != m_client.End(); ++it) {
        msg << " " << (*it)->GetLogin();
        msg << " " << (*it)->GetUpdatePos().GetSeconds();
        msg << " " << (*it)->GetPosition().at(0) << " " << (*it)->GetPosition().at(1);
    }
    msg << " \0";
    uint16_t packetSize = msg.str().length() + 1;
    Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
    NS_LOG_INFO ("UavApplication::SendCliData " << msg.str()<< " @" << Simulator::Now().GetSeconds());
    if (m_sendSck->Send(packet) == packetSize)
    {
      msg.str("");
      msg << "UAV\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tDATAtoSERVER";
      m_packetTrace(msg.str());
      NS_LOG_DEBUG("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - DATA to Server.");
    }

    m_sendCliDataEvent = Simulator::Schedule (Seconds(5.0), &UavApplication::SendCliData, this);
  }
}

void UavApplication::StopApplication(void)
{
  m_running = false;

  if (m_sendEvent.IsRunning())
  {
    Simulator::Remove(m_sendEvent);
  }

  if (m_sendSck)
  {
    m_sendSck->ShutdownRecv();
    m_sendSck->ShutdownSend();
    m_sendSck->Close();
    m_sendSck = 0;
  }
}

void UavApplication::SendPacket(void)
{
  Simulator::Remove(m_sendEvent);

  std::ostringstream msg;
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  msg << "UAV " << pos.x << " " << pos.y << " " << pos.z << " " << m_id << " " << GetNode()->GetObject<UavDeviceEnergyModel>()->GetEnergySource()->GetRemainingEnergy() << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);

  if (m_sendSck->Send(packet) == packetSize)
  {
    msg.str("");
    msg << "UAV\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tSERVER";
    m_packetTrace(msg.str());
    NS_LOG_INFO("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - SERVER ::: posicionamento do UAV para o servidor.");
  }
  else
  {
    NS_LOG_ERROR("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [NÃO] conseguiu enviar a mensagem para o servidor");
  }
  ScheduleTx();
}

void UavApplication::ScheduleTx(void)
{
  if (m_running)
  {
    m_sendEvent = Simulator::Schedule(Seconds(m_updateTime), &UavApplication::SendPacket, this);
  }
}

void UavApplication::DoDispose() {
  NS_LOG_DEBUG ("UavApplication::DoDispose id " << m_id << " REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
  Simulator::Remove(m_sendEvent);
  Simulator::Remove(m_startEvent);
  Simulator::Remove(m_stopEvent);
  if (m_sendSck) {
    m_sendSck->ShutdownRecv();
    m_sendSck->ShutdownSend();
    m_sendSck->Close();
    m_sendSck = 0;
  }
  m_running = false;
}

void UavApplication::TotalEnergyConsumptionTrace (double oldV, double newV)
{
  m_meanConsumption += (newV - oldV);
  m_meanConsumption /= 2.0;
}

} // namespace ns3
