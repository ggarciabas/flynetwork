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
#include "server-application.h"

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/trace-source-accessor.h"

#include <vector>
#include <fstream>
#include <string>

#define ETAPA 120

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ServerApplication");

NS_OBJECT_ENSURE_REGISTERED(ServerApplication);

TypeId
ServerApplication::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ServerApplication")
                          .SetParent<Application>()
                          .SetGroupName("Flynetwork-Application")
                          .AddConstructor<ServerApplication>()
                          .AddAttribute("DataRate", "Data rate of the communication.",
                                        DataRateValue(),
                                        MakeDataRateAccessor(&ServerApplication::m_dataRate),
                                        MakeDataRateChecker())
                          .AddAttribute("ScheduleServer", "Time to schedule server method.",
                                        DoubleValue(ETAPA),
                                        MakeDoubleAccessor(&ServerApplication::m_scheduleServer),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("Rho", "Taxa de reducao da temperatura",
                                        DoubleValue(0.99),
                                        MakeDoubleAccessor(&ServerApplication::m_rho),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("MaxX", "Valor maximo para X",
                                        DoubleValue(0),
                                        MakeDoubleAccessor(&ServerApplication::m_maxx),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("MaxY", "Valor maximo para Y",
                                        DoubleValue(0),
                                        MakeDoubleAccessor(&ServerApplication::m_maxy),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("Custo",
                                        "Custo, metrica de análise do Da de posicionamento.",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&ServerApplication::m_custo),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Environment",
                                        "The environment use to simulate de DA.",
                                        UintegerValue(2),
                                        MakeUintegerAccessor(&ServerApplication::m_environment),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("ServerPort",
                                        "Communication port number",
                                        UintegerValue(8082),
                                        MakeUintegerAccessor(&ServerApplication::m_serverPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("ClientPort",
                                        "Communication port number",
                                        UintegerValue(0),
                                        MakeUintegerAccessor(&ServerApplication::m_cliPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("PathData",
                                        "Path",
                                        StringValue(""),
                                        MakeStringAccessor(&ServerApplication::m_pathData),
                                        MakeStringChecker())
                          .AddAttribute("ScenarioName",
                                        "Name of scenario",
                                        StringValue(""),
                                        MakeStringAccessor(&ServerApplication::m_scenarioName),
                                        MakeStringChecker())
                          .AddAttribute("Ipv4Address", "The address of the node",
                                        Ipv4AddressValue(),
                                        MakeIpv4AddressAccessor(&ServerApplication::m_address),
                                        MakeIpv4AddressChecker())
                          .AddTraceSource("NewUav",
                                          "Ask for new UAVs",
                                          MakeTraceSourceAccessor(&ServerApplication::m_newUav),
                                          "ns3::UavNetwork::NewUavTrace")
                          .AddTraceSource("RemoveUav",
                                            "Ask for remove UAVs",
                                            MakeTraceSourceAccessor(&ServerApplication::m_removeUav),
                                            "ns3::UavNetwork::RemoveUavTrace")
                          .AddTraceSource("PrintUavEnergy",
                                          "Used to print data used to compare",
                                          MakeTraceSourceAccessor(&ServerApplication::m_printUavEnergy),
                                          "ns3::UavNetwork::PrintTraceUavEnergy")
                          .AddTraceSource("PacketTrace",
                                          "Packet trace",
                                          MakeTraceSourceAccessor(&ServerApplication::m_packetTrace),
                                          "ns3::UavNetwork::PacketTrace")
                          .AddTraceSource("ClientPositionTrace",
                                          "Client Position Trace",
                                          MakeTraceSourceAccessor(&ServerApplication::m_clientPosition),
                                          "ns3::UavNetwork::ClientPositionTrace");
  return tid;
}

ServerApplication::ServerApplication()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  //std::cout << "ServerApplication::ServerApplication @" << Simulator::Now().GetSeconds() << "\n";
}

ServerApplication::~ServerApplication()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  //std::cout << "ServerApplication::~ServerApplication @" << Simulator::Now().GetSeconds() << "\n";
  m_uavContainer.Clear();
  m_locationContainer.Clear();
  m_fixedClientContainer.Clear();
  m_clientContainer.Clear();
}

void ServerApplication::AddNewUav(uint32_t id, Ipv4Address addrAdhoc, double totalEnergy, double energyCost, double totalBattery, Ptr<MobilityModel> mob)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << id << addrAdhoc << totalEnergy << energyCost << totalBattery << mob);

  ////NS_LOG_DEBUG ("ServerApplication::AddNewUav Criando uav: " << id << " @" << Simulator::Now().GetSeconds());

  Ptr<Socket> socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

  ObjectFactory obj;
  obj.SetTypeId("ns3::UavModel");
  obj.Set("Id", UintegerValue(id));
  obj.Set("AddressAdhoc", Ipv4AddressValue(addrAdhoc));
  obj.Set("TotalEnergy", DoubleValue(totalEnergy));
  obj.Set("EnergyCost", DoubleValue(energyCost));
  obj.Set("TotalBattery", DoubleValue(totalBattery));
  obj.Set("Socket", PointerValue(socket));

  Ptr<UavModel> uav = obj.Create()->GetObject<UavModel>();
  uav->SetPosition(mob->GetPosition().x, mob->GetPosition().y); // todo UAV inicia na central!
  uav->SetNewPosition(mob->GetPosition().x, mob->GetPosition().y); // para nao dar erro ao validar posicionamento
  if (uav->GetSocket()->Connect(InetSocketAddress(uav->GetAddressAdhoc(), m_serverPort)))
  {
    NS_FATAL_ERROR("Servidor nao conseguiu se conectar com o UAV!");
  }
  m_uavContainer.Add(uav);
}

void ServerApplication::AddSupplyUav(uint32_t id, Ipv4Address addrAdhoc, double totalEnergy, double energyCost, double totalBattery, Ptr<MobilityModel> mob)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << id << addrAdhoc << totalEnergy << energyCost << totalBattery << mob << m_supplyPos);
  Ptr<UavModel> supplied = m_uavContainer.RemoveAt(m_supplyPos);
  ////NS_LOG_DEBUG("Criando supply uav: " << id << " last: " << supplied->GetId());

  Ptr<Socket> socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());

  ObjectFactory obj;
  obj.SetTypeId("ns3::UavModel");
  obj.Set("Id", UintegerValue(id));
  obj.Set("AddressAdhoc", Ipv4AddressValue(addrAdhoc));
  obj.Set("TotalEnergy", DoubleValue(totalEnergy));
  obj.Set("EnergyCost", DoubleValue(energyCost)); // joule/meters
  obj.Set("TotalBattery", DoubleValue(totalBattery));
  obj.Set("Socket", PointerValue(socket));

  Ptr<UavModel> uav = obj.Create()->GetObject<UavModel>();
  uav->SetPosition(mob->GetPosition().x, mob->GetPosition().y); // todo UAV inicia na central!
  uav->SetNewPosition(mob->GetPosition().x, mob->GetPosition().y); // para nao dar erro ao validar posicionamento
  if (uav->GetSocket()->Connect(InetSocketAddress(uav->GetAddressAdhoc(), m_serverPort)))
  {
    NS_FATAL_ERROR("Servidor nao conseguiu se conectar com o UAV!");
  }
  m_uavContainer.Add(uav);
  uav = 0;

  // supply confs
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  vector<double> p;
  p.push_back(pos.x);
  p.push_back(pos.y);
  supplied->SetNewPosition(p);
  supplied->NotConfirmed();
  supplied->CancelSendCentralEvent();
  supplied->SetSendCentralEvent(Simulator::ScheduleNow(&ServerApplication::SendCentralPacket, this, supplied));

  m_uavGoToCentral.Add(supplied);
  supplied = 0;
}

void ServerApplication::AddNewFixedClient(string login, double x, double y)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << login << x << y);
  ObjectFactory obj;
  obj.SetTypeId("ns3::ClientModel");
  obj.Set("Login", StringValue(login));
  Ptr<ClientModel> cli;
  cli = obj.Create()->GetObject<ClientModel>();
  cli->SetPosition(x, y);
  m_fixedClientContainer.Add(cli); // considerando o mesmo container dos clientes, posi este nao é 'limpo' durante a execução do algoritmo
}

void
ServerApplication::TracedCallbackRxApp (Ptr<const Packet> packet, const Address & address)
{
  // NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << packet << address);
  uint8_t *buffer = new uint8_t[packet->GetSize ()];
  packet->CopyData(buffer, packet->GetSize ());
  std::string s = std::string(buffer, buffer+packet->GetSize());
  std::istringstream iss(s);
  std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                 std::istream_iterator<std::string>());
  std::string::size_type sz; // alias of size_t

  InetSocketAddress add = InetSocketAddress::ConvertFrom(address);

  if (results.at(0).compare("UAV") == 0) {
    std::vector<double> pos;
    pos.push_back(std::stod (results.at(1),&sz));
    pos.push_back(std::stod (results.at(2),&sz));

    std::ostringstream mm;
    mm << "SERVER\t-1\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tUAV";
    m_packetTrace(mm.str());
    ////NS_LOG_DEBUG("MSG UAV: " << s);
    pos.push_back(std::stod(results.at(3), &sz)); // UAV tem posição 3D
    if (CalculateDistanceCentral(pos)<0.05) { // caso o UAV esteja na central, desligar!
      Ptr<UavModel> uav = m_uavGoToCentral.FindUavModel(std::stoi(results.at(4), &sz));
      if (uav != NULL)
      {
        ////NS_LOG_DEBUG("SERVER received UAV pos na central @"<< Simulator::Now().GetSeconds() <<"  --- Removendo nó [" << uav->GetId() << "]");
        m_removeUav(uav->GetId()); // desligando todas as informacoes do nó
        uav->Dispose(); // parando eventos do UavModel
        m_uavGoToCentral.RemoveUav(uav);
        uav = 0;
      }
      else
      {
        ////NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV que foi pra central [UAV] ID " << results.at(4));
      }
    } else {
      Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(4), &sz));
      if (uav != NULL)
      {
        uav->SetPosition(pos.at(0), pos.at(1));
        uav->ConfirmPosition(); // confirma o posicionamento do UAV
        uav->SetTotalEnergy(std::stod(results.at(5), &sz)); // atualiza energia total do UAV
        ////NS_LOG_DEBUG ("SERVER - atualizando UAV " << uav->GetId());
        if (uav->IsConfirmed()) {
          ////NS_LOG_DEBUG("SERVER - UAV #" << uav->GetId() << " confirmado no posicionamento");
          ReplyUav(uav);
        }
        uav = 0;
      }
      else
      {
        ////NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [UAV] --- fora da rede?! ID " << results.at(4));
      }
    }
    pos.clear();
   } else if (results.at(0).compare("UAVRECEIVED") == 0)  {
            Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
            if (uav != NULL)
            {
              uav->CancelSendPositionEvent(); // recebida confirmacao do UAV
              ////NS_LOG_DEBUG("SERVER - UAVRECEIVED ::: UAV #" << uav->GetId() << " @" << Simulator::Now().GetSeconds());
            } else
            {
              ////NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [UAVRECEIVED] --- fora da rede?! ID " << results.at(1));
            }
      } else if (results.at(0).compare("CENTRALOK") == 0)  {
               Ptr<UavModel> uav = m_uavGoToCentral.FindUavModel(std::stoi(results.at(1), &sz));
               if (uav != NULL)
               {
                 uav->CancelSendCentralEvent(); // recebida confirmacao do UAV
                 uav = 0;
                 ////NS_LOG_DEBUG("SERVER - CENTRALOK ::: UAV going to central @" << Simulator::Now().GetSeconds());
               } else
               {
                 ////NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [CENTRALOK] ID " << results.at(1));
               }
        }  else if (results.at(0).compare("DATA") == 0) {
          NS_LOG_INFO("ServerApplication::TracedCallbackRxApp " << s << " @" << Simulator::Now().GetSeconds());
          Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
          if (uav != NULL)
          {
            uav->CancelAskCliDataEvent(); // recebida confirmacao do UAV
            uav->SetClientDataConfirmed(true); // recebeu confirmacao!
            uav->SetTotalEnergy(std::stod(results.at(2), &sz)); // atualiza energia total do UAV
            ////NS_LOG_DEBUG("SERVER - DATA confirmation ::: UAV #" << uav->GetId() << " @" << Simulator::Now().GetSeconds());
            int i = 3; // 3 informacoes antes das informacoes dos clientes! contemplam cada cliente!
            for (; i < int(results.size()-1); i+=4) { // id time_update posx posy
              Ptr<ClientModel> cli = m_clientContainer.FindClientModel(results.at(i)); // id
              if (cli != NULL) { // caso já exista, atualiza somente posicao se o tempo de atualizacao for maior!
                if (std::stod(results.at(i+1), &sz) > cli->GetUpdatePos().GetSeconds()) { // time - pega ultima posicao atualizada
                  std::vector<double> pos;
                  pos.push_back(std::stod (results.at(i+2),&sz)); // x
                  pos.push_back(std::stod (results.at(i+3),&sz)); // y
                  cli->SetPosition(pos.at(0), pos.at(1));
                  pos.clear();
                }
              } else { // senao cria um novo
                ObjectFactory obj;
                obj.SetTypeId("ns3::ClientModel");
                obj.Set("Login", StringValue(results.at(i))); // id
                cli = obj.Create()->GetObject<ClientModel>();
                cli->SetUpdatePos(Seconds(std::stod(results.at(i+1), &sz))); // time
                std::vector<double> pos;
                pos.push_back(std::stod (results.at(i+2),&sz)); // x
                pos.push_back(std::stod (results.at(i+3),&sz)); // y
                cli->SetPosition(pos.at(0), pos.at(1));
                m_clientContainer.Add(cli);
                pos.clear();
              }
              ////NS_LOG_DEBUG ("ServerApplication::TracedCallbackRxApp \n" << cli->ToString());
            }
            // repply to UAV
            ReplyAskCliData (uav);
          } else
          {
            ////NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [DATA] --- fora da rede?! ID " << results.at(1));
          }
        } else if (results.at(0).compare("DEPLETION") == 0)  {
            Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
            if (uav != NULL) {
              Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
              vector<double> p;
              p.push_back(pos.x);
              p.push_back(pos.y);
              uav->SetNewPosition(p); // pegar posicao do servidor, ele é a central!
              uav->CancelSendPositionEvent();
              uav->CancelSendCentralEvent();
              uav->SetSendCentralEvent(Simulator::ScheduleNow(&ServerApplication::SendCentralPacket, this, uav));
              p.clear();
              uav = 0;
              // solicitar novo UAV para a rede!
              m_newUav(1, 2); // solicita novo UAV
              // atualizar o posicionamento do Uav na última posicao do vetor, mandando ele para a localização do UAV que esta saindo
              uav = m_uavContainer.GetLast();
              p.push_back(std::stod (results.at(2),&sz));
              p.push_back(std::stod (results.at(3),&sz));
              p.push_back(std::stod(results.at(4), &sz)); // UAV tem posição 3D
              uav->SetNewPosition(p); // pegar posicao do servidor, ele é a central!
              uav->CancelSendPositionEvent();
              uav->NotConfirmed();
              uav->SetSendPositionEvent(Simulator::ScheduleNow(&ServerApplication::SendUavPacket, this, uav));
            } else {
              ////NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [DEPLETION] --- fora da rede?! ID " << results.at(1));
            }
            uav = 0;
          } else {
            std::ostringstream mm;
            mm << "SERVER\t-1\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tCLIENT";
            m_packetTrace(mm.str());
            NS_LOG_INFO("SERVER -- CLIENT ::: recebida informacoes de aplicacao do cliente no endereco " << add.GetIpv4());
            std::ostringstream os;
            os << "./scratch/flynetwork/data/output/" << m_pathData << "/client_data.txt";
            std::ofstream file;
            file.open(os.str(), std::ofstream::out | std::ofstream::app);
            file << Simulator::Now().GetSeconds() << " RECEBIDO" << std::endl; // RECEBIDO pelo servidor
            file.close();
          }
    results.clear();
}

void ServerApplication::ReplyAskCliData(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << uav->GetId());
  std::ostringstream msg;
  msg << "DATAOK "<< '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    msg.str("");
    msg << "SERVER\t-1\tSENT\t" << Simulator::Now().GetSeconds() << "\tDATAOK";
    m_packetTrace(msg.str());
    ////NS_LOG_DEBUG (msg.str());
  }
}

void ServerApplication::ReplyUav(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << uav->GetId());
  std::ostringstream msg;
  msg << "SERVEROK "<< '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    msg.str("");
    msg << "SERVER\t-1\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAVOK";
    m_packetTrace(msg.str());
  }
}

void ServerApplication::StartApplication(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_rmax = std::sqrt(std::pow(m_maxx,2)+std::pow(m_maxy,2));
  m_newUav(1,0); // inicia com um UAV
  ValidateUavPosition();
}

void ServerApplication::SendAskClientPacket(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION (this << uav->GetId());
  uav->CancelAskCliDataEvent();
  std::ostringstream msg;
  msg << "SERVERDATA " << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    NS_LOG_INFO ("SERVER - ask client data " << uav->GetId() << " @" << Simulator::Now().GetSeconds());
  }
  uav->SetAskCliDataEvent(Simulator::Schedule(Seconds(0.3), &ServerApplication::SendAskClientPacket, this, uav));
}

void ServerApplication::AskClientData ()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_serverEvent);
  double t = 0.0;
  for (UavModelContainer::Iterator i = m_uavContainer.Begin(); i != m_uavContainer.End(); ++i) {
    (*i)->CancelAskCliDataEvent();
    (*i)->SetClientDataConfirmed(false);
    (*i)->SetAskCliDataEvent(Simulator::Schedule(Seconds(t), &ServerApplication::SendAskClientPacket, this, (*i)));
    t += 0.5;
  }
  ValidateUavPosition();
}

void ServerApplication::StopApplication(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_serverEvent);
}

void ServerApplication::ValidateUavPosition()
{
  //NS_LOG_DEBUG("ServerApplication::ValidateUavPosition @" << Simulator::Now().GetSeconds());
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  // Verificar se todos os UAVs estão no posicionamento desejado!
  UavModelContainer::Iterator i;
  bool flag = true;
  double t=0.0;
  for (i = m_uavContainer.Begin(); i != m_uavContainer.End(); ++i) {
    bool f_uav = ((*i)->IsConfirmed() && (*i)->ClientDataConfirmed());
    flag = (flag && f_uav); // espera receber informacoes de dados do cliente tbm!

    if (!(*i)->ClientDataConfirmed()) {
      (*i)->CancelAskCliDataEvent();
      (*i)->SetClientDataConfirmed(false);
      (*i)->SetAskCliDataEvent(Simulator::Schedule(Seconds(t), &ServerApplication::SendAskClientPacket, this, (*i)));
      t += 0.5;
    }

    if (!(*i)->IsConfirmed()) {
      (*i)->CancelSendPositionEvent();
      SendUavPacket((*i));
    }

  }
  if (flag) {
    ////NS_LOG_DEBUG ("SERVER - todos os UAVs estao na posicao desejada @" << Simulator::Now().GetSeconds());
    Run();
  } else {
    ////NS_LOG_DEBUG ("Server - [" << Simulator::Now().GetSeconds() << "] $$ [NÃO] estao prontos @" << Simulator::Now().GetSeconds());
    Simulator::Schedule(Seconds(5.00), &ServerApplication::ValidateUavPosition, this);
  }
}

void ServerApplication::Run ()
{
  //NS_LOG_DEBUG("ServerApplication::Run @" << Simulator::Now().GetSeconds());
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  if (!m_clientContainer.IsEmpty() || !m_fixedClientContainer.IsEmpty())
  {
    ////NS_LOG_DEBUG("SERVER - Iniciando execução dos DAs @" << Simulator::Now().GetSeconds());
    std::ostringstream ss;
    ss << "mkdir -p ./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij";
    system(ss.str().c_str());
    ss.str("");
    // runDAPython();
    // runDAPuro();
    runDA();
    NS_LOG_INFO ("ServerApplication::Run liberando client container ");
    m_clientContainer.Clear();
    runAgendamento();
    NS_LOG_INFO ("ServerApplication::Run liberando location container ");
    m_locationContainer.Clear();
  }
  else
  {
    NS_FATAL_ERROR("SERVER - $$$$ [NÃO] existem clientes identificados no servidor, ignorando execução dos DAs -- Se isso ocorre existe erro no cadastro de clientes fixos (que seriam os palcos)!");
  }
  m_serverEvent = Simulator::Schedule(Seconds(m_scheduleServer), &ServerApplication::AskClientData, this);
}

void ServerApplication::SendCentralPacket(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << uav->GetId());
  ////NS_LOG_DEBUG ("ServerApplication::SendCentralPacket @" << Simulator::Now().GetSeconds() << " Id: " << uav->GetId());
  uav->CancelSendCentralEvent();
  NS_ASSERT(uav != 0);
  std::vector<double> pos = uav->GetNewPosition();
  std::ostringstream msg;
  msg << "GOTOCENTRAL " << pos.at(0) << " " << pos.at(1) << " 10.0" << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    NS_LOG_INFO ("SERVER - UAV [" << Simulator::Now().GetSeconds() << "] ::: enviando informacoes da CENTRAL para o UAV #" << uav->GetId());
    msg.str("");
    msg << "SERVER\t-1\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAV";
    m_packetTrace(msg.str());
  } else {
    NS_FATAL_ERROR("SERVER - $$$$ [NÃO] conseguiu enviar informacoes da CENTRAL para o UAV.");
    uav->SetSendCentralEvent(Simulator::Schedule(Seconds(0.01), &ServerApplication::SendCentralPacket, this, uav));
    pos.clear();
    return;
  }
  uav->SetSendCentralEvent(Simulator::Schedule(Seconds(5.0), &ServerApplication::SendCentralPacket, this, uav));
  NS_LOG_INFO (" ------------ SendCentralPacket end @" << Simulator::Now().GetSeconds());
  pos.clear();
}

void ServerApplication::SendUavPacket(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << uav->GetId());
  ////NS_LOG_DEBUG("ServerApplication::SendUavPacket UAV Id "  << uav->GetId() << " @" << Simulator::Now().GetSeconds() << " REF " << uav->GetReferenceCount());
  uav->CancelSendPositionEvent();
  NS_ASSERT(uav != 0);
  std::vector<double> pos = uav->GetNewPosition();
  std::ostringstream msg;
  msg << "GOTO " << pos.at(0) << " " << pos.at(1) << " 10.0" << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    NS_LOG_INFO ("ServerApplication::SendUavPacket SERVER - UAV [" << Simulator::Now().GetSeconds() << "] ::: enviando informacoes de posicionamento para o UAV #" << uav->GetId());
    msg.str("");
    msg << "SERVER\t-1\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAV";
    m_packetTrace(msg.str());
  } else {
    NS_FATAL_ERROR("ServerApplication::SendUavPacket SERVER - $$$$ [NÃO] conseguiu enviar informacoes de posicionamento para o UAV.");
  }
  uav->SetSendPositionEvent(Simulator::Schedule(Seconds(5.0), &ServerApplication::SendUavPacket, this, uav));
  NS_LOG_INFO ("ServerApplication::SendUavPacket  ------------ SendUavPacket end @" << Simulator::Now().GetSeconds());
  pos.clear();
}

void ServerApplication::runDAPython()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );

  std::ofstream cenario, file;
  std::ostringstream os;
  os.str("");
  os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/cenario_in.txt";
  cenario.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);

  if (cenario.is_open())
  {
    cenario << m_environment << "\n";
    cenario << m_locationContainer.GetN() << std::endl;
    Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
    cenario << pos.x << "," << pos.y;

    os.str("");
    os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/client.txt";
    file.open(os.str().c_str(), std::ofstream::out);
    bool first = true;
    for (ClientModelContainer::Iterator i = m_clientContainer.Begin(); i != m_clientContainer.End(); ++i)
    {
      cenario << std::endl
              << (*i)->GetLogin() << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1);
      if (first) {
        file << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1) << "," << (*i)->GetLogin();
        first = false;
      } else {
        file << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1) << "," << (*i)->GetLogin();
      }
    }

    for (ClientModelContainer::Iterator i = m_fixedClientContainer.Begin(); i != m_fixedClientContainer.End(); ++i)
    {
      cenario << std::endl
              << (*i)->GetLogin() << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1);
      if (first) {
        file << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1) << "," << (*i)->GetLogin();
        first = false;
      } else {
        file << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1) << "," << (*i)->GetLogin();;
      }
    }

    file << std::endl;
    file.close ();
    cenario.close ();
    m_clientPosition (os.str ());
  }

  m_totalCliGeral = 0;

  os.str ("");
  os << "python ./scratch/flynetwork/da_python " << m_pathData << " " << int(Simulator::Now().GetSeconds()) << " > ./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/python_log.txt";
  ////NS_LOG_DEBUG (os.str());
  int status = system(os.str().c_str());
  if (status < 0)
  {
    NS_FATAL_ERROR("SERVER - Erro na execução do algoritmo em Python!");
  }
  else
  {
    std::ifstream cenario_in;
    os.str("");
    os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/cenario_out.txt";
    cenario_in.open(os.str().c_str(), std::ofstream::in);
    if (cenario_in.is_open())
    {
      double x, y;
      std::string line;
      ObjectFactory obj;
      obj.SetTypeId("ns3::LocationModel");
      uint32_t id = 0;
      m_locConsTotal = 0.0;
      os.str("");
      os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/location_client.txt";
      std::ofstream location_cli;
      location_cli.open(os.str().c_str(), std::ofstream::out);
      while (getline(cenario_in, line))
      {
        sscanf(line.c_str(), "%lf,%lf\n", &x, &y); // new location
        obj.Set("Id", UintegerValue(id++));
        Ptr<LocationModel> loc = obj.Create()->GetObject<LocationModel>();
        loc->SetPosition(x, y);
        // ler clientes conectados a loc
        getline(cenario_in, line);
        std::istringstream iss(line);
        std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                       std::istream_iterator<std::string>());
        double total =0.0;
        int t_cli = 0;
        for (int i = 0; i < int(results.size()); ++i) {
          Ptr<ClientModel> cli = NULL;
          if (results.at(i).at(0) == 'f') {
            cli = m_fixedClientContainer.FindClientModel(results.at(i)); // id
          } else {
            cli = m_clientContainer.FindClientModel(results.at(i)); // id
          }
          if (cli != NULL) { // caso já exista, atualiza somente posicao se o tempo de atualizacao for maior!
            total += cli->GetConsumption();
          } else {
            NS_FATAL_ERROR ("ServerApplication::runDAPython cliente nao encontrado " << results.at(i));
          }
          t_cli++;
        }
        loc->SetTotalConsumption (total);
        loc->SetTotalCli(t_cli);
        m_totalCliGeral += t_cli;
        m_locationContainer.Add(loc);
        NS_LOG_INFO (loc->toString());
        m_locConsTotal += total; // atualiza total de consumo de todas as localizacoes

        location_cli << loc->GetId() << "," << loc->GetTotalCli() << "," << loc->GetTotalConsumption() << std::endl;
      }
      if ( m_locConsTotal == 0) {
        m_locConsTotal = 1.0; // para nao dar problemas no calculo
      }
      location_cli.close();
      cenario_in.close();
    }
    else
    {
      NS_FATAL_ERROR("SERVER - $$$$ [NÃO] foi possível abrir para ler informacoes de localização");
    }
  }
}

void ServerApplication::CreateCentralLocation(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  ObjectFactory fac;
  fac.SetTypeId("ns3::LocationModel");
  fac.Set("Id", UintegerValue(m_locationContainer.GetN() + 1));
  Ptr<LocationModel> loc = fac.Create()->GetObject<LocationModel>();
  loc->SetPosition(pos.x, pos.y);
  m_locationContainer.Add(loc);

  std::ostringstream os;
  os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/location_client.txt";
  std::ofstream location_cli;
  location_cli.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  location_cli << loc->GetId() << "," << loc->GetTotalCli() << "," << loc->GetTotalConsumption() << std::endl;
  location_cli.close();
}

double
ServerApplication::CalculateDistanceCentral(const std::vector<double> pos) {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << pos);
  Vector v = GetNode()->GetObject<MobilityModel>()->GetPosition();
  double dist = std::sqrt(std::pow(pos.at(0) - v.x, 2) + std::pow(pos.at(1) - v.y, 2));
  NS_LOG_INFO("SERVER -- DIST " << dist);
  return dist; // euclidean, sempre considera a distância atual calculada pelo DA
}

double
ServerApplication::CalculateDistance(const std::vector<double> pos1, const std::vector<double> pos2)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds());
  double dist = std::sqrt(std::pow(pos1.at(0) - pos2.at(0), 2) + std::pow(pos1.at(1) - pos2.at(1), 2));
  return dist; // euclidean, sempre considera a distância atual calculada pelo DA
}

// bool ServerApplication::ValidateMijConvergency(vector<vector<long double>> vec, vector<vector<long double>> m_ij, unsigned siz)
// {
//   // NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << vec << m_ij << siz);
//   for (unsigned i = 0; i < siz; ++i)
//   {
//     for (unsigned j = 0; j < siz; ++j)
//     {
//       if (vec[i][j] != m_ij[i][j])
//         return false;
//     }
//   }

//   for (unsigned i = 0; i < siz; ++i)
//   {
//     vec[i].clear();
//     m_ij[i].clear();
//   }
//   m_ij.clear();
//   vec.clear();

//   return true;
// }

void ServerApplication::runAgendamento(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_ASSERT(m_locationContainer.GetN() != 0);

  //  - fazer com que o número de localizacoes e uavs sejam iguais!
  //    -- se mais loc, add novos UAVs, senão criar localizações na central!
  int diff = m_locationContainer.GetN() - m_uavContainer.GetN();
  if (diff > 0)
  { // add new uav
    NS_LOG_DEBUG("SERVER - Solicitando novos UAVs @" << Simulator::Now().GetSeconds());
    // chamar callback para criar novos UAVs
    m_newUav(diff, 0); // chamando callback
  }
  else if (diff < 0)
  { // add new loc
    NS_LOG_DEBUG("SERVER - Solicitando novas localizações @" << Simulator::Now().GetSeconds());
    for (int i = diff; i < 0; ++i)
    {
      CreateCentralLocation();
    }
  }

  for (LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
       l_j != m_locationContainer.End(); ++l_j)
  {
    NS_LOG_INFO("-- " << (*l_j)->toString());
  }

  //  - calcular o CUSTO ENERGETICO de atribuição do uav para cada localizacao, criando uma matriz Bij
  NS_LOG_DEBUG("SERVER - Iniciando estrutura do DA para agendamento @" << Simulator::Now().GetSeconds());

  vector<vector<long double>> b_ij; // i - UAVs, j - localizacoes
  vector<vector<long double>> custo_x; // i - UAVs, j - localizacoes x=1,2ou3
  // Uav id
  vector<int> uav_ids;
  // Loc id
  vector<int> loc_ids;
  int count = 0;
  double custo;
  vector<double> central_pos;
  Vector vp = GetNode()->GetObject<MobilityModel>()->GetPosition();
  central_pos.push_back(vp.x);
  central_pos.push_back(vp.y);

  int verify_uav; // utilizado para verificar se um UAV não possui carga suficiente para se direcionar a alguma das localizações, isto acontece quando um UAV possui para todas as localizações o valor 1.!
  bool l_id = true;
  NS_LOG_INFO ("Calculando custo ---------------------------- @" << Simulator::Now().GetSeconds());
  for (UavModelContainer::Iterator u_i = m_uavContainer.Begin();
       u_i != m_uavContainer.End(); ++u_i, ++count)
  {
    b_ij.push_back(vector<long double>());
    custo_x.push_back(vector<long double>());
    recalcule: // utilizado para permitir recalcular quando o UAV for suprido!
    uav_ids.push_back((*u_i)->GetId());
    NS_LOG_INFO ("UAV :: " << (*u_i)->toString());
    verify_uav = 0;
    for (LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
         l_j != m_locationContainer.End(); ++l_j)
    {
      NS_LOG_INFO ("LOC :: " << (*l_j)->toString());
      if (l_id) loc_ids.push_back((*l_j)->GetId());
      custo = CalculateCusto((*u_i), (*l_j), central_pos);
      custo_x[count].push_back(custo);
      b_ij[count].push_back(1.0-custo); //std::exp(custo)); // NOVO
      verify_uav += custo; // somando o valor dos custos, assim se ao final tiver o mesmo valor que o total de localizações, quer dizer que este UAV somente tem carga para voltar a central
    }
    l_id = false;

    if (verify_uav == int(m_locationContainer.GetN())) {
      NS_LOG_DEBUG("ServerApplication::runAgendamento --> Enviar UAV " << (*u_i)->GetId() << " para a central  REF " << (*u_i)->GetReferenceCount());
      // criar um novo nó iniciando na região central, como sempre!
      m_supplyPos = count; // posicao que será suprida
      m_newUav(1, 1); // true, pois está o solicitado para suprir uma posicao
      uav_ids.pop_back(); // remove o id anteior, caso tenha sido trocado por um novo UAV
      ////NS_LOG_DEBUG ("ServerApplication::runAgendamento recalculando, novo UAV entra na rede para suprir um UAv que nao tem bateria para qualquer das localizações!");
      custo_x[count].clear();
      b_ij[count].clear();
      goto recalcule;
    }
    NS_LOG_INFO (" ------------------------- ");
  }

  NS_LOG_DEBUG ("RunAgendamento: ");
  for (int k = 0; k < int(uav_ids.size()); ++k)
  {
    NS_LOG_DEBUG(" [" << uav_ids[k] << "," << loc_ids[k] << "]");
  }

  NS_LOG_DEBUG ("FIM custo ---------------------------- @" << Simulator::Now().GetSeconds());

  PrintCusto (custo_x, int(Simulator::Now().GetSeconds()), true, uav_ids, loc_ids);

  // Inicializando
  NS_LOG_DEBUG ("SERVER - inicializando matrizes.");
  double temp = 0.60;
  // Mai
  vector<vector<long double>> m_ij;
  // variavel da tranformacao algebrica (parte do self-amplification)
  vector<vector<long double>> o_ij;
  // variavel da transformacao algebrica (parte o Y_sch)
  vector<vector<long double>> lamb_ij;
  // q_ai
  vector<vector<long double>> q_ij;
  double N = m_uavContainer.GetN();
  unsigned siz = m_uavContainer.GetN();
  // Ptr<UniformRandomVariable> e_ai = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
  // e_ai->SetAttribute ("Min", DoubleValue (min));

  for (unsigned i = 0; i < siz; ++i)
  {
    m_ij.push_back(vector<long double>());
    o_ij.push_back(vector<long double>());
    lamb_ij.push_back(vector<long double>());
    q_ij.push_back(vector<long double>());
    for (unsigned j = 0; j < (unsigned)m_locationContainer.GetN(); ++j)
    {
      // e_ai->SetAttribute ("Max", DoubleValue (max));
      // double rdom = e_ai->GetValue();
      m_ij[i].push_back(1/N); // + rdom);
      o_ij[i].push_back(m_ij[i][j]);
      lamb_ij[i].push_back(0.0);
      q_ij[i].push_back(0.0);
    }
  }

  PrintBij(b_ij, int(Simulator::Now().GetSeconds()), true, uav_ids, loc_ids);

  int print = 0;
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/mij_" << std::setfill ('0') << std::setw (7) << print << ".txt";
  PrintMatrix (m_ij, os.str());
  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/o_ij_" << std::setfill ('0') << std::setw (7) << print << ".txt";
  PrintMatrix (o_ij, os.str());
  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/lamb_ij_" << std::setfill ('0') << std::setw (7) << print << ".txt";
  PrintMatrix (lamb_ij, os.str());
  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/q_ij_" << std::setfill ('0') << std::setw (7) << print++ << ".txt";
  PrintMatrix (q_ij, os.str());

  unsigned itB_max = 1000;
  unsigned itA_max = 1000;
  unsigned itC_max = 500;
  double gamma = 0.95;
  NS_LOG_INFO ("SERVER - iniciando execucao das partes A, B e C @" << Simulator::Now().GetSeconds());

  // Part A
  unsigned itA = 0;
  long double new_mij;
  bool converge_B = true, converge_C = true; // para saber se houve mudanca no valor
  while (temp >= 0.005 && itA < itA_max)
  {
    // Part B
    unsigned itB = 0;
    do
    {
      converge_B = true;

      for (unsigned i = 0; i < siz; ++i)
      {
        for (unsigned j = 0; j < siz; ++j)
        {
          // calculate \lamb_{ij}
          // NS_LOG_DEBUG ("MIJ [" << i << "," << j << "] : " << m_ij[i][j]);
          lamb_ij[i][j] = m_ij[i][j] * b_ij[i][j];
          if (isnan(lamb_ij[i][j])) {
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (q_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (o_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (lamb_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_m_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (m_ij, os.str());
            NS_FATAL_ERROR ("Lab is nan: ["<<i<<","<<j<<"] " << m_ij[i][j] << " * " << b_ij[i][j] << " = " << lamb_ij[i][j] << "   " << itB);
          }
          // calculate Q_{ij}
          q_ij[i][j] = gamma * o_ij[i][j] - lamb_ij[i][j] * b_ij[i][j];
          if (isnan(q_ij[i][j])) {
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (q_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (o_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (lamb_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_m_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (m_ij, os.str());
            NS_FATAL_ERROR ("qij is nan:  " << itB);
          }
          // calculate m_{ij}
          new_mij = exp(q_ij[i][j] / temp);
          if (m_ij[i][j] != new_mij) { // mudou!
            converge_B = false;
            converge_C = false;
          }
          m_ij[i][j] = new_mij;
          if (isnan(m_ij[i][j])) {
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (q_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (o_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (lamb_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/B_m_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
            PrintMatrix (m_ij, os.str());
            NS_FATAL_ERROR ("mij is nan  " << itB);
          }
        }
      }

      // Part C
      unsigned itC = 0;
      os.str("");
      do
      {
        converge_C = true;

        // normalizando as linhas
        unsigned check = 0;
        // os.str("");
        // os << "Linhas --------- " << itA << " " << itB << " " << itC << "\n";
        for (unsigned i = 0; i < siz; ++i)
        {
          // os << "[";
          long double total_linha = 0.0;
          for (unsigned k = 0; k < siz; ++k)
          {
            if (m_ij[i][k] == 1.0) {
              check++;
            }
            total_linha += m_ij[i][k];
          }

          if (total_linha == 0) {
            NS_FATAL_ERROR ("Zerou linha " << i);
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (q_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (o_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (lamb_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_m_ij_" << std::setfill ('0') << std::setw (4) << itB << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (m_ij, os.str());
            NS_FATAL_ERROR ("mij is nan   " << itC);
          }

          // os << total_linha << " --- ";
          for (unsigned j = 0; j < siz; ++j)
          {
            // os << " [" << m_ij[i][j] << ",";
            new_mij = m_ij[i][j] / total_linha;
            if (m_ij[i][j] != new_mij) {
              converge_B = converge_C = false;
            }
            m_ij[i][j] = new_mij;
            // os << m_ij[i][j] << "]\t\t";
            if (isnan(m_ij[i][j])) {
              // NS_LOG_DEBUG (os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (q_ij, os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (o_ij, os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (lamb_ij, os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_m_ij_" << std::setfill ('0') << std::setw (4) << itB << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (m_ij, os.str());
              NS_FATAL_ERROR ("mij is nan   " << itC);
            }
          }
          // os << "\n";
        }
        // os << "]\n";
        // NS_LOG_DEBUG (os.str());

        if (check == siz) {
          NS_LOG_DEBUG ("GOING OUT!! ------------------------>>>>>>");
          goto out;
        }

        // normalizando colunas
        // os.str("");
        // os << "Colunas --------- " << itA << " " << itB << " " << itC << "\n";
        for (unsigned j = 0; j < siz; ++j)
        {
          os << "[";
          long double total_coluna = 0.0;
          for (unsigned k = 0; k < siz; ++k)
          {
            total_coluna += m_ij[k][j];
          }

          if (total_coluna == 0) {
            NS_LOG_DEBUG ("Zerou coluna " << j);
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (q_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (o_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (lamb_ij, os.str());
            os.str("");
            os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_m_ij_" << std::setfill ('0') << std::setw (4) << itB << "_" << itB << "_" << itC << ".txt";
            PrintMatrix (m_ij, os.str());
            NS_FATAL_ERROR ("mij is nan   " << itC);
          }

          // os << total_coluna << " --- ";
          for (unsigned i = 0; i < siz; ++i)
          {
            // os << " [" << m_ij[i][j] << ",";
            new_mij = m_ij[i][j] / total_coluna;
            if (m_ij[i][j] != new_mij) {
              converge_B = converge_C = false;
            }
            m_ij[i][j] = new_mij;
            // os << m_ij[i][j] << "]\t\t";
            if (isnan(m_ij[i][j])) {
              // NS_LOG_DEBUG (os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (q_ij, os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (o_ij, os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (lamb_ij, os.str());
              os.str("");
              os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_m_ij_" << std::setfill ('0') << std::setw (4) << itB << "_" << itB << "_" << itC << ".txt";
              PrintMatrix (m_ij, os.str());
              NS_FATAL_ERROR ("mij is nan   " << itC);
            }
          }
          // os << "\n";
        }
        // os << "]\n";
        // NS_LOG_DEBUG (os.str());

        itC++;
      } while (!converge_C && itC < itC_max);

      // os.str("");
      // os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_q_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
      // PrintMij (q_ij, 1.0, os.str(), uav_ids, loc_ids);
      // os.str("");
      // os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_o_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
      // PrintMij (o_ij, 1.0, os.str(), uav_ids, loc_ids);
      // os.str("");
      // os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_lamb_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
      // PrintMij (lamb_ij, 1.0, os.str(), uav_ids, loc_ids);
      // os.str("");
      // os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/C_m_ij_" << std::setfill ('0') << std::setw (4) << itA << "_" << itB << ".txt";
      // PrintMij (m_ij, 1.0, os.str(), uav_ids, loc_ids);

      itB++;
    } while (!converge_B && itB < itB_max);

    os.str("");
    os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/mij/mij_" << std::setfill ('0') << std::setw (7) << print++ << ".txt";
    PrintMij (m_ij, 1.0, os.str(), uav_ids, loc_ids);

    temp *= m_rho;
    o_ij = m_ij;
    itA++;
  }
  // permite sair dos lacos ao encontrar 1 para cada localizacao
  out:

  NS_LOG_DEBUG("SERVER - Finalizada estrutura do DA para agendamento @" << Simulator::Now().GetSeconds());

  NS_LOG_DEBUG("SERVER - Atualizando posicionamento dos UAVs @" << Simulator::Now().GetSeconds());
  int id, i = 0;
  double t = 0.0;
  std::ofstream file;
  std::ostringstream osbij, osloc, osuav;
  os.str("");
  vector<vector<long double> > f_mij;
  long double val;
  for (UavModelContainer::Iterator u_i = m_uavContainer.Begin();
       u_i != m_uavContainer.End(); ++u_i, ++i)
  {
    f_mij.push_back(vector<long double>());

    recalcule_2:
    NS_LOG_DEBUG("SERVER - UAV "<< (*u_i)->GetId() << " REF " << (*u_i)->GetReferenceCount());
    id = -1;
    val = -1;
    for (unsigned j = 0; j < siz; ++j)
    {
      NS_LOG_DEBUG ("\\t--- j = " << j);
      if (m_ij[i][j] > val && !(m_locationContainer.Get(j)->IsUsed()))
      { // somente aceita a localizacao que nao tenha sido usada por outro UAV
        NS_LOG_DEBUG("\tMaior valor LOC \t" << m_locationContainer.Get(j)->toString());
        id = j;
        val = m_ij[i][j];
      }
      f_mij[i].push_back(0.0);
    } // ao final selecionara a localizacao com maior valor de mij

    if (custo_x[i][id] == 1.0) {
      // Uav não tem bateria suficiente para ir até esta localização!
      NS_LOG_DEBUG("ServerApplication::runAgendamento --> Enviar UAV " << (*u_i)->GetId() << " para a central  REF " << (*u_i)->GetReferenceCount());
      // criar um novo nó iniciando na região central, como sempre!
      m_supplyPos = count; // posicao que será suprida
      m_newUav(1, 1); // true, pois está o solicitando para suprir uma posicao
      NS_LOG_DEBUG ("ServerApplication::runAgendamento recalculando, novo UAV entra na rede para suprir um UAv que nao tem bateria para qualquer das localizações!");
      f_mij[i].clear();
      custo = CalculateCusto((*u_i), m_locationContainer.Get(id), central_pos);
      custo_x[i][id] = custo;
      b_ij[i][id] = custo;
      NS_LOG_DEBUG ("Go to recalcule");
      goto recalcule_2;
    }

    f_mij[i][id] = 1.0;

    NS_LOG_DEBUG ("## ===> UAV " << (*u_i)->GetId() << " to LOC " << id);
    m_locationContainer.Get(id)->SetUsed(); // define como utilizada pelo UAV atual

    NS_LOG_DEBUG("SERVER - LOC "<< id << "\n\t" << m_locationContainer.Get(id)->toString());

    // Salvando onde o UAV estava e para onde ele será enviado.
    NS_LOG_DEBUG("SERVER - Salvando onde o UAV estava e para onde ele será enviado");
    std::vector<double> vp = (*u_i)->GetPosition();
    if (i == 0)
      osuav << (*u_i)->GetId() << "," << vp.at(0) << "," << vp.at(1); // posicao do UAV
    else
      osuav << "," << (*u_i)->GetId() << "," << vp.at(0) << "," << vp.at(1); // posicao do UAV
    vp = m_locationContainer.Get(id)->GetPosition();
    if (i == 0)
      osloc << id << "," << vp.at(0) << "," << vp.at(1); // posicao da LOC
    else
      osloc << "," << id << "," << vp.at(0) << "," << vp.at(1); // posicao da LOC
    if (i == 0)
      osbij << b_ij[i][id]; // b_ij, custo de deslocamento
    else
      osbij << "," << b_ij[i][id]; // b_ij, custo de deslocamento


    NS_LOG_DEBUG("SERVER - definindo novo posicionamento @" << Simulator::Now().GetSeconds());


    (*u_i)->SetNewPosition(m_locationContainer.Get(id)->GetPosition());
    (*u_i)->NotConfirmed(); // atualiza o valor para identificar se o UAV chegou a posicao correta
    (*u_i)->CancelSendPositionEvent();
    if (CalculateDistanceCentral(m_locationContainer.Get(id)->GetPosition())==0.0) {
      (*u_i)->SetSendCentralEvent(Simulator::Schedule(Seconds(t), &ServerApplication::SendCentralPacket, this, (*u_i)));
      m_uavGoToCentral.Add((*u_i));
      m_uavContainer.RemoveUav((*u_i));
      --u_i;
    } else {
      (*u_i)->SetSendPositionEvent(Simulator::Schedule(Seconds(t), &ServerApplication::SendUavPacket, this, (*u_i)));
    }
    t += 0.005;
    vp.clear();
  }

  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/" << int(Simulator::Now().GetSeconds()) << "/f_mij.txt";
  PrintMij (f_mij, temp, os.str(), uav_ids, loc_ids);

  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/uav_loc.txt";
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  Vector serv_pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  file << m_maxx << "," << m_maxy << std::endl << serv_pos.x << "," << serv_pos.y << std::endl << osuav.str() << std::endl << osloc.str() << std::endl << osbij.str() << std::endl;
  file.close();

  PrintCusto (custo_x, int(Simulator::Now().GetSeconds()), false, uav_ids, loc_ids); // pode ter sido modificado no laco anterior, um UAv pode ter sido suprido
  PrintBij(b_ij, int(Simulator::Now().GetSeconds()), false, uav_ids, loc_ids);
  m_printUavEnergy(int(Simulator::Now().GetSeconds())); // esperando a solucao final, UAVs podem ser trocados

  NS_LOG_DEBUG ("-- Finalizado posicionamento dos UAVs @" << Simulator::Now().GetSeconds());

  // liberando memoria
  for (unsigned i = 0; i < siz; ++i)
    {
      m_ij[i].clear();
      o_ij[i].clear();
      lamb_ij[i].clear();
      q_ij[i].clear();
      b_ij[i].clear();
      custo_x[i].clear();
      f_mij[i].clear();
    }
  m_ij.clear();
  o_ij.clear();
  lamb_ij.clear();
  q_ij.clear();
  b_ij.clear();
  central_pos.clear();
  custo_x.clear();
  f_mij.clear();
}

long double
ServerApplication::CalculateCusto (Ptr<UavModel> uav, Ptr<LocationModel> loc, vector<double> central_pos)
{
  NS_LOG_DEBUG ("ServerApplication::CalculateCusto > uavId: " << uav->GetId() << " locId: " << loc->GetId());
  long double custo = 1.0;
  long double b_ui_atu = uav->GetTotalEnergy(); // bateria atual
  long double ce_ui_la_lj = uav->CalculateEnergyCost(CalculateDistance(uav->GetPosition(), loc->GetPosition())); // custo energetico
  long double ce_ui_lj_lc = uav->CalculateEnergyCost(CalculateDistance(loc->GetPosition(), central_pos));
  long double b_ui_tot = uav->GetTotalBattery();
  long double b_ui_res = b_ui_atu*0.98 - ce_ui_la_lj - ce_ui_lj_lc; // bateria residual
  long double ce_te_lj = loc->GetTotalConsumption() * m_scheduleServer;
  long double P_te = b_ui_res/ce_te_lj;

  if (b_ui_res > 0) {
    // sobre os custo ver: https://github.com/ggarciabas/Calculo-de-Posicionamento
    switch (m_custo) {
      case 1:
        custo = (ce_ui_la_lj + ce_ui_lj_lc) / b_ui_tot; // media do custo
        break;
      // case 2:
      //   custo = 1 - P_te;
      //   if (custo < 0.0) {
      //     custo = 0.0; // aleatorio ou sequencia! Nao interssa.
      //   }
      //   break;
      case 2:
        custo = 1-P_te;
        if (custo < 0.0) {
          custo = (ce_ui_la_lj + ce_ui_lj_lc) / b_ui_tot;
        }
        break;
      case 3:
        custo = 1 - P_te;
        if (custo < 0.0) {
          custo = 0.0;
        }
        custo += (ce_ui_la_lj + ce_ui_lj_lc) / b_ui_tot;
        custo /= 2.0;
        break;
    }
  }
  ////NS_LOG_DEBUG ("ServerApplication::CalculateCusto > mcusto: " << m_custo << " custo: " << custo);
  central_pos.clear();
  return custo;
}

void
ServerApplication::PrintBij (vector<vector<long double>> b_ij, int print, bool before, vector<int> uav_ids, vector<int> loc_ids)
{
  std::ostringstream os;
  if (before) {
    os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/bij.txt";
  } else {
    os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/bij_final.txt";
  }
  std::ofstream file;
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  vector<int>::iterator u_i = uav_ids.begin();
  file << (*u_i);
  u_i++;
  for (; u_i != uav_ids.end(); ++u_i)
  {
    file << "," << (*u_i);
  }
  file << std::endl;
  vector<int>::iterator l_j = loc_ids.begin();
  file << (*l_j);
  l_j++;
  for (; l_j != loc_ids.end(); ++l_j)
  {
    file << "," << (*l_j);
  }
  file << std::endl;
  for (vector<vector<long double>>::iterator i = b_ij.begin(); i != b_ij.end(); ++i)
  {
    vector<long double>::iterator j = (*i).begin();
    file << (*j);
    j++;
    for (; j != (*i).end(); ++j)
    {
      file << "," << (*j);
    }
    file << std::endl;
  }

  file.close();
  for (vector<vector<long double>>::iterator i = b_ij.begin(); i != b_ij.end(); ++i)
  {
    (*i).clear();
  }
  b_ij.clear();
}

void
ServerApplication::PrintCusto (vector<vector<long double>> custo, int print, bool before, vector<int> uav_ids, vector<int> loc_ids)
{
  std::ostringstream os;
  if (before) {
    os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/custo_" << m_custo << ".txt";
  } else  {
    os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/custo_" << m_custo << "_final.txt";
  }
  std::ofstream file;
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  vector<int>::iterator u_i = uav_ids.begin();
  file << (*u_i);
  u_i++;
  for (; u_i != uav_ids.end(); ++u_i)
  {
    file << "," << (*u_i);
  }
  file << std::endl;
  vector<int>::iterator l_j = loc_ids.begin();
  file << (*l_j);
  l_j++;
  for (; l_j != loc_ids.end(); ++l_j)
  {
    file << "," << (*l_j);
  }
  file << std::endl;
  for (vector<vector<long double>>::iterator i = custo.begin(); i != custo.end(); ++i) {
    vector<long double>::iterator j = (*i).begin();
    if (j != (*i).end()) {
      file << (*j);
      j++;
      for (; j != (*i).end(); ++j)
      {
        file << "," << (*j);
      }
      file << std::endl;
    }
  }

  file.close();

  for (vector<vector<long double>>::iterator i = custo.begin(); i != custo.end(); ++i)
  {
    (*i).clear();
  }
  custo.clear();
}

void
ServerApplication::PrintMatrix (vector<vector<long double>> m, std::string nameFile)
{
  std::ofstream file;
  file.open(nameFile.c_str(), std::ofstream::out | std::ofstream::app);
  for (vector<vector<long double>>::iterator i = m.begin(); i != m.end(); ++i)
  {
    vector<long double>::iterator j = (*i).begin();
    file << (*j);
    j++;
    for (; j != (*i).end(); ++j)
    {
      file << "," << (*j);
    }
    file << std::endl;
  }
  file.close();
}

void
ServerApplication::PrintMij (vector<vector<long double>> m_ij, double temp, std::string nameFile, vector<int> uav_ids, vector<int> loc_ids)
{
  std::ofstream file;
  file.open(nameFile.c_str(), std::ofstream::out | std::ofstream::app);
  file << temp << std::endl;
  vector<int>::iterator u_i = uav_ids.begin();
  file << (*u_i);
  u_i++;
  for (; u_i != uav_ids.end(); ++u_i)
  {
    file << "," << (*u_i);
  }
  file << std::endl;
  vector<int>::iterator l_j = loc_ids.begin();
  file << (*l_j);
  l_j++;
  for (; l_j != loc_ids.end(); ++l_j)
  {
    file << "," << (*l_j);
  }
  file << std::endl;
  for (vector<vector<long double>>::iterator i = m_ij.begin(); i != m_ij.end(); ++i)
  {
    vector<long double>::iterator j = (*i).begin();
    file << (*j);
    j++;
    for (; j != (*i).end(); ++j)
    {
      file << "," << (*j);
    }
    file << std::endl;
  }
  file.close();
  for (vector<vector<long double>>::iterator i = m_ij.begin(); i != m_ij.end(); ++i)
  {
    (*i).clear();
  }
  m_ij.clear();
}

void
ServerApplication::PhyRxOkTraceAdhoc (std::string context, Ptr<const Packet> packet, double snr, WifiMode mode, enum WifiPreamble preamble)
{
  NS_LOG_INFO("SERVER - PHYRXOK mode=" << mode << " snr=" << snr << " " << *packet);
}

void
ServerApplication::PhyRxErrorTraceAdhoc (std::string context, Ptr<const Packet> packet, double snr)
{
  NS_LOG_INFO("SERVER - PHYRXERROR snr=" << snr << " " << *packet);
}

void
ServerApplication::PhyTxTraceAdhoc (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower)
{
  NS_LOG_INFO("SERVER - PHYTX mode=" << mode << " " << *packet);
}

void ServerApplication::DoDispose() {
  NS_LOG_INFO ("ServerApplication::DoDispose REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());

  Simulator::Remove(m_serverEvent);
  m_uavContainer.Clear();
  m_locationContainer.Clear();
  m_fixedClientContainer.Clear();
  m_clientContainer.Clear();
}

// https://github.com/ggarciabas/nsnam_ns3/blob/17c1f9200727381852528ac4798f040128ac842a/scratch/flynetwork/da_cpp/deterministic-annealing.cc
void ServerApplication::runDA() {
  //NS_LOG_DEBUG("ServerApplication::runDA @" << Simulator::Now().GetSeconds());

  std::ofstream file;
  std::ostringstream os;
  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/client.txt";
  file.open(os.str().c_str(), std::ofstream::out);
  bool first = true;
  double tMov = m_clientContainer.GetN();
  double tFix = m_fixedClientContainer.GetN();
  double pFix = 2; // peso dos clientes fixos - clientes móveis sempre com peso de 1"
  for (ClientModelContainer::Iterator i = m_clientContainer.Begin(); i != m_clientContainer.End(); ++i)
  {
    (*i)->EraseLocation();
    (*i)->SetPci(1/(tMov+tFix*pFix));
    if (first) {
      file << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1)  << "," << (*i)->GetLogin();;
      first = false;
    } else {
      file << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1)  << "," << (*i)->GetLogin();;
    }
  }
  for (ClientModelContainer::Iterator i = m_fixedClientContainer.Begin(); i != m_fixedClientContainer.End(); ++i)
  {
    (*i)->EraseLocation();
    (*i)->SetPci(pFix/(tMov+tFix*pFix));
    if (first) {
      file << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1)  << "," << (*i)->GetLogin();;
      first = false;
    } else {
      file << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1)  << "," << (*i)->GetLogin();;
    }
  }
  file << std::endl;
  file.close ();
  m_clientPosition (os.str ()); // Adicionando informacoes reais do ambiente

  m_clientDaContainer.Clear();
  m_clientDaContainer.Add(m_fixedClientContainer); // clientes fixos antes pra dar prioridade na capacidade do UAV!
  m_clientDaContainer.Add(m_clientContainer);

  // constantes
  double t_min = 1e-7;
  double r_max = std::sqrt(std::pow(m_maxx, 2) + std::pow(m_maxy, 2));
  // 1550 series https://www.cisco.com/c/en/us/products/collateral/wireless/aironet-1550-series/data_sheet_c78-641373.html
  // 1570 series https://www.cisco.com/c/en/us/products/wireless/aironet-1570-series/datasheet-listing.html
  double uav_cob = 280.5; // metros verificar distancia_sinr.py
  double ptCli = 28; // dBm - potencia de transmissao máxima para o Ap Aironet 1550 series 802.11n 2.4GHz
  double fsInterf = 0.0008; // fator de sobreposicao de espaco 5 (50%)
  double dRCli = 6.5; // Mbps - taxa considerada por usuário
  double raio_cob = 115.47; // metros - para clientes utilizando equação de antena direcional com esparramento verificar Klaine2018
  double sinrCliMin = -93; // dBm - tabela de Receive sensitivity para 2.4GHz 802.11n (HT20) MCS 0
  double lambda = 3e8/2.4e9; // metros
  double b = 3.7;
  double pi = 3.141516; // pi
  double maxDrUav = 1024; // Mbps -- verificar alguma Ref!!
  double gain = 4; // dBi - tanto o ganho de recepcao como o de transmissao
  double N_W = 10e-9*2e7; // dB - N0 = 10e-9 W/Hz -- B = 20MHz - Livro Goldsmith ref para N0
  // Fuck explanation dB and log relation: https://www.physicsforums.com/threads/confusion-with-db-equation-10-or-20.641850/#post-4105917
  double pl_ref = 20*std::log10(4*pi/lambda); // dB - Friis Model
  // --> https://www.isa.org/standards-publications/isa-publications/intech-magazine/2002/november/db-vs-dbm/
  // Use dB when expressing the ratio between two power values. Use dBm when expressing an absolute value of power.
  double pr_ref = ptCli + gain + gain - pl_ref; // dBm - potencia do sinal na distancia de referencia
  double t = 0.6;
  int locId = 0;
  int max_iterB = 5000;

  os.str("");
  os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/da_log.txt";
  file.open(os.str().c_str(), std::ofstream::out);
  // //NS_LOG_DEBUG ("\n\t t_min =" << t_min << "\n \t r_max =" << r_max << "\n \t ptUav ="<< ptUav << "\n \t ptCli =" << ptCli << "\n \t fsInterf ="
  //             << fsInterf  << "\n \t dRCli =" << dRCli << "\n \t sinrCliMin =" << sinrCliMin<< "\n \t fcCli ="
  //             << fcCli  << "\n \t comp_onda =" << comp_onda << "\n \t pi =" << pi << "\n \t maxDrUav =" << maxDrUav << "\n \t gain =" << gain
  //               << "\n \t N_W =" << N_W << "\n \t t =" << t <<  "\n \t locId =" << locId << "\n \t max_iterB =" << max_iterB << "\n\tplRefCli_dB: " << plRefCli_dB << "uav_cob: " << uav_cob);

  ObjectFactory lObj;
  lObj.SetTypeId("ns3::LocationModel");

  Ptr<LocationModel> lCentral = lObj.Create()->GetObject<LocationModel> ();
  lCentral->SetId(9999);
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  lCentral->SetPosition(pos.x, pos.y); // iniciando a localizacao que representará a central
  lCentral->IniciarMovimentoA(); // somente para nao dar problemas ao executar toString
  lCentral->IniciarMovimentoB();
  Ptr<LocationModel> loc = lObj.Create()->GetObject<LocationModel>();
  loc->SetId(locId++);
  CentroDeMassa(loc, lCentral, r_max);
  loc->IniciarMovimentoA(); // salvando posicionamento para comparacao de movimento no laco A
  loc->IniciarMovimentoB();
  loc->SetPunishNeighboor(0.2); // ALTERADO: valor inicial de punicao!
  loc->InitializeWij (m_clientDaContainer.GetN()*dRCli); // considera que todos os clientes estao conectados ao primeiro UAv, isto para nao ter que calcular a distancia na primeira vez, esta validacao será feita a partir da primeira iteracao do laco A
  loc->SetFather(lCentral, CalculateDistance(lCentral->GetPosition(r_max), loc->GetPosition(r_max)), r_max, uav_cob);
  m_locationContainer.Add(loc);
  double percentCli = 1.0;
  int iter = 0;
  do {// laco A
    iter++;
    file << "------------------------------> ItA: " << iter  << " temp: " << t << "\n";
    int tMovCon = 0;
    int tFixCon = 0;
    bool locConnected = true;
    bool capacidade = true;
    bool movimentoB = true;
    int iterB = 0;
    do { // laco B
      capacidade = locConnected = true;
      iterB++;
      tMovCon = 0;
      tFixCon = 0;

      for (ClientModelContainer::Iterator ci = m_clientDaContainer.Begin(); ci != m_clientDaContainer.End(); ++ci) {
        (*ci)->SetConnected(false); // limpando para não dar conflito!
        double Zci = 0.0;
        double low_dchilj = 1.5; // maior distancia é 1.0, valores normalizados!
        for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
          double dcilj = CalculateDistance((*ci)->GetPosition(r_max), (*lj)->GetPosition(r_max));
          double pljci = std::exp ( - ((dcilj + (*lj)->GetWij()/maxDrUav + (((*ci)->IsConnected()) ? 1 : 0))/t) ); // NOVO - Verifica se o cliente possui conexao, caso nao tenha calcula normalmente, senao adiciona 1 para que a probabilidade deste em relacao ao UAv seja insignificante.
          Zci += pljci;
          (*lj)->SetTempPljci(pljci);
          if (low_dchilj > dcilj) { // achou UAV mais proximo
            low_dchilj = dcilj;
            // https://bitbucket.org/cpgeimestrado/rascunhocpgei/src/master/conversor.cpp
            double pl_dB = 10*std::log10(dcilj)*b; // dB - modelo simplificado goldsmith
            long double pr_W = dBmToWatts(pr_ref - pl_dB); // W
            long double it_W = fsInterf*pr_W; // w
            long double sinr_W = pr_W / (it_W + N_W); // W - modelo de goldsmith considera para escalar!!!
            long double sinr_dBm = WattsToDbm(sinr_W); // dBm
            if (low_dchilj <= raio_cob/r_max && sinr_dBm >= sinrCliMin) { // esta dentro da area de cobertura maxima da antena e recebe SINR min
              // NS_LOG_DEBUG ("-> CLI " << (*ci)->GetLogin() <<  " com " << (*lj)->GetId() << "\t Distancia: " << dcilj*r_max << "\t SINR: " << sinr_dBm << "dBm");
              Ptr<LocationModel> lCon = (*ci)->GetLocConnected();
              (*ci)->SetConnected(true);
              (*ci)->SetDataRate(sinr_dBm);
              if (lCon) { // caso tenha alguma informacao anterior, desconsidera nos calculos, para isto atualiza o loc
                lCon->toString();
                if (lCon->GetId() == (*lj)->GetId()) {
                  lCon->UpdateDistCli (dcilj);
                  lCon = 0;
                  continue; // nao faz alteracoes! Desnecessario!
                }
                lCon->RemoveClient(dRCli, (*ci)->GetConsumption());
              }
              lCon = 0;
              (*ci)->SetLocConnected((*lj));
              // calcular a SNR e caso seja maior que o mínimo, considerar cliente conectado
              (*lj)->NewClient(dRCli, (*ci)->GetConsumption(), dcilj);
              (*ci)->SetDataRate(sinr_dBm);
              file << "tFix: " << tFixCon << "\ttMovCon: " << tMovCon << std::endl;
            }
          }
        }
        for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
          if (Zci < 1e-90) {
            file << "--> Zci esta baixo! @" << Simulator::Now().GetSeconds() << "\n";
            // t *= 1.2; // aumenta 120%
            t = 0.1;
            percentCli *= 0.7; // reduz 70%
            break;
          }
          (*lj)->AddPljCi((*ci), Zci, r_max); // finaliza o calculo do pljci na funcao e cadastra no map relacionando o ci
        }
        if ((*ci)->IsConnected()) {
          if ((*ci)->GetLogin().at(0) == 'f') {
            tFixCon++;
          } else {
            tMovCon++;
          }
        }
      }

      // calcular lj novos - não consigo fazer no laco anterior pela falta dos valores acumulados (não tentar colcoar la!)
      for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
        (*lj)->UpdatePosition (m_maxx, m_maxy);
        // Avalia a utilizacao de capacidade das localizações
        capacidade = capacidade && (*lj)->ValidarCapacidade(maxDrUav);
        (*lj)->ClearChildList();
      }

      // Se houve mudança no posicionamento, se faz necessario verificar um novo pai para cada lj
      movimentoB = MovimentoB();
      if (movimentoB) {
        for (int j = m_locationContainer.GetN()-1; j >= 0; j--) { // Obs.: >=0 para que seja feito o calculo da localizacao 0 com a central, não irá entrar no segundo laco devido a condicao imposta lá!
          locConnected = FindFather (j, r_max, uav_cob, lCentral, locConnected);
        }
      }
      else {
        for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
          locConnected = (*lj)->IsConnected() && locConnected;
        }
      }

      file << "Itb: " << iterB << "\n\tTemp: " << t << "\n\ttMovCon: " << tMovCon << "\n\ttFixCon: " << tFixCon << "\n\tLocConnected: " << ((locConnected) ? "true" : "false") << "\n\tCapacidade: " << ((capacidade) ? "true":"false") << "\n";

    } while (movimentoB && iterB < max_iterB);

    if (locConnected) {
      if (capacidade) {
        if (tFixCon == tFix) {
          if (tMovCon >= tMov*percentCli) {
            file << "--> Finalizado - temp=" << t << std::endl;
            // t *= 0.5; // resfria bastante
            GraficoCenarioDa(t, iter, lCentral, uav_cob, r_max, raio_cob, maxDrUav);
            break;
          } else {
            file << "--> " << iter << " @"<< Simulator::Now().GetSeconds() << " clientes móveis nao conectados [" << percentCli << "] !\n";
          }
        } else {
          file << "--> " << iter << " @"<< Simulator::Now().GetSeconds() << " clientes fixos nao conectados! t[" << t << "]\n";
        }
      } else {
        file << "--> " << iter << " @"<< Simulator::Now().GetSeconds() << " capacidade superior!\n";
      }
    } else {
      file << "--> " << iter << " @"<< Simulator::Now().GetSeconds() << " localizações não conecatadas!\n";
    }

    if (!MovimentoA()) { // } || (tFixCon != tFix && t == 0.1)) {
      file << "--> Solicitando nova localizacao por não existir movimento em A @" << Simulator::Now().GetSeconds() << std::endl;
      // new_uav:
      Ptr<LocationModel> nLoc = lObj.Create()->GetObject<LocationModel> ();
      nLoc->SetId(locId++);
      CentroDeMassa(nLoc, lCentral, r_max);
      nLoc->IniciarMovimentoA(); // salvando posicionamento para comparacao de movimento no laco A
      nLoc->IniciarMovimentoB();
      m_locationContainer.Add(nLoc);
      nLoc->SetPunishNeighboor(0.2);
      nLoc->InitializeWij (0.0); // ninguem esta conectado a nova localizacao
      FindFather (m_locationContainer.GetN()-1, r_max, uav_cob, lCentral, true);
      t *= 2.0;
      GraficoCenarioDa(t, iter, lCentral, uav_cob, r_max, raio_cob, maxDrUav);
      for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
        (*lj)->IniciarMovimentoA();
        (*lj)->LimparHistorico();
        (*lj)->UpdatePunishNeighboor(uav_cob/r_max);
      }
      continue;
    }

    GraficoCenarioDa(t, iter, lCentral, uav_cob, r_max, raio_cob, maxDrUav);
    // Reiniciar Movimento A para cada Localizacao
    for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
      (*lj)->IniciarMovimentoA();
      (*lj)->LimparHistorico();
      (*lj)->UpdatePunishNeighboor(uav_cob/r_max);
    }

    t = t*0.9; // reduz 90%  a tempreatura

  } while (t > t_min); // laco da temperatura

  file << std::endl;
  file.close ();

  m_totalCliGeral = 0;
  m_locConsTotal = 0; // atualiza total de consumo de todas as localizacoes

  // os.str("");
  // os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/location_data_rate.txt";
  // file.open(os.str().c_str(), std::ofstream::out);
  // for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
  //   file << (*lj)->GetId() << "," << (*lj)->GetDataRate() << std::endl;
  // }
  // file.close();

  os.str("");
  os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/client_data_rate.txt";
  file.open(os.str().c_str(), std::ofstream::out);
  for (ClientModelContainer::Iterator ci = m_clientContainer.Begin(); ci != m_clientContainer.End(); ++ci) {
    file << (*ci)->GetLogin() << "," << (*ci)->GetDataRate() << std::endl;
  }
  file.close();

  if ( m_locConsTotal == 0) {
    m_locConsTotal = 1.0; // para nao dar problemas no calculo
  }
}

bool ServerApplication::FindFather (int pos, double r_max, double uav_cob, Ptr<LocationModel> lCentral, bool locConnected)
{
  std::vector<double> p1 (m_locationContainer.Get(pos)->GetPosition(r_max));
  int id = -1; // a principio se conecta com a central
  double dist = CalculateDistance(lCentral->GetPosition(r_max), p1);
  for (int k = pos - 1; k >= 0; --k) {
    std::vector<double> p2 (m_locationContainer.Get(k)->GetPosition(r_max));
    double d = CalculateDistance(p1, p2);
    if (d <= dist) { // achou algum nó mais perto
      id = k;
      dist = d;
    }
  }
  if (id == -1) { // menor distancia é para com a central
    m_locationContainer.Get(pos)->SetFather(lCentral, dist, r_max, uav_cob);
    locConnected = m_locationContainer.Get(pos)->IsConnected() && locConnected;
  } else { // menor distancia é para algum outro UAV, cadastrar o pai e o filho!
    m_locationContainer.Get(pos)->SetFather(m_locationContainer.Get(id), dist, r_max, uav_cob);
    locConnected = m_locationContainer.Get(pos)->IsConnected() && locConnected;
    m_locationContainer.Get(id)->AddChild(m_locationContainer.Get(pos), r_max); // novo filho para id!
  }
  return locConnected;
}

void ServerApplication::GraficoCenarioDa (double temp, int iter, Ptr<LocationModel> lCentral, double uav_cob, double r_max, double max_antena, double maxDrUav) {
  std::ofstream file;
  std::ostringstream os;
  os.str("");
  os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/da_loc_cpp_" << std::setfill ('0') << std::setw (15) << iter << ".txt";
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  file << iter << std::endl;
  file << uav_cob << std::endl;
  file << max_antena << std::endl;

  LocationModelContainer::Iterator lj = m_locationContainer.Begin();
  lj = m_locationContainer.Begin(); // imprimindo distancia maxima com clientes
  file << (*lj)->GetMaxDistClient()*r_max;
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetMaxDistClient()*r_max;
  }
  file << "\n";

  file << m_maxx << "," << m_maxy << std::endl;

  file << temp << std::endl;

  file << lCentral->GetXPosition() << "," << lCentral->GetYPosition() << std::endl;
  lj = m_locationContainer.Begin(); // imprimindo a posicao atual da localizacao
  file << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  }
  file << "\n";

  lj = m_locationContainer.Begin(); // imprimindo a posicao antiga
  file << (*lj)->GetXPositionA() << "," << (*lj)->GetYPositionA();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetXPositionA() << "," << (*lj)->GetYPositionA();
  }
  file << "\n";

  lj = m_locationContainer.Begin(); // imprimindo a posicao do pai
  file << (*lj)->GetFather()->GetXPosition() << "," << (*lj)->GetFather()->GetYPosition();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetFather()->GetXPosition() << "," << (*lj)->GetFather()->GetYPosition();
  }
  file << "\n";

  lj = m_locationContainer.Begin(); // imprimindo neigh
  file << (*lj)->GetPunishNeighboor();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetPunishNeighboor();
  }
  file << "\n";

  lj = m_locationContainer.Begin(); // imprimindo neigh
  file << (*lj)->IsConnected();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->IsConnected();
  }
  file << "\n";

  lj = m_locationContainer.Begin(); // imprimindo Wij
  file << (*lj)->GetWij()/maxDrUav;
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetWij()/maxDrUav;
  }
  file << "\n";
  // data rate client
  ClientModelContainer::Iterator ci = m_clientDaContainer.Begin();
  file << (*ci)->GetDataRate();
  for (; ci != m_clientDaContainer.End(); ++ci) {
    file << "," << (*ci)->GetDataRate();
  }

  file.close();

  // os.str ("");
  // // custo, etapa, main_path, teste, iter
  // os << "python ./scratch/flynetwork/data/da_loc.py custo_" << m_custo << " " << int(Simulator::Now().GetSeconds()) << " ./scratch/flynetwork/data/output/" << m_scenarioName << "/ False " << iter;
  // //NS_LOG_DEBUG (os.str());
  // system(os.str().c_str());
  // os.str ("");
  // os << "convert -delay 20 -loop 0 ./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/*.png" << " ./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/da_loc.gif";
  // ////NS_LOG_DEBUG (os.str());
  // system(os.str().c_str());
}

bool ServerApplication::MovimentoA() {
  // verificar se as localizações tiveram o MovimentoA, este movimento é pra validar dentro do laço A
  LocationModelContainer::Iterator lj = m_locationContainer.Begin();
  bool retorno = (*lj)->MovimentoA();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    retorno = retorno || (*lj)->MovimentoA();
  }
  return retorno;
}

bool ServerApplication::MovimentoB() {
  // verificar se as localizações tiveram o MovimentoB, este movimento é pra validar dentro do laço B
  LocationModelContainer::Iterator lj = m_locationContainer.Begin();
  bool retorno = (*lj)->MovimentoB(); (*lj)->IniciarMovimentoB(); // atualizar para o novo posicionamento senão fica em laco infinito!
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    retorno = retorno || (*lj)->MovimentoB();
    (*lj)->IniciarMovimentoB(); // atualizar para o novo posicionamento senão fica em laco infinito!
  }
  return retorno;
}

void ServerApplication::CentroDeMassa (Ptr<LocationModel> l, Ptr<LocationModel> central, double r_max) {
  double x=0, y=0;
  bool con = true;
  double ccon = 0;
  for (ClientModelContainer::Iterator i = m_clientDaContainer.Begin(); i != m_clientDaContainer.End(); ++i) {
    con = con && (*i)->IsConnected();
    if (!(*i)->IsConnected()) { // NOVO: considera somente os que nao estao conectados!
      x += (*i)->GetXPosition();
      y += (*i)->GetYPosition();
      ccon = ccon + 1;
    }
  }

  if (con) { // todos os cliente estao conectados
    std::cout << "==> Todos os clientes conectados.\n";
    l->SetPosition(central->GetXPosition(), central->GetYPosition()); // posicionar no centro dos clientes
  } else {
    std::cout << "Centro de massa: " << x/ccon << " " << y/ccon << std::endl;
    l->SetPosition(x/ccon, y/ccon); // posicionar no centro dos clientes
  }
}

void ServerApplication::runDAPuro() {
  //NS_LOG_DEBUG("ServerApplication::runDAPuro @" << Simulator::Now().GetSeconds());
  std::ofstream file;
  std::ostringstream os;
  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_pathData<<"/etapa/"<<int(Simulator::Now().GetSeconds())<<"/client.txt";
  file.open(os.str().c_str(), std::ofstream::out);
  bool first = true;
  for (ClientModelContainer::Iterator i = m_clientContainer.Begin(); i != m_clientContainer.End(); ++i)
  {
    (*i)->SetPci(1/(double)(m_clientContainer.GetN()+m_fixedClientContainer.GetN()));
    if (first) {
      file << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1);
      first = false;
    } else {
      file << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1);
    }
  }
  for (ClientModelContainer::Iterator i = m_fixedClientContainer.Begin(); i != m_fixedClientContainer.End(); ++i)
  {
    (*i)->SetPci(1/(double)(m_clientContainer.GetN()+m_fixedClientContainer.GetN()));
    if (first) {
      file << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1);
      first = false;
    } else {
      file << "," << (*i)->GetPosition().at(0) << "," << (*i)->GetPosition().at(1);
    }
  }
  file << std::endl;
  file.close ();

  m_clientDaContainer.Clear();
  m_clientDaContainer.Add(m_clientContainer);
  m_clientDaContainer.Add(m_fixedClientContainer);

  // constantes
  double t_min = 1e-7;
  double r_max = std::sqrt(std::pow(m_maxx, 2) + std::pow(m_maxy, 2));
  double raio_cob = 0.0;
  switch (m_environment) { // ver valores no arquivo main.py
    case 1:
      raio_cob = 19.0581; break;
    case 2:
      raio_cob = 79.6821; break;
    case 3:
      raio_cob = 101.609; break;
    case 4:
      raio_cob = 108.295; break;
  }
  double t = 0.9;
  int locId = 0;
  int max_iterB = 1000;

  // -------------------
  //std::cout << "[\n\tTmin:\t\t" << t_min
            // << "\n\tRMax:\t\t" << r_max
            // << "\n\tRaioCob:\t\t" << raio_cob
            // << "\n\tEnv:\t\t" << m_environment
            // << "\n\tMaxIterB:\t\t" << max_iterB
            // << "\n\tTini:\t\t" << t << "\n]";
  // //std::cout << "Esperando ....";
  // std::cin >> t;
  // -------------------

  ObjectFactory lObj;
  lObj.SetTypeId("ns3::LocationModel");

  Ptr<LocationModel> lCentral = lObj.Create()->GetObject<LocationModel> ();
  lCentral->SetId(9999);
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  lCentral->SetPosition(pos.x, pos.y); // iniciando a localizacao que representará a central
  lCentral->IniciarMovimentoB();

  Ptr<LocationModel> loc = lObj.Create()->GetObject<LocationModel>();
  loc->SetId(locId++);
  CentroDeMassa(loc, lCentral, r_max);
  loc->IniciarMovimentoB();
  m_locationContainer.Add(loc);



  int iter = 0;
  do {// laco A
    iter++;

    int tMovCon = 0;
    int iterB = 0;
    bool movimentoB = false;
    do { // laco B
      iterB++;
      tMovCon = 0;
      //std::cout << "Iteracao: " << iterB << "\n";
      double totalZci = 0.0;
      for (ClientModelContainer::Iterator ci = m_clientDaContainer.Begin(); ci != m_clientDaContainer.End(); ++ci) {
        double Zci = 0.0;
        double low_dchilj = r_max+10;
        for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
          double dcilj = CalculateDistance((*ci)->GetPosition(), (*lj)->GetPosition());
          double pljci = std::exp ( - (dcilj/t) );
          Zci += pljci;
          (*lj)->SetTempPljci(pljci);
          if (low_dchilj > dcilj) { // achou loc mais proximo
            low_dchilj = dcilj;
          }
        }
        if (low_dchilj <= raio_cob) {
          (*ci)->SetConnected(true);
          tMovCon++;
        } else {
          (*ci)->SetConnected(false);
        }
        double totalPljci = 0.0;
        for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
          if (Zci < 1e-30) {
            Zci = 1e-30;
            t *= 1.1;
          }
          totalPljci +=  (*lj)->AddPljCiPuro((*ci), Zci, r_max);
        }
        totalZci += Zci;
      }
      movimentoB = MovimentoB();
      for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
        (*lj)->UpdatePosition (m_maxx, m_maxy);
      }
    } while (movimentoB && iterB <= max_iterB);

    // eliminar duplicados
    double dist;
    for (int j = m_locationContainer.GetN()-1; j > 0; j--) {
      std::vector<double> p1 (m_locationContainer.Get(j)->GetPosition());
      for (int k = j - 1; k >= 0; --k) {
        dist = CalculateDistance((m_locationContainer.Get(k)->GetPosition()), p1);
        if (dist == 0) {
          m_locationContainer.Erase(j);
          break;
        }
      }
    }

    // ------------------ Para teste somente
    //std::cout << "LacoB (fim) Temp: " << t << " IteracaoB: " << iterB << "\n[\n";
    // for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
      //std::cout << (*lj)->toString();
    // }
    //std::cout << "]\n";
    // -----------------

    if (tMovCon >= m_clientDaContainer.GetN()*0.8) {
      break; // finalizar Da
    }

    //std::cout << "---> Novo UAV\n";
    Ptr<LocationModel> nLoc = lObj.Create()->GetObject<LocationModel> ();
    nLoc->SetId(locId++);
    CentroDeMassa(nLoc, lCentral, r_max);
    nLoc->IniciarMovimentoB();
    m_locationContainer.Add(nLoc);

    GraficoCenarioDaPuro(t, iter, lCentral, raio_cob);

    t = t*0.9; // reduz 90%  a tempreatura

    //std::cout << "------------------------------------------------------------------\n";
    //std::cout << "------------------------------------------------------------------\n";
  } while (t > t_min); // laco da temperatura

  GraficoCenarioDaPuro(t, iter, lCentral, raio_cob);

  //std::cout << "Finalizou...\n";
  // std::cin >> lixo;

  m_totalCliGeral = 0;
  m_locConsTotal = 0; // atualiza total de consumo de todas as localizacoes

  os.str("");
  os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/location_client.txt";
  std::ofstream location_cli;
  location_cli.open(os.str().c_str(), std::ofstream::out);
  for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
    location_cli << (*lj)->GetId() << "," << (*lj)->GetTotalCli() << "," << (*lj)->GetTotalConsumption() << std::endl;
  }
  location_cli.close();

  if ( m_locConsTotal == 0) {
    m_locConsTotal = 1.0; // para nao dar problemas no calculo
  }
}

void ServerApplication::GraficoCenarioDaPuro (double temp, int iter, Ptr<LocationModel> lCentral, double raio_cob) {
  std::ofstream file;
  std::ostringstream os;
  os.str("");
  os <<"./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/da_loc_puro_" << std::setfill ('0') << std::setw (15) << iter << ".txt";
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  LocationModelContainer::Iterator lj = m_locationContainer.Begin(); // imprimindo a posicao atual da localizacao
  file << m_maxx << "," << m_maxy << std::endl;
  file << temp << std::endl;
  file << lCentral->GetXPosition() << "," << lCentral->GetYPosition() << std::endl;
  file << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  }
  file << "\n";
  file.close();

  // os.str ("");
  // os << "python ./scratch/flynetwork/data/da_loc_puro.py " << m_pathData << " " << int(Simulator::Now().GetSeconds()) << " " << iter << " " << raio_cob;
  // ////NS_LOG_DEBUG (os.str());
  // system(os.str().c_str());
  // os.str ("");
  // os << "convert -delay 20 -loop 0 ./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/*.png" << " ./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << int(Simulator::Now().GetSeconds()) << "/da_loc.gif";
  // ////NS_LOG_DEBUG (os.str());
  // system(os.str().c_str());
}

} // namespace ns3
