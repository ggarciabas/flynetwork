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

#define ETAPA 300

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
  m_goto.push_back(0.0); // inicializando para nao ocorrer conflito
  m_goto.push_back(0.0);
  m_central.push_back(0.0); // inicializando para nao ocorrer conflito
  m_central.push_back(0.0);
  m_depletion = false;
}

void UavApplication::Reset () {
  m_depletion = false;
}

UavApplication::~UavApplication()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO ("UavApplication::~UavApplication @" << Simulator::Now().GetSeconds());
}

void UavApplication::Start(double stoptime) {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << stoptime);
  NS_LOG_INFO("UavApplication::Start [" << m_id << "]");
  Simulator::Remove(m_startEvent);
  Simulator::Remove(m_stopEvent);
  // SetStartTime(Simulator::Now());
  SetStopTime(Seconds(stoptime));
  StartApplication();
}

void UavApplication::StartApplication(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("UavApplication::StartApplication [" << m_id << "]");
  m_running = true;
  // criando socket para enviar informacoes ao servidor
  m_sendSck = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
  if (m_sendSck->Connect(InetSocketAddress(m_peer, m_serverPort))) {
    NS_FATAL_ERROR ("UAV - $$ [NÃO] conseguiu conectar com Servidor!");
  }
  // SendPacket();
}

void UavApplication::Stop() {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("UavApplication::Stop [" << m_id << "]");
  // SetStopTime(Simulator::Now());
  Simulator::Remove(m_stopEvent);
  StopApplication();
}

void UavApplication::StopApplication()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO("UavApplication::StopApplication [" << m_id << "]");
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
  NS_LOG_DEBUG ("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " course changed!!! ---- ~~~~");

  Simulator::Remove(m_sendEvent);
  NS_LOG_DEBUG("UAV #" << m_id << " chegou ao seu posicionamento final.");
  SendPacket();

  if (m_depletion) {
    return; // nao fazer topicos abaixo em estado de emergencia
  }

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
  NS_LOG_DEBUG ("UavApplication::CourseChange UAV " << m_id << " totalCli " << m_client.GetN());
  // for (ClientModelContainer::Iterator it = m_client.Begin(); it != m_client.End(); ++it) {
  //    NS_LOG_DEBUG ("\t" << (*it)->GetLogin());
  // }

  int i = m_client.GetN()-1;
  Ptr<ClientDeviceEnergyModel> c_dev = GetNode()->GetObject<ClientDeviceEnergyModel>();
  for (; i >= 0; i--)
  {
    // NS_LOG_DEBUG ("\t- [" << i << "] Cliente " << m_client.Get(i)->GetLogin());
    if ((Simulator::Now().GetSeconds() - m_client.Get(i)->GetUpdatePos().GetSeconds()) > ETAPA) {
      // NS_LOG_DEBUG ("\t\t- removendo " << m_client.Get(i)->GetLogin());
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
  NS_LOG_DEBUG("---->>> EnergyDepletionCallback");
  std::cout << "---->>> EnergyDepletionCallback\n";
  m_depletion = true;
  // avisar central e mudar posição para central!
  m_packetDepletion = Simulator::ScheduleNow(&UavApplication::SendPacketDepletion, this);
  // Ir para central
  Ptr<UavDeviceEnergyModel> dev = GetNode()->GetObject<UavDeviceEnergyModel>();
  dev->StopHover();
  GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(m_central.at(0), m_central.at(1), 1.0)); // Verficar necessidade de subir em no eixo Z
}

/*
  Pacote para envio de informacao de onde o UAV estava afim de se repor aquela localizacao, o UAV será redirecionado para a central pois este só tem bateria para voltar!
*/
void UavApplication::SendPacketDepletion(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  m_packetDepletion.Cancel();

  if (m_running) {
    std::ostringstream m;
    Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
    m << "DEPLETION " << m_id << " " << pos.x << " " << pos.y << " " << pos.z << " " << '\0';
    uint16_t packetSize = m.str().length() + 1;
    Ptr<Packet> packet = Create<Packet>((uint8_t *)m.str().c_str(), packetSize);
    if (m_sendSck && m_sendSck->Send(packet) == packetSize)
    {
      m.str("");
      m << "UAV\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAVRECEIVED";
      //m_packetTrace(m.str());
      NS_LOG_INFO("UAV #" << m_id << " enviando UAVRECEIVED");
    }
    else
    {
      NS_LOG_ERROR("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [NÃO] SENT DEPLETION");
      m_packetDepletion = Simulator::Schedule(Seconds(0.01), &UavApplication::SendPacketDepletion, this);
      return;
    }
  } else {
    NS_LOG_DEBUG("Uav not running\n");
  }
  
  m_packetDepletion = Simulator::Schedule(Seconds(0.05), &UavApplication::SendPacketDepletion, this);
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
      NS_LOG_DEBUG("UAV #" << m_id << " recebeu SERVEROK");
    } else if (results.at(0).compare("DATAOK") == 0) {
        Simulator::Remove(m_sendCliDataEvent);
        NS_LOG_DEBUG("UAV #" << m_id << " recebeu DATAOK");
      } else if (results.at(0).compare("GOTO") == 0)
      {
        if (m_depletion) return; // nao deixar executar acoes a nao ser voltar para a central em estado de emergencia        
        std::ostringstream mm;
        mm << "UAV\t" << m_id << "\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tGOTO";
        //m_packetTrace(mm.str());
        NS_LOG_DEBUG(mm.str() << " @" << Simulator::Now().GetSeconds());
        double z = std::stod(results.at(3), &sz) - GetNode()->GetObject<MobilityModel>()->GetPosition().z;
        // repply to server
        ReplyServer();
        // atualizar central
        m_central[0] = std::stod(results.at(4), &sz); // x
        m_central[1] = std::stod(results.at(5), &sz); // y
        // verificar se já não está na posicao
        vector<double> pa, pn;
        pn.push_back(std::stod(results.at(1), &sz));
        pn.push_back(std::stod(results.at(2), &sz));
        if (CalculateDistance(m_goto, pn) != 0) { // verificar se já nao receberu um pacote com este comando de posicionamento
          NS_LOG_DEBUG("UAV recebeu novo posicionamento @" << Simulator::Now().GetSeconds());
          pa.push_back(GetNode()->GetObject<MobilityModel>()->GetPosition().x);
          pa.push_back(GetNode()->GetObject<MobilityModel>()->GetPosition().y);
          m_goto[0] = std::stod(results.at(1), &sz); // atualizando goto
          m_goto[1] = std::stod(results.at(2), &sz); // atualizando goto
          if (CalculateDistance(pa, pn) != 0) { // verificar se já nao está no posicionamento desejado
            // mudar o posicionamento do UAV
            Ptr<UavDeviceEnergyModel> dev = GetNode()->GetObject<UavDeviceEnergyModel>();
            dev->StopHover();
            GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(std::stod(results.at(1), &sz), std::stod(results.at(2), &sz), (z > 0) ? z : 0.0)); // Verficar necessidade de subir em no eixo Z
          } else {
            NS_LOG_DEBUG ("UAV " << m_id << " na posicao correta, atualizando servidor @" << Simulator::Now().GetSeconds());
            SendPacket();
          }
        } else {
          NS_LOG_DEBUG ("UAV recebeu pacote com mesmo comando de posicionamento @" << Simulator::Now().GetSeconds());
        }
      } else if (results.at(0).compare("DEPLETIONOK") == 0)
        {
          m_packetDepletion.Cancel();
          std::ostringstream mm;
          mm << "UAV\t" << m_id << "\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tDEPLETIONOK";
          //m_packetTrace(mm.str());
          NS_LOG_DEBUG(mm.str() << " @" << Simulator::Now().GetSeconds());          
        } else if (results.at(0).compare("SERVERDATA") == 0)
          {
            if (m_depletion) return; // estado de emergencia
            NS_LOG_DEBUG ("UAV " << m_id << " -- SERVERDATA @" << Simulator::Now().GetSeconds());
            SendCliData();
          }

      results.clear();
  }
}

double
UavApplication::CalculateDistance(const std::vector<double> pos1, const std::vector<double> pos2)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << pos1 << pos2);
  double dist = std::sqrt(std::pow(pos1.at(0) - pos2.at(0), 2) + std::pow(pos1.at(1) - pos2.at(1), 2));
  return dist; // euclidean
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
      //m_packetTrace(m.str());
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
    NS_LOG_DEBUG ("UavApplication::SendCliData " << msg.str()<< " @" << Simulator::Now().GetSeconds());
    if (m_sendSck->Send(packet) == packetSize)
    {
      msg.str("");
      msg << "UAV\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tDATAtoSERVER";
      //m_packetTrace(msg.str());
      NS_LOG_INFO("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - DATA to Server.");
    }

    m_sendCliDataEvent = Simulator::Schedule (Seconds(5.0), &UavApplication::SendCliData, this);
  }
}

void UavApplication::SendPacket(void)
{
  Simulator::Remove(m_sendEvent);
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  std::ostringstream msg;
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  msg << "UAV " << pos.x << " " << pos.y << " " << pos.z << " " << m_id << " " << GetNode()->GetObject<UavDeviceEnergyModel>()->GetEnergySource()->GetRemainingEnergy() << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (m_sendSck->Send(packet) == packetSize)
  {
    msg.str("");
    msg << "UAV\t" << m_id << "\tSENT\t" << Simulator::Now().GetSeconds() << "\tSERVER";
    NS_LOG_DEBUG (msg.str() << " @" << Simulator::Now().GetSeconds());
  }
  else
  {
    NS_LOG_ERROR("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " [NÃO] conseguiu enviar a mensagem para o servidor");
  }
  m_sendEvent = Simulator::Schedule(Seconds(0.5), &UavApplication::SendPacket, this);
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
  NS_LOG_INFO ("UavApplication::TotalLeasedTrace " << newV);
  m_totalLeased = newV;
}

void UavApplication::TotalEnergyConsumptionTrace (double oldV, double newV)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << oldV << newV);
  m_meanConsumption += (newV - oldV);
  m_meanConsumption /= 2.0;
}

} // namespace ns3
