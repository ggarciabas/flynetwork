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
#include "client-device-energy-model.h"

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
                          .AddAttribute("PathData",
                                        "Name of scenario",
                                        StringValue("none"),
                                        MakeStringAccessor(&UavApplication::m_pathData),
                                        MakeStringChecker())
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
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO ("UavApplication::UavApplication @" << Simulator::Now().GetSeconds());
  m_running = false;
  m_meanConsumption = 0.0;
}

UavApplication::~UavApplication()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO ("UavApplication::~UavApplication @" << Simulator::Now().GetSeconds());
}

void UavApplication::Start(double stoptime) {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << stoptime);
  NS_LOG_DEBUG("UavApplication::Start [" << m_id << "]");
  Simulator::Remove(m_startEvent);
  Simulator::Remove(m_stopEvent);
  // SetStartTime(Simulator::Now());
  SetStopTime(Seconds(stoptime));
  StartApplication();
}

void UavApplication::StartApplication(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::StartApplication [" << m_id << "]");
  m_running = true;
  // criando socket para enviar informacoes ao servidor
  m_sendSck = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
  if (m_sendSck->Connect(InetSocketAddress(m_peer, m_serverPort))) {
    NS_FATAL_ERROR ("UAV - $$ [NÃO] conseguiu conectar com Servidor!");
  }
  ScheduleTx();
}

void UavApplication::Stop() {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::Stop [" << m_id << "]");
  // SetStopTime(Simulator::Now());
  Simulator::Remove(m_stopEvent);
  StopApplication();
}

void UavApplication::StopApplication()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::StopApplication [" << m_id << "]");
  m_meanConsumption = 0.0;
  Simulator::Remove(m_stopEvent);
  m_running = false;
  Simulator::Remove(m_sendEvent);
  Simulator::Remove(m_sendCliDataEvent);
  Simulator::Remove(m_packetDepletion);
  if (m_sendSck)
  {
    m_sendSck->ShutdownRecv();
    m_sendSck->ShutdownSend();
    m_sendSck->Close();
    m_sendSck = 0;
  }
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
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << mob);
  // Obs.: verificar UavMobility para tirar duvidas, este somente é executado quando chega ao destino!
  NS_LOG_INFO ("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " course changed!!! ---- ~~~~");

  Simulator::Remove(m_sendEvent);
  NS_LOG_INFO("UAV #" << m_id << " chegou ao seu posicionamento final.");
  SendPacket();

  Ptr<UavDeviceEnergyModel> dev = GetNode()->GetObject<UavDeviceEnergyModel>();
  double energy = dev->ChangeThreshold(); // atualiza valor minimo para retorno na bateria
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/course_changed/course_changed_" << m_id << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << "," <<  mob->GetPosition().x << "," << mob->GetPosition().y << "," << energy << std::endl;
  file.close();
  dev->StartHover();

  // clear ClientModelContainer based on last update time
  NS_LOG_DEBUG ("UavApplication::CourseChange UAV " << m_id << " total " << m_client.GetN());
  for (ClientModelContainer::Iterator it = m_client.Begin(); it != m_client.End(); ++it) {
     NS_LOG_DEBUG ("\t" << (*it)->GetLogin());
  }

  int i = m_client.GetN()-1;
  Ptr<ClientDeviceEnergyModel> c_dev = GetNode()->GetObject<ClientDeviceEnergyModel>();
  for (; i >= 0; i--)
  {
    NS_LOG_DEBUG ("\t- [" << i << "] Cliente " << m_client.Get(i)->GetLogin());
    if ((Simulator::Now().GetSeconds() - m_client.Get(i)->GetUpdatePos().GetSeconds()) > 60.0) {
      NS_LOG_DEBUG ("\t\t- removendo " << m_client.Get(i)->GetLogin());
      m_client.RemoveAt(i);
      c_dev->RemoveClient();
    }
  }
}

void
UavApplication::SetId(uint32_t id)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << id);
  m_id = id;
}

uint32_t
UavApplication::GetId()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_id;
}

void
UavApplication::EnergyRechargedCallback()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("---- EnergyRechargedCallback ");
}

void
UavApplication::EnergyDepletionCallback()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("---->>> EnergyDepletionCallback - ASK SAFE POSITION");
  // avisar central e mudar posição para central!
  m_packetDepletion = Simulator::ScheduleNow(&UavApplication::SendPacketDepletion, this);
}

void UavApplication::SendPacketDepletion(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  m_packetDepletion.Cancel();
  std::ostringstream msg;
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  msg << "DEPLETION " << m_id << " " << pos.x << " " << pos.y << " " << pos.z << " " << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (m_sendSck->Send(packet) == packetSize)
  {
    msg.str("");
    msg << "UAV\t" << m_id << "\tSENT DEPLETION\t" << Simulator::Now().GetSeconds() << "\tSERVER";
    m_packetTrace(msg.str());
    NS_LOG_INFO("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - SERVER ::: SENT DEPLETION.");
  }
  else
  {
    NS_LOG_ERROR("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [NÃO] SENT DEPLETION");
    m_packetDepletion = Simulator::Schedule(Seconds(0.01), &UavApplication::SendPacketDepletion, this);
    return;
  }
  m_packetDepletion = Simulator::Schedule(Seconds(0.5), &UavApplication::SendPacketDepletion, this);
}

void
UavApplication::TracedCallbackRxApp (Ptr<const Packet> packet, const Address & address)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << packet << address);
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
    } else if (results.at(0).compare("GOTO") == 0)
      {
        std::ostringstream mm;
        mm << "UAV\t" << m_id << "\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tGOTO";
        m_packetTrace(mm.str());
        double z = std::stod(results.at(3), &sz) - GetNode()->GetObject<MobilityModel>()->GetPosition().z;
        // mudar o posicionamento do UAV
        Ptr<UavDeviceEnergyModel> dev = GetNode()->GetObject<UavDeviceEnergyModel>();
        dev->StopHover();
        GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(std::stod(results.at(1), &sz), std::stod(results.at(2), &sz), (z > 0) ? z : 0.0)); // Verficar necessidade de subir em no eixo Z
        // repply to server
        ReplyServer();
      } else if (results.at(0).compare("GOTOCENTRAL") == 0)
        {
          m_packetDepletion.Cancel();
          std::ostringstream mm;
          mm << "UAV\t" << m_id << "\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tGOTOCENTRAL";
          m_packetTrace(mm.str());
          double z = std::stod(results.at(3), &sz) - GetNode()->GetObject<MobilityModel>()->GetPosition().z;
          // mudar o posicionamento do UAV
          Ptr<UavDeviceEnergyModel> dev = GetNode()->GetObject<UavDeviceEnergyModel>();
          dev->StopHover();
          GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(std::stod(results.at(1), &sz), std::stod(results.at(2), &sz), (z > 0) ? z : 0.0)); // Verficar necessidade de subir em no eixo Z
          // repply to server
          ReplyServerCentral();
        } else if (results.at(0).compare("SERVERDATA") == 0)
          {
            SendCliData();
          }

      results.clear();
  }
}

void
UavApplication::ReplyServerCentral ()
{ // confirma recebimento do posicionamento para o servidor
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  if (m_running) {
    std::ostringstream m;
    m << "CENTRALOK " << m_id << "\0";
    uint16_t packetSize = m.str().length() + 1;
    Ptr<Packet> packet = Create<Packet>((uint8_t *)m.str().c_str(), packetSize);
    if (m_sendSck && m_sendSck->Send(packet) == packetSize)
    {
      m.str("");
      m << "UAV\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tCENTRALOK";
      m_packetTrace(m.str());
      NS_LOG_INFO("UAV #" << m_id << " enviando CENTRALOK");
    }
    else
    {
      NS_LOG_ERROR("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [NÃO] CENTRALOK");
      Simulator::Schedule(Seconds(0.5), &UavApplication::ReplyServerCentral, this);
    }
  }
}

void
UavApplication::ReplyServer ()
{ // confirma recebimento do posicionamento para o servidor
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
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
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << packet << address);
  if (m_running) {
    NS_LOG_INFO ("UavApplication::TracedCallbackRxAppInfra @" << Simulator::Now().GetSeconds());
    uint8_t *buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::string msg = std::string(buffer, buffer + packet->GetSize());
    std::istringstream iss(msg);
    std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>());

    NS_LOG_INFO ("UavApplication::TracedCallbackRxAppInfra :: " << msg << " @" << Simulator::Now().GetSeconds());

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
        Ptr<ClientDeviceEnergyModel> c_dev = GetNode()->GetObject<ClientDeviceEnergyModel>();
        c_dev->AddClient();
      }
      cli->SetPosition(pos.at(0), pos.at(1));
      cli->SetUpdatePos (Simulator::Now());
    }
    results.clear();

  }
}

void UavApplication::SendCliData ()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  if (m_running) {
    Simulator::Remove(m_sendCliDataEvent);
    std::ostringstream msg;
    msg << "DATA " << m_id << " " << GetNode()->GetObject<UavDeviceEnergyModel>()->GetEnergySource()->GetRemainingEnergy();
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
      NS_LOG_INFO("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - DATA to Server.");
    }

    m_sendCliDataEvent = Simulator::Schedule (Seconds(5.0), &UavApplication::SendCliData, this);
  }
}

void UavApplication::SendPacket(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
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
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  if (m_running)
  {
    m_sendEvent = Simulator::Schedule(Seconds(m_updateTime), &UavApplication::SendPacket, this);
  }
}

void UavApplication::DoDispose() {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO ("UavApplication::DoDispose id " << m_id << " REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
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

void UavApplication::TotalLeasedTrace (int oldV, int newV)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << oldV << newV);
  NS_LOG_DEBUG ("UavApplication::TotalLeasedTrace " << newV);
  m_totalLeased = newV;
}

void UavApplication::TotalEnergyConsumptionTrace (double oldV, double newV)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << oldV << newV);
  m_meanConsumption += (newV - oldV);
  m_meanConsumption /= 2.0;
}

} // namespace ns3
