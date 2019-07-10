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
#include "server-application.h"

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/trace-source-accessor.h"

#include <vector>
#include <fstream>
#include <string>

#include <algorithm>
#include <random>

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
                                        DoubleValue(300),
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
  // std::cout << "ServerApplication::ServerApplication @" << Simulator::Now().GetSeconds() << " Schedule Server [" << m_scheduleServer << "]\n";
  m_step = 0;
  m_aleatorio = true;
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

void ServerApplication::AddNewUav(uint32_t id, Ipv4Address addrAdhoc, double totalEnergy, double energyCost, double hoverCost, double totalBattery, Ptr<MobilityModel> mob)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << id << addrAdhoc << totalEnergy << energyCost << totalBattery << mob);

  // NS_LOG_DEBUG ("ServerApplication::AddNewUav Criando uav: " << id << " @" << Simulator::Now().GetSeconds());

  Ptr<Socket> socket;
  #ifdef TCP
    socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
  #endif
  #ifdef UDP
    socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  #endif

  ObjectFactory obj;
  obj.SetTypeId("ns3::UavModel");
  obj.Set("Id", UintegerValue(id));
  obj.Set("AddressAdhoc", Ipv4AddressValue(addrAdhoc));
  obj.Set("TotalEnergy", DoubleValue(totalEnergy));
  obj.Set("EnergyCost", DoubleValue(energyCost));
  obj.Set("HoverCost", DoubleValue(hoverCost));
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

void ServerApplication::AddSupplyUav(uint32_t id, Ipv4Address addrAdhoc, double totalEnergy, double energyCost, double hoverCost, double totalBattery, Ptr<MobilityModel> mob)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << id << addrAdhoc << totalEnergy << energyCost << totalBattery << mob << m_supplyPos);
  Ptr<UavModel> supplied = m_uavContainer.RemoveAt(m_supplyPos);
  ////NS_LOG_DEBUG("Criando supply uav: " << id << " last: " << supplied->GetId());

  Ptr<Socket> socket;
  #ifdef TCP
    socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
  #endif
  #ifdef UDP
    socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  #endif

  ObjectFactory obj;
  obj.SetTypeId("ns3::UavModel");
  obj.Set("Id", UintegerValue(id));
  obj.Set("AddressAdhoc", Ipv4AddressValue(addrAdhoc));
  obj.Set("TotalEnergy", DoubleValue(totalEnergy));
  obj.Set("EnergyCost", DoubleValue(energyCost)); // joule/meters
  obj.Set("HoverCost", DoubleValue(hoverCost)); // joule/s
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
  supplied->CancelSendPositionEvent();
  supplied->SetSendPositionEvent(Simulator::ScheduleNow(&ServerApplication::SendUavPacket, this, supplied));

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

  if (results.at(0).compare("UAV") == 0) {
    std::vector<double> pos;
    pos.push_back(std::stod (results.at(1),&sz));
    pos.push_back(std::stod (results.at(2),&sz));

    std::ostringstream mm;
    mm << "SERVER\t-1\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tUAV";
    pos.push_back(std::stod(results.at(3), &sz)); // UAV tem posição 3D
    if (CalculateDistanceCentral(pos)==0.0) { // caso o UAV esteja na central, desligar!
      Ptr<UavModel> uav = m_uavGoToCentral.FindUavModel(std::stoi(results.at(4), &sz));
      if (uav != NULL)
      {
        NS_LOG_DEBUG("SERVER received UAV pos na central @"<< Simulator::Now().GetSeconds() <<"  --- Removendo nó [" << uav->GetId() << "]");
        m_removeUav(uav->GetId(), (int)m_step); // desligando todas as informacoes do nó
        uav->Dispose(); // parando eventos do UavModel
        m_uavGoToCentral.RemoveUav(uav);
      }
      else
      {
        NS_LOG_DEBUG ("SERVER --- nao encontrou UAV no container central, procurando no container padrao -> @" << Simulator::Now().GetSeconds());
        uav = m_uavContainer.FindUavModel(std::stoi(results.at(4), &sz));
        if (uav != NULL)
        {
          NS_LOG_DEBUG("SERVER received UAV pos na central @"<< Simulator::Now().GetSeconds() <<"  --- Removendo nó [" << uav->GetId() << "]");
          m_removeUav(uav->GetId(), (int)m_step); // desligando todas as informacoes do nó
          uav->Dispose(); // parando eventos do UavModel
          m_uavContainer.RemoveUav(uav);
        }
        else
        {
          NS_LOG_DEBUG("SERVER - $$$$ [NÃO -- m_uavContainer] foi possivel encontrar o UAV que foi pra central [UAV] ID " << results.at(4));
        }
      }
      uav = 0;      
    } else {
      Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(4), &sz));
      if (uav != NULL)
      {
        uav->SetPosition(pos.at(0), pos.at(1));
        uav->ConfirmPosition(); // confirma o posicionamento do UAV
        uav->SetTotalEnergy(std::stod(results.at(5), &sz)); // atualiza energia total do UAV
        NS_LOG_DEBUG ("SERVER - atualizando UAV " << uav->GetId() << " @" << Simulator::Now().GetSeconds());
        if (uav->IsConfirmed()) {
          NS_LOG_DEBUG("SERVER - UAV #" << uav->GetId() << " confirmado no posicionamento");
          ReplyUav(uav);
        } else {
          NS_LOG_DEBUG("SERVER - UAV #" << uav->GetId() << " NAO chegou ao posicionamento");
          uav->CancelSendPositionEvent(); // cancela anterior
          uav->SetSendPositionEvent(Simulator::ScheduleNow(&ServerApplication::SendUavPacket, this, uav)); // envia posicionamento
        }
      }
      else
      {
        NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [UAV] --- fora da rede?! ID " << results.at(4));
      }
      uav = 0;
    }
    pos.clear();
   } else if (results.at(0).compare("UAVRECEIVED") == 0)  {
            Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
            if (uav != NULL)
            {
              NS_LOG_DEBUG("SERVER - UAVRECEIVED ::: UAV #" << uav->GetId() << " CANCEL SEND POSITION @" << Simulator::Now().GetSeconds());
              uav->CancelSendPositionEvent(); // recebida confirmacao do UAV      
              uav->GotoReceived(true);        
            } else
            {
              NS_LOG_DEBUG("SERVER - nao foi possivel encontrar o UAV [UAVRECEIVED no container padrao] ID " << results.at(1));
              uav = m_uavGoToCentral.FindUavModel(std::stoi(results.at(1), &sz));
              if (uav != NULL)
              {
                NS_LOG_DEBUG("SERVER - UAVRECEIVED ::: UAV #" << uav->GetId() << " @" << Simulator::Now().GetSeconds());
                uav->CancelSendPositionEvent(); // recebida confirmacao do UAV      
                uav->GotoReceived(true);        
              } else
              {
                NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV fora da rede?! ID " << results.at(1));
              }
            }
            uav = 0;
      } else if (results.at(0).compare("DATA") == 0) {
          NS_LOG_DEBUG("ServerApplication::TracedCallbackRxApp " << s << " @" << Simulator::Now().GetSeconds());
          Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
          if (uav != NULL)
          {
            uav->CancelAskCliDataEvent(); // recebida confirmacao do UAV
            uav->SetClientDataConfirmed(true); // recebeu confirmacao!
            uav->SetTotalEnergy(std::stod(results.at(2), &sz)); // atualiza energia total do UAV
            NS_LOG_DEBUG("SERVER - DATA confirmation ::: UAV #" << uav->GetId() << " @" << Simulator::Now().GetSeconds());
            m_allCli = 0.0;
            int i = 3; // 3 informacoes antes das informacoes dos clientes! contemplam cada cliente!
            Ptr<ClientModel> cli;
            for (; i < int(results.size()-1); i+=6) { // id time_update posx posy consumption totCli
              ObjectFactory obj;
              obj.SetTypeId("ns3::ClientModel");
              obj.Set("Id", UintegerValue(std::stoi(results.at(i)))); // id
              cli = obj.Create()->GetObject<ClientModel>();
              cli->SetUpdatePos(Seconds(std::stod(results.at(i+1), &sz))); // time
              std::vector<double> pos;
              pos.push_back(std::stod (results.at(i+2),&sz)); // x
              pos.push_back(std::stod (results.at(i+3),&sz)); // y
              cli->SetPosition(pos.at(0), pos.at(1));
              cli->SetConsumption(std::stod (results.at(i+4),&sz)); // consumption
              cli->SetTotalCli(std::stod (results.at(i+5),&sz)); // total de clientes que este equivale
              m_allCli += std::stod (results.at(i+5),&sz);
              m_clientContainer.Add(cli);
              pos.clear();
            }
            // repply to UAV
            ReplyAskCliData (uav);
          } else
          {
            NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [DATA] --- fora da rede?! ID " << results.at(1));
          }
        } else if (results.at(0).compare("NEWUAV") == 0)  {
            NS_LOG_DEBUG("SERVER - received NEWUAV  from UAV " << results.at(1));
            Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
            if (uav != NULL) {              
              ReplyNewUav(uav);
              m_uavGoToCentral.Add(uav);
              m_uavContainer.RemoveUav(uav);
              // solicitar novo UAV para a rede!
              m_newUav(1, 3); // solicita novo UAV
              // atualizar o posicionamento do Uav na última posicao do vetor, mandando ele para a localização do UAV que esta solicitando carga
              uav = m_uavContainer.GetLast();
              std::vector<double> p;
              p.push_back(std::stod (results.at(2),&sz));
              p.push_back(std::stod (results.at(3),&sz));
              p.push_back(std::stod (results.at(4), &sz)); // UAV tem posição 3D
              uav->SetNewPosition(p); 
              uav->CancelSendPositionEvent();
              uav->NotConfirmed();
              uav->SetSendPositionEvent(Simulator::ScheduleNow(&ServerApplication::SendUavPacket, this, uav));
              uav = 0;    
            } else {
              NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV [NEWUAV] --- fora da rede?! ID " << results.at(1));
            }
          } 
          #ifdef COM_SERVER
            else {// enviando pro UAV somente
              InetSocketAddress add = InetSocketAddress::ConvertFrom(address);
              // std::ostringstream mm;
              // mm << "SERVER\t-1\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tCLIENT";
              // m_packetTrace(mm.str());
              NS_LOG_FUNCTION("SERVER -- CLIENT ::: recebida informacoes de aplicacao do cliente no endereco " << add.GetIpv4());
              #ifdef LOG_CLIENT
                std::ostringstream os;
                os << global_path << "/" << m_pathData << "/client/" << add.GetIpv4() << ".txt";
                std::ofstream file;
                file.open(os.str(), std::ofstream::out | std::ofstream::app);
                file << Simulator::Now().GetSeconds() << " RECEBIDO " << packet->GetSize () << std::endl;// RECEBIDO pelo servidor
                file.close();
              #endif
            }
          #endif
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
    //m_packetTrace(msg.str());
    ////NS_LOG_DEBUG (msg.str());
  }
}

void ServerApplication::ReplyNewUav(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << uav->GetId());
  std::ostringstream msg;
  msg << "NEWUAVOK "<< '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    msg.str("");
    msg << "SERVER\tSENT\t@" << Simulator::Now().GetSeconds() << "\tNEWUAVOK";
    //m_packetTrace(msg.str());
  }
  NS_LOG_DEBUG (msg.str());
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
    msg << "SERVER\tSENT\t@" << Simulator::Now().GetSeconds() << "\tSERVEROK";
    //m_packetTrace(msg.str());
  }
  NS_LOG_DEBUG (msg.str());
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

    if (!(*i)->IsConfirmed() && !(*i)->IsGotoReceived()) { // nao reenvia, caso ja tenha recebido informacao do posicionamento
      (*i)->CancelSendPositionEvent();
      SendUavPacket((*i));
    }

  }
  if (flag) {
    NS_LOG_DEBUG ("SERVER - todos os UAVs estao na posicao desejada @" << Simulator::Now().GetSeconds());
    Run();
  } else {
    NS_LOG_DEBUG ("Server - [" << Simulator::Now().GetSeconds() << "] $$ UAVs [NÃO] estao prontos @" << Simulator::Now().GetSeconds());
    Simulator::Schedule(Seconds(1.00), &ServerApplication::ValidateUavPosition, this);
  }
}

void ServerApplication::Run ()
{
  Simulator::Remove(m_serverEvent);
  NS_LOG_DEBUG("ServerApplication::Run @" << Simulator::Now().GetSeconds());
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  if (!m_clientContainer.IsEmpty() || !m_fixedClientContainer.IsEmpty())
  {
    NS_LOG_DEBUG("\tSERVER - Iniciando execução dos DAs @" << Simulator::Now().GetSeconds());
    std::ostringstream ss;
    ss << "mkdir -p " << global_path << "/"<<m_pathData<<"/etapa/" << m_step << "";
    system(ss.str().c_str());
    std::ofstream file;
    ss.str("");
    ss <<global_path << "/" << m_pathData << "/etapa_time.txt";
    file.open(ss.str().c_str(), std::ofstream::out | std::ofstream::app);
    file << m_step << "," << int(Simulator::Now().GetSeconds()) << "\n";
    file.close();
    ss.str("");
    #ifdef COMPARE_COST
      ss << "mkdir -p " << global_path << "/"<<m_pathData<<"/compare/" << m_step;
      system(ss.str().c_str());
      ss.str("");
    #endif
    // runDAPython();
    // runDAPuro();
    ss.str("");
    ss <<global_path << "/" << m_pathData << "/seed.txt";
    file.open(ss.str().c_str(), std::ofstream::out | std::ofstream::app);
    file << int(Simulator::Now().GetSeconds()) << "," << SeedManager::GetSeed() << "," << global_ksize << "\n";
    file.close();
    
    if (m_custo == 5 && m_aleatorio) {
      aleatorio();
      m_aleatorio = false; // para executar a distribuicao somente na primeira etapa
    } else {
      runDA();
    }
    NS_LOG_INFO ("ServerApplication::Run liberando client container ");
    m_clientContainer.Clear();
    runAgendamento();
    NS_LOG_INFO ("ServerApplication::Run liberando location container ");
    m_locationContainer.Clear();

    m_step++;
  }
  else
  {
    NS_FATAL_ERROR("SERVER - $$$$ [NÃO] existem clientes identificados no servidor, ignorando execução dos DAs -- Se isso ocorre existe erro no cadastro de clientes fixos (que seriam os palcos)!");
  }
  // std::cout << "m_scheduleServer==" << m_scheduleServer << std::endl;
  m_serverEvent = Simulator::Schedule(Seconds(m_scheduleServer-1.0), &ServerApplication::AskClientData, this);
}

void ServerApplication::SendUavPacket(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << uav->GetId());
  ////NS_LOG_DEBUG("ServerApplication::SendUavPacket UAV Id "  << uav->GetId() << " @" << Simulator::Now().GetSeconds() << " REF " << uav->GetReferenceCount());
  uav->CancelSendPositionEvent();
  NS_ASSERT(uav != 0);
  Vector posCentral = GetNode()->GetObject<MobilityModel>()->GetPosition();
  std::vector<double> pos = uav->GetNewPosition();
  std::ostringstream msg;
  msg << "GOTO " << pos.at(0) << " " << pos.at(1) << " 10.0 " << posCentral.x << " " << posCentral.y << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    NS_LOG_DEBUG ("ServerApplication::SendUavPacket SERVER - UAV [" << Simulator::Now().GetSeconds() << "] ::: enviando informacoes de posicionamento para o UAV #" << uav->GetId());
    msg.str("");
    msg << "SERVER\t-1\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAV";
    //m_packetTrace(msg.str());
  } else {
    NS_FATAL_ERROR("ServerApplication::SendUavPacket SERVER - $$$$ [NÃO] conseguiu enviar informacoes de posicionamento para o UAV.");
  }
  uav->SetSendPositionEvent(Simulator::Schedule(Seconds(5.0), &ServerApplication::SendUavPacket, this, uav));
  NS_LOG_INFO ("ServerApplication::SendUavPacket  ------------ SendUavPacket end @" << Simulator::Now().GetSeconds());
  pos.clear();
  
  // enviando posicionamento!
  uav->GotoReceived(false);
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
  os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/location_client.txt";
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

  vector<vector<long double>> custo_x; // i - UAVs, j - localizacoes x=1,2ou3
  vector<int> uav_ids; // Uav id
  vector<int> loc_ids; // Loc id

  int count = 0;
  long double custo;
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
      goto recalcule;
    }
    NS_LOG_INFO (" ------------------------- ");
  }
  NS_LOG_DEBUG ("FIM custo ---------------------------- @" << Simulator::Now().GetSeconds());

  // PrintCusto (custo_x, m_step, true, uav_ids, loc_ids);

  std::ostringstream os;

  #ifdef COMPARE_COST
    std::ofstream file_uav, file_ule, file_l, file_c1, file_c2, file_c3, file_c4, file_uce, file_ult;
    os.str("");
    os << global_path << "/" << m_pathData << "/compare/"<<m_step<<"/uav_loc_travel_info.txt";
    file_ult.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/uav_loc_energy_info.txt";
    file_ule.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/uav_central_energy_info.txt";
    file_uce.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/uav_info.txt";
    file_uav.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/compare_c1.txt";
    file_c1.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/compare_c2.txt";
    file_c2.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/compare_c3.txt";
    file_c3.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/compare_c4.txt";
    file_c4.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    file_c1 << "compare_c1\n";
    file_c1 << m_uavContainer.GetN() << "\n";
    file_c2 << "compare_c2\n";
    file_c2 << m_uavContainer.GetN() << "\n";
    file_c3 << "compare_c3\n";
    file_c3 << m_uavContainer.GetN() << "\n";
    file_c4 << "compare_c4\n";
    file_c4 << m_uavContainer.GetN() << "\n";
    for (UavModelContainer::Iterator u_i = m_uavContainer.Begin();
       u_i != m_uavContainer.End(); ++u_i, ++count)
    {
      std::vector<double> vp = (*u_i)->GetPosition();
      file_uav << (*u_i)->GetId() << "," << vp.at(0) << "," << vp.at(1) << "," << (*u_i)->GetTotalEnergy() << "," << (*u_i)->GetTotalBattery() << "\n";
      file_uce << (*u_i)->CalculateEnergyCost(CalculateDistance((*u_i)->GetPosition(), central_pos)) << "\n";
      for (LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
         l_j != m_locationContainer.End(); ++l_j)
      {
        double t_loc = CalculateDistance((*u_i)->GetPosition(), (*l_j)->GetPosition()) / 5.0; // time to go to location
        file_ult << t_loc << ","; // 5m/s
        file_ule << (*u_i)->CalculateEnergyCost(CalculateDistance((*u_i)->GetPosition(), (*l_j)->GetPosition())) << ",";
        // custo
        long double b_ui_atu = (*u_i)->GetTotalEnergy(); // bateria atual
        long double ce_ui_la_lj = (*u_i)->CalculateEnergyCost(CalculateDistance((*u_i)->GetPosition(), (*l_j)->GetPosition())); // custo energetico
        long double ce_ui_lj_lc = (*u_i)->CalculateEnergyCost(CalculateDistance((*l_j)->GetPosition(), central_pos));
        long double b_ui_tot = (*u_i)->GetTotalBattery();
        long double b_ui_res = b_ui_atu*0.98 - ce_ui_la_lj - ce_ui_lj_lc; // bateria residual
        if (b_ui_res > 0) {
          file_c1 << (ce_ui_la_lj + ce_ui_lj_lc) / b_ui_tot << ","; 
          long double ce_te_lj = (*l_j)->GetTotalConsumption() * (m_scheduleServer-t_loc);
          long double P_te = b_ui_res/ce_te_lj;
          file_c2 << 1.0/P_te << ","; // invertendo para minimizar
          P_te = b_ui_res/(ce_te_lj+(ce_ui_la_lj + ce_ui_lj_lc));
          file_c3 << 1.0/P_te << ",";
          P_te = b_ui_res/(ce_te_lj+(ce_ui_la_lj + ce_ui_lj_lc + (*u_i)->GetHoverCost()*(m_scheduleServer-t_loc)));
          file_c4 << 1.0/P_te << ","; // invertendo para minimizar
        } else {
          file_c1 << "'1.0,";
          file_c2 << "'1.0,";
          file_c3 << "'1.0,";
          file_c4 << "'1.0,";
        }
      }
      file_ule << "\n";
      file_c1 << "\n";
      file_c2 << "\n";
      file_c3 << "\n";
      file_c4 << "\n";
      file_ult << "\n";
    }
    file_uce.close();
    file_ule.close();
    file_uav.close();
    file_c1.close();
    file_c2.close();
    file_c3.close();
    file_c4.close();
    file_ult.close();
    os.str("");
    os << global_path << "/"<<m_pathData<<"/compare/"<<m_step<<"/loc_info.txt";
    file_l.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
    for (LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
         l_j != m_locationContainer.End(); ++l_j)
    {
      std::vector<double> vp = (*l_j)->GetPosition();
      file_l << vp.at(0) << "," << vp.at(1) << "," << (*l_j)->GetTotalConsumption() << "," << (*l_j)->GetTotalCli() << "\n";
    }
    file_l.close();
  #endif

  // dsitribuindo UAVs
  std::vector<int> s_final; // cada posicao representa um UAV e o valor contido a localização que este deve ir!
  if (m_custo < 5) // somente executo o DA para os custos 1 à 4!
    s_final = DaPositioning(custo_x, m_uavContainer.GetN());
  else if (m_custo > 5) // para os custos maior que 5 será executada a busca exaustiva
    s_final = Exhaustive(custo_x, m_uavContainer.GetN()); // lembrando que custo_x já tem calculado o valor do custo correspondente
  else  { // executa aleatoriamente a escolha caso seja custo = 5
    for (int i = 0; i<(int)m_locationContainer.GetN(); ++i) {
      s_final.push_back(i);
    } 
    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(s_final), std::end(s_final), rng);
  }

  NS_LOG_DEBUG("SERVER - Finalizada estrutura do DA para agendamento @" << Simulator::Now().GetSeconds());

  NS_LOG_DEBUG("SERVER - Atualizando posicionamento dos UAVs @" << Simulator::Now().GetSeconds());
  int id, i = 0;
  // std::ofstream file;
  double t = 0.0;
  vector<vector<long double> > f_mij;
  for (UavModelContainer::Iterator u_i = m_uavContainer.Begin();
       u_i != m_uavContainer.End(); ++u_i, ++i)
  {
    id = s_final[i]; // id da localizacao que deve ir!
    f_mij.push_back(vector<long double>((int)m_locationContainer.GetN(), 0)); // inicia com zeros
    f_mij[i][id] = 1.0; // gravando matriz da solucao final

    if (custo_x[i][id] == 1.0) { // UAv nao tem bateria suficiente para ir ate a localizacao
      // solicitando novo UAV para a posicao
      m_supplyPos = count; // posicao que será suprida
      m_newUav(1, 1); // true, pois está o solicitando para suprir uma posicao
      custo = CalculateCusto((*u_i), m_locationContainer.Get(id), central_pos); // calculando custo do novo uav para a loc
      custo_x[i][id] = custo;
    }

    NS_LOG_DEBUG ("## ===> UAV " << (*u_i)->GetId() << " to LOC " << id);
    m_locationContainer.Get(id)->SetUsed(); // define como utilizada pelo UAV atual

    NS_LOG_DEBUG("SERVER - LOC "<< id << "\n\t" << m_locationContainer.Get(id)->toString());

    // Salvando onde o UAV estava e para onde ele será enviado.
    NS_LOG_DEBUG("SERVER - Salvando onde o UAV estava e para onde ele será enviado");
    std::vector<double> vp = (*u_i)->GetPosition();
    // if (i == 0)
    //   osuav << (*u_i)->GetId() << "," << vp.at(0) << "," << vp.at(1); // posicao do UAV
    // else
    //   osuav << "," << (*u_i)->GetId() << "," << vp.at(0) << "," << vp.at(1); // posicao do UAV
    vp = m_locationContainer.Get(id)->GetPosition();
    // if (i == 0)
    //   osloc << id << "," << vp.at(0) << "," << vp.at(1); // posicao da LOC
    // else
    //   osloc << "," << id << "," << vp.at(0) << "," << vp.at(1); // posicao da LOC

    NS_LOG_DEBUG("SERVER - definindo novo posicionamento @" << Simulator::Now().GetSeconds());

    (*u_i)->SetNewPosition(m_locationContainer.Get(id)->GetPosition());
    (*u_i)->NotConfirmed(); // atualiza o valor para identificar se o UAV chegou a posicao correta
    (*u_i)->CancelSendPositionEvent();
    if (CalculateDistanceCentral(m_locationContainer.Get(id)->GetPosition())==0.0) {
      (*u_i)->SetSendPositionEvent(Simulator::Schedule(Seconds(t), &ServerApplication::SendUavPacket, this, (*u_i)));
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
  os << global_path << "/"<<m_pathData<<"/etapa/" << m_step << "/f_mij.txt";
  PrintMij (f_mij, 0.0, os.str(), uav_ids, loc_ids);

  // os.str("");
  // os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/uav_loc.txt";
  // file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  // Vector serv_pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  // file << m_maxx << "," << m_maxy << std::endl << serv_pos.x << "," << serv_pos.y << std::endl << osuav.str() << std::endl << osloc.str() << std::endl;
  // file.close();

  // PrintCusto (custo_x, m_step, false, uav_ids, loc_ids); // pode ter sido modificado no laco anterior, um UAv pode ter sido suprido
  m_printUavEnergy(m_step); // esperando a solucao final, UAVs podem ser trocados

  NS_LOG_DEBUG ("-- Finalizado posicionamento dos UAVs @" << Simulator::Now().GetSeconds());
}

void 
ServerApplication::Permute (std::vector<int> uav_loc, int start, int end, int N) {
  // std::cout << "[" << start << "," << end << "] ";
  if (start == end) {
    double val = 0.0;
    for (int i = 0; i<N; ++i) { // percorre os UAVs
      // std::cout << uav_loc[i] << " ";
      val = val + g_custo[i][uav_loc[i]]; // acumula o custo total da solução em uav_loc
    }
    // std::cout << std::endl;
    if (val < m_global_val) { // avalia se é menor!
      // std::cout << "New value: " << val << std::endl;
      m_global_val = val;
      for (int i = 0; i<N; ++i) { // copiando
        m_minconf[uav_loc[i]] = i; // salvando uav/loc para nao ser necessario converter!
      }
    }
  } else {
    for (int i = start; i <= end; ++i) {
      int aux = uav_loc[i];
      uav_loc[i] = uav_loc[start];
      uav_loc[start] = aux;
      Permute (uav_loc, start+1, end, N);
      // backtrack
      aux = uav_loc[i];
      uav_loc[i] = uav_loc[start];
      uav_loc[start] = aux;
    }
  }
}

std::vector<int> 
ServerApplication::Exhaustive (std::vector<std::vector<long double> > custo, int N) {
  m_global_val = 1000.0;
  m_minconf.clear();
  for (int i = 0; i<N; ++i) { // i - UAVs, j - localizacoes
    g_custo.push_back(std::vector<long double>());
    for(int j = 0; j < N; j++)
    {      
      g_custo[i].push_back(custo[i][j]);
    }
  }
  
  std::vector<int> uav_loc;
  for (int i = 0; i<N; ++i) {
    uav_loc.push_back(i);
    m_minconf.push_back(i);
  }  

  Permute(uav_loc, 0, N-1, N);

  for (int i = 0; i<N; ++i) {
    g_custo[i].clear();
  }
  g_custo.clear();
  return m_minconf;
}

std::vector<int> ServerApplication::DaPositioning (std::vector<std::vector<long double> > b_ij, unsigned N) {
  double temp = 0.9;
  std::vector<int> proposed_FINAL;
  std::vector<int> used;
  // Mai
  std::vector<std::vector<long double> > m_ij;
  // variavel da tranformacao algebrica (parte do self-amplification)
  std::vector<std::vector<long double> > o_ij;
  // variavel da transformacao algebrica (parte o Y_sch)
  std::vector<std::vector<long double> > lamb_ij;
  // q_ai
  std::vector<std::vector<long double> > q_ij;
  // Ptr<UniformRandomVariable> e_ai = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
  // e_ai->SetAttribute ("Min", DoubleValue (min));
  // double rdom;

  for (unsigned i = 0; i < N; ++i) // UAV
  {
    m_ij.push_back(std::vector<long double>());
    o_ij.push_back(std::vector<long double>());
    lamb_ij.push_back(std::vector<long double>());
    q_ij.push_back(std::vector<long double>());
    for (unsigned j = 0; j < N; ++j) // LOC
    {
      // rdom = (double)rand()/RAND_MAX; // [0,1]
      m_ij[i].push_back(1.0/(double)N); // + rdom);
      o_ij[i].push_back(m_ij[i][j]);
      lamb_ij[i].push_back(0.0);
      q_ij[i].push_back(0.0);
    }
    proposed_FINAL.push_back(-1);
    used.push_back(-1);
  }

  long double gamma = 1;
  long double new_mij;
  unsigned i, j, k;
  int check;

  int itC_max = 50;
  int itB_max = 50;

  // std::cout << std::fixed << std::setw(8) << std::setprecision(8);

  while (temp >= 1e-3)
  {
    // std::cout << "---------------------------- temp: " << temp << std::endl;

    for (i = 0; i < N; ++i)
    {   
      used[i] = -1;
    }

    check = 0;
    bool converge_B = true;
    int itB = 0;
    do {
      converge_B = true;

      for (i = 0; i < N; ++i)
      {
        for (j = 0; j < N; ++j)
        {
          // calculate \lamb_{ij}
          lamb_ij[i][j] = m_ij[i][j] * b_ij[i][j];
          // calculate Q_{ij}
          q_ij[i][j] =  gamma * o_ij[i][j] - lamb_ij[i][j] * b_ij[i][j];        
          // calculate m_{ij}
          new_mij = expl((q_ij[i][j] / (long double) temp)); 
          if (isnan(new_mij)) {
            std::cout << "NAN! \n";
            exit(1);
          }
          if (m_ij[i][j] != new_mij) {
            converge_B = false;
          }
          m_ij[i][j] = new_mij;        
        }
      }

      bool converge_C = true;
      int itC = 0;
      do {
        converge_C = true;

        long double total;
        for (i = 0; i < N; ++i)
        {
          total = 0.0;
          for (k = 0; k < N; ++k)
          {
            total += m_ij[i][k];
          }
          for (k = 0; k < N; ++k)
          {
            new_mij = m_ij[i][k] / total;
            if (m_ij[i][k] != new_mij) {
              converge_C = converge_B = false;
            }
            m_ij[i][k] = new_mij;
          }
        }

        for (i = 0; i < N; ++i)
        {
          total = 0.0;
          for (k = 0; k < N; ++k)
          {
              total += m_ij[k][i];
          }
          for (k = 0; k < N; ++k)
          {
              new_mij = m_ij[k][i] / total;
              if (m_ij[k][i] != new_mij) {
                  converge_C = converge_B = false;
              }
              m_ij[k][i] = new_mij;
          }
        }
        itC++;
      } while (!converge_C && itC < itC_max);
      // if (converge_C) {
      //   std::cout << "Saiu por convergencia C: " << itC << std::endl;
      // }
      itB++;
    } while (!converge_B && itB < itB_max);
    // if (converge_B) {
    //   std::cout << "Saiu por convergencia B: " << itB << std::endl;
    // }

    // { // validate MIJ
    //   long double validatel;
    //   std::vector<long double> validate(N,0);
    //   std::cout << "...... MIJ --------- line\n";
    //   for (i = 0; i < N; ++i)
    //   {
    //     validatel = 0.0;
    //     std::cout << "[";
    //     for (j = 0; j < N; ++j)
    //     {
    //       std::cout << m_ij[i][j] << "\t\t";
    //       validatel += m_ij[i][j];
    //       validate[i] += m_ij[j][i];
    //     }  
    //     std::cout << "] = " << validatel << "\n";    
    //   }
    //   std::cout << "---------------------------------------------------------\n";
    //   std::cout << "[";
    //   for (i = 0; i < N; ++i)
    //   {
    //     std::cout << validate[i] << "\t\t";
    //   }
    //   std::cout << "]\n";
    // }

    // Validate solution
    for (i = 0; i < N; ++i)
    {
      for (j = 0; j < N; ++j)
      {
        if (m_ij[i][j] > 0.99999) {
          check++;
          proposed_FINAL[i] = j;
          used[j] = 1;  
          break;
        }
      }
    }

    // std::cout << "proposed_FINAL \n[";
    // for(i = 0; i < N; i++)
    // {
    //   std::cout << proposed_FINAL[i] << "\t\t";
    // }
    // std::cout << "]\n";

    if (check == (int)N) {
      goto out;
    }

    temp *= 0.99;
    o_ij = m_ij;
    // std::cout << " ---------------------------------------- \n";
  }
  // permite sair dos lacos ao encontrar 1 para cada localizacao
  out:
  if (check < (int) N) {
    unsigned find = 0;
    for(i = 0; i < N; i++)
    {
      if (proposed_FINAL[i] == -1) {
        // find  sequential unused
        for (; find < N; find++) {
          if (used[find] == -1) {
            proposed_FINAL[i] = find;
            used[find] = 1;
            find++;
            break;
          }
        }
      }
    }
  }
  return proposed_FINAL;
}

long double
ServerApplication::CalculateCusto (Ptr<UavModel> uav, Ptr<LocationModel> loc, vector<double> central_pos)
{
  // NS_LOG_DEBUG ("ServerApplication::CalculateCusto > uavId: " << uav->GetId() << " locId: " << loc->GetId());
  long double custo = 1.0;
  double t_loc = CalculateDistance(uav->GetPosition(), loc->GetPosition()) / global_speed; // time to go to location
  long double b_ui_atu = uav->GetTotalEnergy(); // bateria atual
  long double ce_ui_la_lj = uav->CalculateEnergyCost(CalculateDistance(uav->GetPosition(), loc->GetPosition())); // custo energetico
  long double ce_ui_lj_lc = uav->CalculateEnergyCost(CalculateDistance(loc->GetPosition(), central_pos));
  long double b_ui_res = b_ui_atu - ce_ui_la_lj - ce_ui_lj_lc; // bateria residual
  long double ce_te_lj = loc->GetTotalConsumption() * (m_scheduleServer-t_loc);
  double ce_hv;
  double inf = global_nc*etapa*3 + (2*m_rmax/global_speed)*global_ec_persec + etapa*global_ec_persec; // 3 Joules/s wifi

  // NS_LOG_DEBUG ("b_ui_atu=" << b_ui_atu << " b_ui_res=" << b_ui_res);

  if (b_ui_res > 0) {
    switch (m_custo) {
      case 1:
      case 6: // para calcular o exaustivo e diferenciar nas pastas! (ver: https://github.com/ggarciabas/teste/issues/45)
        custo = (ce_ui_la_lj + ce_ui_lj_lc) / inf; // media do custo
        break;
      case 2:
      case 7: // para calcular o exaustivo e diferenciar nas pastas! (ver: https://github.com/ggarciabas/teste/issues/45
        custo = ce_te_lj/inf; // UAV que terá mais bateria para servir a localizacao (conseguira servir por mais tempo)
        // NS_LOG_DEBUG ("P_te=" << P_te << " custo=" << custo);
        break;
      case 3:
      case 8: // para calcular o exaustivo e diferenciar nas pastas! (ver: https://github.com/ggarciabas/teste/issues/45
        custo = (ce_te_lj + ce_ui_la_lj + ce_ui_lj_lc) / inf; // quantos TEs consegue suprir?!
        // NS_LOG_DEBUG ("P_te=" << P_te << " custo=" << custo);
        break;
      case 4: // custo 2 -> com hover
      case 9: // para calcular o exaustivo e diferenciar nas pastas! (ver: https://github.com/ggarciabas/teste/issues/45
        ce_hv = uav->GetHoverCost()*(m_scheduleServer-t_loc) ; // custo para o TE inteiro, considerando locs e hover
        custo = (ce_te_lj + ce_ui_la_lj + ce_ui_lj_lc + ce_hv) / inf;
        // NS_LOG_DEBUG ("ce_hv=" << ce_hv << " P_te=" << P_te << " custo=" << custo);
        break;
      case 5: // aleatorio
        custo = 0; // quanto menor melhor, só pro aleatorio
        break;
    }
  } else {
    custo = inf;
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
    os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/bij.txt";
  } else {
    os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/bij_final.txt";
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
    os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/custo_" << m_custo << ".txt";
  } else  {
    os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/custo_" << m_custo << "_final.txt";
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
  file << Simulator::Now().GetSeconds() << std::endl;
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

void ServerApplication::aleatorio() {
  // area de distribuicao para teste  
  // obtendo região dos palcos para distribuir
  /*
  ClientModelContainer::Iterator it = m_fixedClientContainer.Begin();
  double maxX, maxY, minX, minY;
  maxX = minX = (*it)->GetXPosition();
  maxY = minY = (*it)->GetYPosition();
  it++;
  for (; it != m_fixedClientContainer.End(); ++it) {
    if (maxX < (*it)->GetXPosition()) {
      maxX = (*it)->GetXPosition();
    } else if (minX > (*it)->GetXPosition()) {
      minX = (*it)->GetXPosition();
    }
    if (maxY < (*it)->GetYPosition()) {
      maxY = (*it)->GetYPosition();
    } else if (minY > (*it)->GetYPosition()) {
      minY = (*it)->GetYPosition();
    }
  }
  */

  // distribuir a quantidade escolhida aleatoriamente
  Ptr<UniformRandomVariable> totLoc = CreateObject<UniformRandomVariable>();  
  totLoc->SetAttribute ("Min", DoubleValue (1));
  totLoc->SetAttribute ("Max", DoubleValue (m_fixedClientContainer.GetN()*2));

  Ptr<UniformRandomVariable> xPos = CreateObject<UniformRandomVariable>();  
  xPos->SetAttribute ("Min", DoubleValue (0));
  xPos->SetAttribute ("Max", DoubleValue (m_maxx));

  Ptr<UniformRandomVariable> yPos = CreateObject<UniformRandomVariable>();  
  yPos->SetAttribute ("Min", DoubleValue (0));
  yPos->SetAttribute ("Max", DoubleValue (m_maxy));

  int tLoc = totLoc->GetValue();
  ObjectFactory lObj;
  lObj.SetTypeId("ns3::LocationModel");
  for (int i = 0; i < tLoc; ++i) {
    Ptr<LocationModel> loc = lObj.Create()->GetObject<LocationModel>();
    loc->SetId(i);
    loc->SetPosition(xPos->GetValue(), yPos->GetValue());
    m_locationContainer.Add(loc);
  }

  // como validar se tem conexão??
  Ptr<LocationModel> lCentral = lObj.Create()->GetObject<LocationModel> ();
  lCentral->SetId(9999);
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition(); // posicao do servidor
  lCentral->SetPosition(pos.x, pos.y);

  // 1- Para cada Localizacao distribuída: m_locationContainer
  // 2- Iniciar distribuição de pontos para conexão
  // (função recursiva) 1- Informar p1, p2 e R
  //                    2- Se distancia entre p1 e p2 menor que R, retorna 0 
  //                    3- Senão, gera valor aleatório entre u=[0,1]
  //                    4- Posiciona nova localizacao em p3 = (1-u)p1 + up2
  //                    5- Chamar função recursiva para (p1, p3)
  //                    6- Chamar função recursiva para (p2, p3)
  for (LocationModelContainer::Iterator it = m_locationContainer.Begin(); it != m_locationContainer.End(); ++it) {
    this->aleNewLoc ((*it)->GetXPosition(), (*it)->GetYPosition(), pos.x, pos.y);
  }

  // salvando
  std::ofstream file;
  std::ostringstream os;
  os <<global_path << "/" << m_pathData << "/aleatorio.txt";
  file.open(os.str().c_str(), std::ofstream::out);
  file << m_maxx << "," << m_maxy << std::endl;  
  file << lCentral->GetXPosition() << "," << lCentral->GetYPosition() << std::endl;

  LocationModelContainer::Iterator lj = m_locationContainer.Begin();
  file << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  }
  file.close();
} 

void ServerApplication::aleNewLoc (double x1, double y1, double x2, double y2)
{ 
  if (std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2)) <= global_uav_cob) 
  {
    return;
  }
  Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable>();
  double u = urv->GetValue();
  std::cout << "U: " << u << "\n";
  ObjectFactory lObj;
  lObj.SetTypeId("ns3::LocationModel");
  Ptr<LocationModel> loc = lObj.Create()->GetObject<LocationModel>();
  loc->SetId(m_locationContainer.GetN());
  loc->SetPosition((x1*u)+(x2*(1-u)), (y1*u)+(y2*(1-u)));
  m_locationContainer.Add(loc);
  this->aleNewLoc(x1, y1, loc->GetXPosition(), loc->GetYPosition());
  this->aleNewLoc(x2, y2, loc->GetXPosition(), loc->GetYPosition());
}

// https://github.com/ggarciabas/nsnam_ns3/blob/17c1f9200727381852528ac4798f040128ac842a/scratch/teste/da_cpp/deterministic-annealing.cc
void ServerApplication::runDA() {
  NS_LOG_DEBUG("ServerApplication::runDA @" << Simulator::Now().GetSeconds());

  std::ofstream file;
  std::ostringstream os;
  os.str("");
  os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/client.txt";
  file.open(os.str().c_str(), std::ofstream::out);
  bool first = true;
  double tFix = m_fixedClientContainer.GetN();
  for (ClientModelContainer::Iterator i = m_clientContainer.Begin(); i != m_clientContainer.End(); ++i)
  {
    (*i)->EraseLocation();
    (*i)->SetPci(1/(m_allCli+tFix) * (*i)->GetTotalCli()); // peso baseado no total de clientes na regiao, principalmente quando utilizado cluster
    if (first) {
      file << (*i)->GetPosition().at(0) << " " << (*i)->GetPosition().at(1);
      first = false;
    } else {
      file << " " << (*i)->GetPosition().at(0) << " " << (*i)->GetPosition().at(1);
    }
  }
  file.close ();
  m_clientPosition (os.str ()); // Adicionando informacoes reais do ambiente
  for (ClientModelContainer::Iterator i = m_fixedClientContainer.Begin(); i != m_fixedClientContainer.End(); ++i)
  {
    (*i)->EraseLocation();
    (*i)->SetPci(1);
  }

  m_clientDaContainer.Clear();
  m_clientDaContainer.Add(m_fixedClientContainer); // clientes fixos antes pra dar prioridade na capacidade do UAV!
  m_clientDaContainer.Add(m_clientContainer);

  // constantes
  double t_min = 1e-7;
  double r_max = std::sqrt(std::pow(m_maxx, 2) + std::pow(m_maxy, 2));
  // 1550 series https://www.cisco.com/c/en/us/products/collateral/wireless/aironet-1550-series/data_sheet_c78-641373.html
  // 1570 series https://www.cisco.com/c/en/us/products/wireless/aironet-1570-series/datasheet-listing.html
  double uav_cob = global_uav_cob; 
  // double ptCli = 28; // dBm - potencia de transmissao máxima para o Ap Aironet 1550 series 802.11n 2.4GHz
  // double fsInterf = 0.0008; // fator de sobreposicao de espaco 5 (50%)
  // double dRCli = 6.5; // Mbps - taxa considerada por usuário
  double raio_cob = global_cli_cob; 
  // double sinrCliMin = -93; // dBm - tabela de Receive sensitivity para 2.4GHz 802.11n (HT20) MCS 0
  // double lambda = 3e8/2.4e9; // metros
  // double b = 3.7;
  // double pi = 3.141516; // pi
  // double maxDrUav = 1024; // Mbps -- verificar alguma Ref!!
  // double gain = 4; // dBi - tanto o ganho de recepcao como o de transmissao
  // double N_W = 10e-9*2e7; // dB - N0 = 10e-9 W/Hz -- B = 20MHz - Livro Goldsmith ref para N0
  // Fuck explanation dB and log relation: https://www.physicsforums.com/threads/confusion-with-db-equation-10-or-20.641850/#post-4105917
  // double pl_ref = 20*std::log10(4*pi/lambda); // dB - Friis Model
  // --> https://www.isa.org/standards-publications/isa-publications/intech-magazine/2002/november/db-vs-dbm/
  // Use dB when expressing the ratio between two power values. Use dBm when expressing an absolute value of power.
  // double pr_ref = ptCli + gain + gain - pl_ref; // dBm - potencia do sinal na distancia de referencia
  double t = 0.6;
  int locId = 0;
  int max_iterB = 5000;


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
  loc->CopyFirstPosition();
  loc->SetPunishNeighboor(0.2); // ALTERADO: valor inicial de punicao!
  // loc->InitializeWij (m_clientDaContainer.GetN()*dRCli); // considera que todos os clientes estao conectados ao primeiro UAv, isto para nao ter que calcular a distancia na primeira vez, esta validacao será feita a partir da primeira iteracao do laco A
  loc->SetFather(lCentral, CalculateDistance(lCentral->GetPosition(r_max), loc->GetPosition(r_max)), r_max, uav_cob);
  m_locationContainer.Add(loc);
  double percentCli = 1.0;
  int iter = 0;
  do {// laco A
    iter++;
    // file << "------------------------------> ItA: " << iter  << " temp: " << t << "\n";
    int tMovCon = 0;
    int tFixCon = 0;
    bool locConnected = true;
    // bool capacidade = true;
    bool movimentoB = true;
    int iterB = 0;
    do { // laco B
      locConnected = true;
      iterB++;
      tMovCon = 0;
      tFixCon = 0;

      for (ClientModelContainer::Iterator ci = m_clientDaContainer.Begin(); ci != m_clientDaContainer.End(); ++ci) {
        (*ci)->SetConnected(false); // limpando para não dar conflito!
        double Zci = 0.0;
        double low_dchilj = 1.5; // maior distancia é 1.0, valores normalizados!
        for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
          (*lj)->ClearClientString();
          double dcilj = CalculateDistance((*ci)->GetPosition(r_max), (*lj)->GetPosition(r_max));
          double pljci = std::exp ( - ((dcilj /*+ (*lj)->GetWij()/maxDrUav*/ + (((*ci)->IsConnected()) ? 1 : 0))/t) ); // NOVO - Verifica se o cliente possui conexao, caso nao tenha calcula normalmente, senao adiciona 1 para que a probabilidade deste em relacao ao UAv seja insignificante.
          Zci += pljci;
          (*lj)->SetTempPljci(pljci);
          if (low_dchilj > dcilj) { // achou UAV mais proximo
            low_dchilj = dcilj;
            // https://bitbucket.org/cpgeimestrado/rascunhocpgei/src/master/conversor.cpp
            // double pl_dB = 10*std::log10(dcilj)*b; // dB - modelo simplificado goldsmith
            // long double pr_W = dBmToWatts(pr_ref - pl_dB); // W
            // long double it_W = fsInterf*pr_W; // w
            // long double sinr_W = pr_W / (it_W + N_W); // W - modelo de goldsmith considera para escalar!!!
            // long double sinr_dBm = WattsToDbm(sinr_W); // dBm
            if (low_dchilj <= raio_cob/r_max) { // && sinr_dBm >= sinrCliMin) { // esta dentro da area de cobertura maxima da antena e recebe SINR min
              // NS_LOG_DEBUG ("-> CLI " << (*ci)->GetLogin() <<  " com " << (*lj)->GetId() << "\t Distancia: " << dcilj*r_max << "\t SINR: " << sinr_dBm << "dBm");
              Ptr<LocationModel> lCon = (*ci)->GetLocConnected();
              (*ci)->SetConnected(true);
              (*ci)->SetDataRate(0.0);
              if (lCon) { // caso tenha alguma informacao anterior, desconsidera nos calculos, para isto atualiza o loc
                lCon->toString();
                if (lCon->GetId() == (*lj)->GetId()) {
                  lCon->UpdateDistCli (dcilj);
                  lCon = 0;
                  continue; // nao faz alteracoes! Desnecessario!
                }
                lCon->RemoveClient(0.0, (*ci)->GetConsumption(), (*ci));
              }
              lCon = 0;
              (*ci)->SetLocConnected((*lj));
              // calcular a SNR e caso seja maior que o mínimo, considerar cliente conectado
              (*lj)->NewClient(0.0, (*ci)->GetConsumption(), dcilj, (*ci));
              (*ci)->SetDataRate(0.0);
              // file << "tFix: " << tFixCon << "\ttMovCon: " << tMovCon << std::endl;
            }
          }
        }
        for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
          if (Zci < 1e-90) {
            // file << "--> Zci esta baixo! @" << Simulator::Now().GetSeconds() << "\n";
            // t *= 1.2; // aumenta 120%
            t = 0.1;
            percentCli *= 0.9; 
            break;
          }
          (*lj)->AddPljCi((*ci), Zci, r_max); // finaliza o calculo do pljci na funcao e cadastra no map relacionando o ci
        }
        if ((*ci)->IsConnected()) {
          if ((*ci)->GetLogin().at(0) == 'f') {
            tFixCon++;
          } else {
            tMovCon += (*ci)->GetTotalCli(); // considerando o total de clientes na regiao, principalmente quanto utilizado cluster
          }
        }
      }

      // calcular lj novos - não consigo fazer no laco anterior pela falta dos valores acumulados (não tentar colcoar la!)
      for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
        (*lj)->UpdatePosition (m_maxx, m_maxy);
        // Avalia a utilizacao de capacidade das localizações
        // capacidade = capacidade && (*lj)->ValidarCapacidade(maxDrUav);
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

      // file << "Itb: " << iterB << "\n\tTemp: " << t << "\n\ttMovCon: " << tMovCon << "\n\ttFixCon: " << tFixCon << "\n\tLocConnected: " << ((locConnected) ? "true" : "false") << "\n\tCapacidade: " << ((capacidade) ? "true":"false") << "\n";

    } while (movimentoB && iterB < max_iterB);

    if (locConnected) {
      NS_LOG_DEBUG("DARun: localizacoes conectadas\n");
        if (tFixCon == tFix) {
          NS_LOG_DEBUG("DARun: clientes fixos conectados\n");
          if (tMovCon >= m_allCli*percentCli) {
            NS_LOG_DEBUG("DARun: porcentagem de clientes conectados alcancado\n");
            // file << "--> Finalizado - temp=" << t << std::endl;
            // t *= 0.5; // resfria bastante
            // GraficoCenarioDa(t, iter, lCentral, uav_cob, r_max, raio_cob, 0.0);
            break;
          } else {
            NS_LOG_DEBUG("DARun: porcentagem de clientes NAO alcancado\n");
          }
        } else {
          NS_LOG_DEBUG("DARun: clientes fixos NAO conectados\n");
        } 
    } else {
      NS_LOG_DEBUG("DARun: localizacoes NAO conectadas\n");
    }

    if (!MovimentoA()) { // } || (tFixCon != tFix && t == 0.1)) {
      // file << "--> Solicitando nova localizacao por não existir movimento em A @" << Simulator::Now().GetSeconds() << std::endl;
      // new_uav:
      NS_LOG_DEBUG("DARun: nova localização\n");
      Ptr<LocationModel> nLoc = lObj.Create()->GetObject<LocationModel> ();
      nLoc->SetId(locId++);
      CentroDeMassa(nLoc, lCentral, r_max);
      nLoc->IniciarMovimentoA(); // salvando posicionamento para comparacao de movimento no laco A
      nLoc->IniciarMovimentoB();
      nLoc->CopyFirstPosition();
      m_locationContainer.Add(nLoc);
      nLoc->SetPunishNeighboor(0.2);
      // nLoc->InitializeWij (0.0); // ninguem esta conectado a nova localizacao
      FindFather (m_locationContainer.GetN()-1, r_max, uav_cob, lCentral, true);
      t *= 2.0;
      // GraficoCenarioDa(t, iter, lCentral, uav_cob, r_max, raio_cob, maxDrUav);
      for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
        (*lj)->IniciarMovimentoA();
        (*lj)->LimparHistorico();
        (*lj)->UpdatePunishNeighboor(uav_cob/r_max);
      }
      continue;
    }

    // GraficoCenarioDa(t, iter, lCentral, uav_cob, r_max, raio_cob, maxDrUav);
    // Reiniciar Movimento A para cada Localizacao
    for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
      (*lj)->IniciarMovimentoA();
      (*lj)->LimparHistorico();
      (*lj)->UpdatePunishNeighboor(uav_cob/r_max);
    }

    t = t*0.9; // reduz 90%  a tempreatura

  } while (t > t_min); // laco da temperatura

  // file << std::endl;
  // file.close ();

  m_totalCliGeral = 0;
  m_locConsTotal = 0; // atualiza total de consumo de todas as localizacoes

  os.str("");
  os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/da_solve.txt";
  file.open(os.str().c_str(), std::ofstream::out);
  file << raio_cob << std::endl;
  file << m_maxx << "," << m_maxy << std::endl;
  file << lCentral->GetXPosition() << "," << lCentral->GetYPosition() << std::endl;

  LocationModelContainer::Iterator lj = m_locationContainer.Begin();
  lj = m_locationContainer.Begin(); // imprimindo a posicao atual da localizacao
  file << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetXPosition() << "," << (*lj)->GetYPosition();
  }
  file << "\n";

  lj = m_locationContainer.Begin(); // imprimindo a posicao antiga
  file << (*lj)->GetXFirstPosition() << "," << (*lj)->GetYFirstPosition();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetXFirstPosition() << "," << (*lj)->GetYFirstPosition();
  }
  file << "\n";
  file.close();

  // os.str("");
  // os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/location_data_rate.txt";
  // file.open(os.str().c_str(), std::ofstream::out);
  // for (LocationModelContainer::Iterator lj = m_locationContainer.Begin(); lj != m_locationContainer.End(); ++lj) {
  //   file << (*lj)->GetId() << "," << (*lj)->GetDataRate() << std::endl;
  // }
  // file.close();

  // os.str("");
  // os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/client_data_rate.txt";
  // file.open(os.str().c_str(), std::ofstream::out);
  // for (ClientModelContainer::Iterator ci = m_clientContainer.Begin(); ci != m_clientContainer.End(); ++ci) {
  //   file << (*ci)->GetLogin() << "," << (*ci)->GetDataRate() << std::endl;
  // }
  // file.close();

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
  os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/da_loc_cpp_" << std::setfill ('0') << std::setw (15) << iter << ".txt";
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
  file << "\n";

  lj = m_locationContainer.Begin(); // imprimindo consumption
  file << (*lj)->GetTotalConsumption();
  lj++;
  for (; lj != m_locationContainer.End(); ++lj) {
    file << "," << (*lj)->GetTotalConsumption();
  }

  file.close();

  // os.str ("");
  // // custo, etapa, main_path, teste, iter
  // os << "python " << global_path << "/data/da_loc.py custo_" << m_custo << " " << m_step << " " << global_path << "/" << m_scenarioName << "/ False " << iter;
  // //NS_LOG_DEBUG (os.str());
  // system(os.str().c_str());
  // os.str ("");
  // os << "convert -delay 20 -loop 0 " << global_path << "/" << m_pathData << "/etapa/" << m_step << "/*.png" << " " << global_path << "/" << m_pathData << "/etapa/" << m_step << "/da_loc.gif";
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
    // std::cout << "==> Todos os clientes conectados.\n";
    l->SetPosition(central->GetXPosition(), central->GetYPosition()); // posicionar no centro dos clientes
  } else {
    // std::cout << "Centro de massa: " << x/ccon << " " << y/ccon << std::endl;
    l->SetPosition(x/ccon, y/ccon); // posicionar no centro dos clientes
  }
}

void ServerApplication::runDAPuro() {
  //NS_LOG_DEBUG("ServerApplication::runDAPuro @" << Simulator::Now().GetSeconds());
  std::ofstream file;
  std::ostringstream os;
  os.str("");
  os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/client.txt";
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
          tMovCon+= (*ci)->GetTotalCli();
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

    if (tMovCon >= m_allCli*0.8) {
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
  os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/location_client.txt";
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
  os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/da_loc_puro_" << std::setfill ('0') << std::setw (15) << iter << ".txt";
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
  // os << "python " << global_path << "/data/da_loc_puro.py " << m_pathData << " " << m_step << " " << iter << " " << raio_cob;
  // ////NS_LOG_DEBUG (os.str());
  // system(os.str().c_str());
  // os.str ("");
  // os << "convert -delay 20 -loop 0 " << global_path << "/" << m_pathData << "/etapa/" << m_step << "/*.png" << " " << global_path << "/" << m_pathData << "/etapa/" << m_step << "/da_loc.gif";
  // ////NS_LOG_DEBUG (os.str());
  // system(os.str().c_str());
}


void ServerApplication::runDAPython()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );

  std::ofstream cenario, file;
  std::ostringstream os;
  os.str("");
  os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/cenario_in.txt";
  cenario.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  if (cenario.is_open())
  {
    cenario << m_environment << "\n";
    cenario << m_locationContainer.GetN() << std::endl;
    Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
    cenario << pos.x << "," << pos.y;

    os.str("");
    os << global_path << "/"<<m_pathData<<"/etapa/"<<m_step<<"/client.txt";
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

    #ifdef COMPARE_COST
      os.str("");
      os << "cp " << global_path << "/" << m_pathData << "/etapa/" << m_step << "/client.txt " << global_path << "/" << m_pathData << "/compare/" << m_step;
    #endif
  }

  m_totalCliGeral = 0;

  os.str ("");
  os << "python " << global_path << "/da_python " << m_pathData << " " << m_step << " > " << global_path << "/" << m_pathData << "/etapa/" << m_step << "/python_log.txt";
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
    os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/cenario_out.txt";
    cenario_in.open(os.str().c_str(), std::ofstream::in);
    if (cenario_in.is_open())
    {
      double x, y;
      std::string line;
      ObjectFactory obj;
      obj.SetTypeId("ns3::LocationModel");
      uint32_t id = 0;
      m_locConsTotal = 0.0;
      // os.str("");
      // os <<global_path << "/" << m_pathData << "/etapa/" << m_step << "/location_client.txt";
      // std::ofstream location_cli;
      // location_cli.open(os.str().c_str(), std::ofstream::out);
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

        // location_cli << loc->GetId() << "," << loc->GetTotalCli() << "," << loc->GetTotalConsumption() << std::endl;
      }
      if ( m_locConsTotal == 0) {
        m_locConsTotal = 1.0; // para nao dar problemas no calculo
      }
      // location_cli.close();
      cenario_in.close();
    }
    else
    {
      NS_FATAL_ERROR("SERVER - $$$$ [NÃO] foi possível abrir para ler informacoes de localização");
    }
  }
}



} // namespace ns3
