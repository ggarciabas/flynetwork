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
                                        DoubleValue(60.0),
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
  NS_LOG_FUNCTION(this);
  std::cout << "ServerApplication::ServerApplication @" << Simulator::Now().GetSeconds() << "\n";
}

ServerApplication::~ServerApplication()
{
  NS_LOG_FUNCTION(this);
  std::cout << "ServerApplication::~ServerApplication @" << Simulator::Now().GetSeconds() << "\n";
  m_uavContainer.Clear();
  m_locationContainer.Clear();
  m_fixedClientContainer.Clear();
  m_clientContainer.Clear();
}

void ServerApplication::AddNewUav(uint32_t id, Ipv4Address addrAdhoc, double totalEnergy, double energyCost, double totalBattery, Ptr<MobilityModel> mob)
{
  NS_LOG_FUNCTION(this);

  NS_LOG_DEBUG ("ServerApplication::AddNewUav Criando uav: " << id << " @" << Simulator::Now().GetSeconds());

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
  NS_LOG_FUNCTION(this);

  NS_LOG_DEBUG("Criando supply uav: " << id);

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
  m_uavContainer.Add(uav, m_supplyPos); // posicao que deve ser suprida
}

void ServerApplication::AddNewFixedClient(string login, double x, double y)
{
  NS_LOG_FUNCTION(this);
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
  NS_LOG_FUNCTION(this);
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
    pos.push_back(std::stod(results.at(3), &sz)); // UAV tem posição 3D
    Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(4), &sz));
    if (uav != NULL)
    {
      uav->SetPosition(pos.at(0), pos.at(1));
      uav->ConfirmPosition(); // confirma o posicionamento do UAV
      uav->SetTotalEnergy(std::stod(results.at(5), &sz)); // atualiza energia total do UAV
      NS_LOG_INFO ("SERVER - atualizando UAV " << uav->GetId());
      if (uav->IsConfirmed()) {
        NS_LOG_INFO("SERVER - UAV #" << uav->GetId() << " confirmado no posicionamento");
        ReplyUav(uav);
      }
    }
    else
    {
      NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV --- fora da rede?! ID " << results.at(4));
    }
    pos.clear();
   } else if (results.at(0).compare("UAVRECEIVED") == 0)  {
            Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
            if (uav != NULL)
            {
              uav->CancelSendPositionEvent(); // recebida confirmacao do UAV
              NS_LOG_INFO("SERVER - UAVRECEIVED ::: UAV #" << uav->GetId() << " @" << Simulator::Now().GetSeconds());
            } else
            {
              NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV --- fora da rede?! ID " << results.at(1));
            }
        } else if (results.at(0).compare("DATA") == 0) {
          NS_LOG_INFO("ServerApplication::TracedCallbackRxApp " << s << " @" << Simulator::Now().GetSeconds());
          Ptr<UavModel> uav = m_uavContainer.FindUavModel(std::stoi(results.at(1), &sz));
          if (uav != NULL)
          {
            uav->CancelAskCliDataEvent(); // recebida confirmacao do UAV
            uav->SetClientDataConfirmed(true); // recebeu confirmacao!
            uav->SetTotalEnergy(std::stod(results.at(2), &sz)); // atualiza energia total do UAV
            NS_LOG_INFO("SERVER - DATA confirmation ::: UAV #" << uav->GetId() << " @" << Simulator::Now().GetSeconds());
            int i = 4; // 4 informacoes antes das informacoes dos clientes!
            double consumo_mean = std::stod(results.at(3), &sz) / ((results.size() - 1 - i)/4.0); // blocos de 4 contemplam cada cliente!
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
              cli->SetConsumption(cli->GetConsumption()+consumo_mean);
              NS_LOG_INFO ("ServerApplication::TracedCallbackRxApp \n" << cli->ToString());
            }
            // repply to UAV
            ReplyAskCliData (uav);
          } else
          {
            NS_LOG_DEBUG("SERVER - $$$$ [NÃO] foi possivel encontrar o UAV --- fora da rede?! ID " << results.at(1));
          }
        } else {
          std::ostringstream mm;
          mm << "SERVER\t-1\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tCLIENT";
          m_packetTrace(mm.str());
          NS_LOG_INFO("SERVER -- CLIENT ::: recebida informacoes de aplicacao do cliente no endereco " << add.GetIpv4());
        }
    results.clear();
}

void ServerApplication::ReplyAskCliData(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this);
  std::ostringstream msg;
  msg << "DATAOK "<< '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    msg.str("");
    msg << "SERVER\t-1\tSENT\t" << Simulator::Now().GetSeconds() << "\tDATAOK";
    m_packetTrace(msg.str());
  }
}

void ServerApplication::ReplyUav(Ptr<UavModel> uav)
{
  NS_LOG_FUNCTION(this);
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
  NS_LOG_FUNCTION(this);
  m_rmax = std::sqrt(std::pow(m_maxx,2)+std::pow(m_maxy,2));
  m_newUav(1,false); // inicia com um UAV
  ValidateUavPosition();
}

void ServerApplication::SendAskClientPacket(Ptr<UavModel> uav)
{
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
  NS_LOG_FUNCTION(this);
  Simulator::Remove(m_serverEvent);
}

void ServerApplication::ValidateUavPosition()
{
  NS_LOG_FUNCTION(this);
  // Verificar se todos os UAVs estão no posicionamento desejado!
  UavModelContainer::Iterator i;
  bool flag = true;
  for (i = m_uavContainer.Begin(); i != m_uavContainer.End(); ++i) {
    flag = (flag && ((*i)->IsConfirmed() && (*i)->ClientDataConfirmed())); // espera receber informacoes de dados do cliente tbm!
  }
  if (flag) {
    NS_LOG_INFO ("SERVER - todos os UAVs estao na posicao desejada @" << Simulator::Now().GetSeconds());
    Run();
  } else {
    NS_LOG_INFO ("Server - [" << Simulator::Now().GetSeconds() << "] $$ [NÃO] estao prontos @" << Simulator::Now().GetSeconds());
    Simulator::Schedule(Seconds(20.0), &ServerApplication::ValidateUavPosition, this);
  }
}

void ServerApplication::Run ()
{
  NS_LOG_FUNCTION(this);
  if (!m_clientContainer.IsEmpty() || !m_fixedClientContainer.IsEmpty())
  {
    NS_LOG_DEBUG("SERVER - Iniciando execução dos DAs @" << Simulator::Now().GetSeconds());
    std::ostringstream ss;
    ss << "mkdir -p ./scratch/flynetwork/data/output/"<<m_scenarioName<<"/" << int(Simulator::Now().GetSeconds());
    system(ss.str().c_str());
    ss.str("");
    runDAPython();
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

void ServerApplication::SendUavPacket(Ptr<UavModel> uav)
{
  NS_LOG_DEBUG("UAV Id "  << uav->GetId() << " @" << Simulator::Now().GetSeconds() << " REF " << uav->GetReferenceCount());
  uav->CancelSendPositionEvent();
  NS_ASSERT(uav != 0);
  NS_LOG_FUNCTION(this);
  std::vector<double> pos = uav->GetNewPosition();
  std::ostringstream msg;
  msg << "SERVER " << pos.at(0) << " " << pos.at(1) << " 10.0" << '\0';
  uint16_t packetSize = msg.str().length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
  if (uav->GetSocket()->Send(packet, 0) == packetSize) {
    NS_LOG_INFO ("SERVER - UAV [" << Simulator::Now().GetSeconds() << "] ::: enviando informacoes de posicionamento para o UAV #" << uav->GetId());
    msg.str("");
    msg << "SERVER\t-1\tSENT\t" << Simulator::Now().GetSeconds() << "\tUAV";
    m_packetTrace(msg.str());
  } else {
    NS_FATAL_ERROR("SERVER - $$$$ [NÃO] conseguiu enviar informacoes de posicionamento para o UAV.");
  }
  uav->SetSendPositionEvent(Simulator::Schedule(Seconds(5.0), &ServerApplication::SendUavPacket, this, uav));
  NS_LOG_INFO (" ------------ SendUavPacket end @" << Simulator::Now().GetSeconds());
  pos.clear();
}

void ServerApplication::runDAPython()
{
  NS_LOG_FUNCTION(this);

  std::ofstream cenario, file;
  std::ostringstream os;
  os.str("");
  os <<"./scratch/flynetwork/data/output/" << m_scenarioName << "/" << int(Simulator::Now().GetSeconds()) << "/cenario_in.txt";
  cenario.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);

  if (cenario.is_open())
  {
    cenario << m_environment << "\n";
    cenario << m_locationContainer.GetN() << std::endl;
    Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
    cenario << pos.x << "," << pos.y;

    os.str("");
    os << "./scratch/flynetwork/data/output/"<<m_scenarioName<<"/"<<int(Simulator::Now().GetSeconds())<<"/client.txt";
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
  os << "python ./scratch/flynetwork/da_python " << m_scenarioName << " " << int(Simulator::Now().GetSeconds()) << " > ./scratch/flynetwork/data/output/" << m_scenarioName << "/" << int(Simulator::Now().GetSeconds()) << "/python_log.txt";
  int status = system(os.str().c_str());
  if (status < 0)
  {
    NS_FATAL_ERROR("SERVER - Erro na execução do algoritmo em Python!");
  }
  else
  {
    std::ifstream cenario_in;
    os.str("");
    os <<"./scratch/flynetwork/data/output/" << m_scenarioName << "/" << int(Simulator::Now().GetSeconds()) << "/cenario_out.txt";
    cenario_in.open(os.str().c_str(), std::ofstream::in);
    if (cenario_in.is_open())
    {
      double x, y;
      std::string line;
      ObjectFactory obj;
      obj.SetTypeId("ns3::LocationModel");
      uint32_t id = 0;
      m_locConsTotal = 0.0;
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
      }
      if ( m_locConsTotal == 0) {
        m_locConsTotal = 1.0; // para nao dar problemas no calculo
      }
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
  NS_LOG_FUNCTION(this);
  Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  ObjectFactory fac;
  fac.SetTypeId("ns3::LocationModel");
  fac.Set("Id", UintegerValue(m_locationContainer.GetN() + 1));
  Ptr<LocationModel> loc = fac.Create()->GetObject<LocationModel>();
  loc->SetPosition(pos.x, pos.y);
  m_locationContainer.Add(loc);
}

double
ServerApplication::CalculateDistanceCentral(const std::vector<double> pos) {
  NS_LOG_FUNCTION(this);
  Vector v = GetNode()->GetObject<MobilityModel>()->GetPosition();
  double dist = std::sqrt(std::pow(pos.at(0) - v.x, 2) + std::pow(pos.at(1) - v.y, 2));
  NS_LOG_INFO("SERVER -- DIST " << dist);
  return dist; // euclidean, sempre considera a distância atual calculada pelo DA
}

double
ServerApplication::CalculateDistance(const std::vector<double> pos1, const std::vector<double> pos2)
{
  NS_LOG_FUNCTION(this);
  double dist = std::sqrt(std::pow(pos1.at(0) - pos2.at(0), 2) + std::pow(pos1.at(1) - pos2.at(1), 2));
  return dist; // euclidean, sempre considera a distância atual calculada pelo DA
}

bool ServerApplication::ValidateMijConvergency(vector<vector<double>> vec, vector<vector<double>> m_ij, unsigned siz)
{
  NS_LOG_FUNCTION(this);
  for (unsigned i = 0; i < siz; ++i)
  {
    for (unsigned j = 0; j < siz; ++j)
    {
      if (vec[i][j] != m_ij[i][j])
        return false;
    }
  }

  for (unsigned i = 0; i < siz; ++i)
  {
    vec[i].clear();
    m_ij[i].clear();
  }
  m_ij.clear();
  vec.clear();

  return true;
}

void ServerApplication::runAgendamento(void)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_locationContainer.GetN() != 0);

  //  - fazer com que o número de localizacoes e uavs sejam iguais!
  //    -- se mais loc, add novos UAVs, senão criar localizações na central!
  int diff = m_locationContainer.GetN() - m_uavContainer.GetN();
  if (diff > 0)
  { // add new uav
    NS_LOG_DEBUG("SERVER - Solicitando novos UAVs @" << Simulator::Now().GetSeconds());
    // chamar callback para criar novos UAVs
    m_newUav(diff, false); // chamando callback
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
  vector<vector<double>> b_ij; // i - UAVs, j - localizacoes
  vector<vector<double>> custo_x; // i - UAVs, j - localizacoes x=1,2ou3
  int count = 0;
  double custo;
  vector<double> central_pos;
  Vector vp = GetNode()->GetObject<MobilityModel>()->GetPosition();
  central_pos.push_back(vp.x);
  central_pos.push_back(vp.y);

  int verify_uav; // utilizado para verificar se um UAV não possui carga suficiente para se direcionar a alguma das localizações, isto acontece quando um UAV possui para todas as localizações o valor 1.!

  NS_LOG_INFO ("Calculando custo ---------------------------- @" << Simulator::Now().GetSeconds());
  for (UavModelContainer::Iterator u_i = m_uavContainer.Begin();
       u_i != m_uavContainer.End(); ++u_i, ++count)
  {
    NS_LOG_INFO ("UAV :: " << (*u_i)->toString());

    verify_uav = 0;
    b_ij.push_back(vector<double>());
    custo_x.push_back(vector<double>());
    for (LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
         l_j != m_locationContainer.End(); ++l_j)
    {
      NS_LOG_INFO ("LOC :: " << (*l_j)->toString());

      double b_ui_atu = (*u_i)->GetTotalEnergy(); // bateria atual
      double ce_ui_la_lj = (*u_i)->CalculateEnergyCost(CalculateDistance((*u_i)->GetPosition(), (*l_j)->GetPosition())); // custo energetico
      double ce_ui_lj_lc = (*u_i)->CalculateEnergyCost(CalculateDistance((*l_j)->GetPosition(), central_pos));
      double b_ui_tot = (*u_i)->GetTotalBattery();

      double b_ui_res = b_ui_atu - ce_ui_la_lj - ce_ui_lj_lc; // bateria residual

      double c_lj = (*l_j)->GetTotalCli();
      double c_total = m_totalCliGeral;

      if (b_ui_res > 0) {
        switch (m_custo) {
          case 1:
            custo = (ce_ui_la_lj + ce_ui_lj_lc) / b_ui_tot; // media do custo
            break;
          case 2:
            custo = 1 - (c_lj/c_total + b_ui_res/b_ui_tot)/2.0;
            break;
          case 3:
            custo = ((1 - (c_lj/c_total + b_ui_res/b_ui_tot)/2.0) + ((ce_ui_la_lj + ce_ui_lj_lc) / b_ui_tot))/2.0;
        }
      } else {
        NS_LOG_INFO("Sem bateria para esta localizacao!");
        custo = 1.0; // máximo, como se consumisse toda a bateria quando carregada!
      }

      custo_x[count].push_back(custo);
      b_ij[count].push_back(custo);
      verify_uav += custo; // somando o valor dos custos, assim se ao final tiver o mesmo valor que o total de localizações, quer dizer que este UAV somente tem carga para voltar a central
    }

    if (verify_uav == int(m_locationContainer.GetN())) {
      NS_LOG_DEBUG("ServerApplication::runAgendamento --> Enviar UAV " << (*u_i)->GetId() << " para a central  REF " << (*u_i)->GetReferenceCount());
      m_removeUav((*u_i)->GetId());
      (*u_i)->Dispose();
      m_supplyPos = count; // posicao que será suprida
      // criar um novo nó iniciando na região central, como sempre!
      m_newUav(1, true); // true, pois está sendo solicitado para suprir uma posicao
      // calcular o custo para este novo nó e todas as localizações
      // u_i--; // verificar se isto dá certo!
    }
    NS_LOG_INFO (" ------------------------- ");
  }
  NS_LOG_INFO ("FIM custo ---------------------------- @" << Simulator::Now().GetSeconds());

  PrintCusto(custo_x, int(Simulator::Now().GetSeconds()));

  // Inicializando
  NS_LOG_INFO ("SERVER - inicializando matrizes.");
  double temp = 0.99;
  // Mai
  vector<vector<double>> m_ij;
  // variavel da tranformacao algebrica (parte do self-amplification)
  vector<vector<double>> o_ij;
  // variavel da transformacao algebrica (parte o Y_sch)
  vector<vector<double>> lamb_ij;
  // q_ai
  vector<vector<double>> q_ij;
  double N = m_uavContainer.GetN();
  unsigned siz = m_uavContainer.GetN();
  // Ptr<UniformRandomVariable> e_ai = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
  // e_ai->SetAttribute ("Min", DoubleValue (min));

  for (unsigned i = 0; i < siz; ++i)
  {
    m_ij.push_back(vector<double>());
    o_ij.push_back(vector<double>());
    lamb_ij.push_back(vector<double>());
    q_ij.push_back(vector<double>());
    for (unsigned j = 0; j < (unsigned)m_locationContainer.GetN(); ++j)
    {
      // e_ai->SetAttribute ("Max", DoubleValue (max));
      // double rdom = e_ai->GetValue();
      m_ij[i].push_back(1 / N); // + rdom);
      o_ij[i].push_back(m_ij[i][j]);
      lamb_ij[i].push_back(0.0);
      q_ij[i].push_back(0.0);
    }
  }

  unsigned itB_max = 1000;
  unsigned itA_max = 1000;
  unsigned itC_max = 50;
  double gamma = 0.95;
  vector<vector<double>> copyB_mij;
  vector<vector<double>> copyC_mij;
  NS_LOG_INFO ("SERVER - iniciando execucao das partes A, B e C @" << Simulator::Now().GetSeconds());

  PrintBij(b_ij, int(Simulator::Now().GetSeconds()));

  int print = 0;
  PrintMij (m_ij, print++, 1.0);

  // Part A
  unsigned itA = 0;
  while (temp >= 0.005 && itA < itA_max)
  {
    // Part B
    unsigned itB = 0;
    do
    {
      copyB_mij = m_ij; // copy assignment copies data

      for (unsigned i = 0; i < siz; ++i)
      {
        for (unsigned j = 0; j < siz; ++j)
        {
          // calculate \lamb_{ij}
          lamb_ij[i][j] = m_ij[i][j] * b_ij[i][j];
          // calculate Q_{ij}
          q_ij[i][j] = gamma * o_ij[i][j] - lamb_ij[i][j] * b_ij[i][j];
          // calculate m_{ij}
          m_ij[i][j] = exp(q_ij[i][j] / temp);
        }
      }

      // Part C
      unsigned itC = 0;
      do
      {
        copyC_mij = m_ij; // copy assignment copies data

        // normalizando as linhas
        unsigned check = 0;
        for (unsigned i = 0; i < siz; ++i)
        {
          double total_linha = 0.0;
          for (unsigned k = 0; k < siz; ++k)
          {
            if (m_ij[i][k] == 1.0) {
              check++;
            }
            total_linha += m_ij[i][k];
          }
          for (unsigned j = 0; j < siz; ++j)
          {
            m_ij[i][j] /= total_linha;
          }
        }

        if (check == siz) {
          NS_LOG_DEBUG ("GOING OUT!! ------------------------>>>>>>");
          goto out;
        }

        // normalizando colunas
        for (unsigned j = 0; j < siz; ++j)
        {
          double total_coluna = 0.0;
          for (unsigned k = 0; k < siz; ++k)
          {
            total_coluna += m_ij[k][j];
          }

          for (unsigned i = 0; i < siz; ++i)
          {
            m_ij[i][j] /= total_coluna;
          }
        }

        itC++;
      } while (!ValidateMijConvergency(copyC_mij, m_ij, siz) && itC < itC_max);

      itB++;
    } while (!ValidateMijConvergency(copyB_mij, m_ij, siz) && itB < itB_max);

    PrintMij (m_ij, print++, temp);

    temp *= m_rho;
    o_ij = m_ij;
    itA++;
  }
  // permite sair dos lacos ao encontrar 1 para cada localizacao
  out:

  NS_LOG_INFO("SERVER - Finalizada estrutura do DA para agendamento @" << Simulator::Now().GetSeconds());

  NS_LOG_INFO("SERVER - Atualizando posicionamento dos UAVs @" << Simulator::Now().GetSeconds());
  int id, i = 0;
  double t = 0.0;
  std::ofstream file;
  std::ostringstream osbij, os, osloc, osuav;
  double val;
  for (UavModelContainer::Iterator u_i = m_uavContainer.Begin();
       u_i != m_uavContainer.End(); ++u_i, ++i)
  {
    NS_LOG_INFO("SERVER - UAV "<< (*u_i)->GetId() << " REF " << (*u_i)->GetReferenceCount());
    id = -1;
    val = -1;
    for (unsigned j = 0; j < siz; ++j)
    {
      NS_LOG_INFO("\tLOC \t" << m_locationContainer.Get(j)->toString());
      if (m_ij[i][j] > val && !(m_locationContainer.Get(j)->IsUsed()))
      { // somente aceita a localizacao que nao tenha sido usada por outro UAV
        id = j;
        val = m_ij[i][j];
      }
    } // ao final selecionara a localizacao com maior valor de mij

    NS_LOG_DEBUG ("## ===> UAV " << (*u_i)->GetId() << " to LOC " << id);
    m_locationContainer.Get(id)->SetUsed(); // define como utilizada pelo UAV atual


    NS_LOG_INFO("SERVER - LOC "<< id << "\n\t" << m_locationContainer.Get(id)->toString());

    // Salvando onde o UAV estava e para onde ele será enviado.
    NS_LOG_INFO("SERVER - Salvando onde o UAV estava e para onde ele será enviado");
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

    if (CalculateDistanceCentral(vp)==0.0) { // caso o UAV seja enviado para a central, se faz necessario remover o nó!
      NS_LOG_DEBUG("ServerApplication::runAgendamento --> SERVER @"<< Simulator::Now().GetSeconds() <<"  --- Removendo nó [" << (*u_i)->GetId() << "]");
      m_removeUav((*u_i)->GetId()); // desligando todas as informacoes do nó
      (*u_i)->Dispose(); // parando eventos do UavModel
      m_uavContainer.RemoveAt(i);
      i--;
      u_i--;
    } else {
      NS_LOG_INFO("SERVER - definindo novo posicionamento @" << Simulator::Now().GetSeconds());
      (*u_i)->SetNewPosition(m_locationContainer.Get(id)->GetPosition());
      (*u_i)->NotConfirmed(); // atualiza o valor para identificar se o UAV chegou a posicao correta
      (*u_i)->CancelSendPositionEvent();
      (*u_i)->SetSendPositionEvent(Simulator::Schedule(Seconds(t), &ServerApplication::SendUavPacket, this, (*u_i)));
      t += 0.5;
    }
    vp.clear();
  }

  os.str("");
  os << "./scratch/flynetwork/data/output/"<<m_scenarioName<<"/"<<int(Simulator::Now().GetSeconds())<<"/uav_loc.txt";
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  Vector serv_pos = GetNode()->GetObject<MobilityModel>()->GetPosition();
  file << m_maxx << "," << m_maxy << std::endl << serv_pos.x << "," << serv_pos.y << std::endl << osuav.str() << std::endl << osloc.str() << std::endl << osbij.str() << std::endl;
  file.close();

  NS_LOG_DEBUG ("-- Finalizado posicionamento dos UAVs @" << Simulator::Now().GetSeconds());

  // liberando memoria
  for (unsigned i = 0; i < siz; ++i)
    {
      m_ij[i].clear();
      o_ij[i].clear();
      lamb_ij[i].clear();
      q_ij[i].clear();
      b_ij[i].clear();
      copyB_mij[i].clear();
      custo_x[i].clear();
    }
  m_ij.clear();
  o_ij.clear();
  lamb_ij.clear();
  q_ij.clear();
  b_ij.clear();
  copyB_mij.clear();
  central_pos.clear();
  custo_x.clear();
}

void
ServerApplication::PrintBij (vector<vector<double>> b_ij, int print)
{
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/"<<m_scenarioName<<"/"<<Simulator::Now().GetSeconds()<<"/bij.txt";
  std::ofstream file;
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  UavModelContainer::Iterator u_i = m_uavContainer.Begin();
  file << (*u_i)->GetId();
  u_i++;
  for (; u_i != m_uavContainer.End(); ++u_i)
  {
    file << "," << (*u_i)->GetId();
  }
  file << std::endl;
  LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
  file << (*l_j)->GetId();
  l_j++;
  for (; l_j != m_locationContainer.End(); ++l_j)
  {
    file << "," << (*l_j)->GetId();
  }
  file << std::endl;
  for (vector<vector<double>>::iterator i = b_ij.begin(); i != b_ij.end(); ++i)
  {
    vector<double>::iterator j = (*i).begin();
    file << (*j);
    j++;
    for (; j != (*i).end(); ++j)
    {
      file << "," << (*j);
    }
    file << std::endl;
  }

  file.close();
  for (vector<vector<double>>::iterator i = b_ij.begin(); i != b_ij.end(); ++i)
  {
    (*i).clear();
  }
  b_ij.clear();
}


void
ServerApplication::PrintCusto (vector<vector<double>> custo, int print)
{
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/"<<m_scenarioName<<"/"<<Simulator::Now().GetSeconds()<<"/custo_" << m_custo << ".txt";
  std::ofstream file;
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  UavModelContainer::Iterator u_i = m_uavContainer.Begin();
  file << (*u_i)->GetId();
  u_i++;
  for (; u_i != m_uavContainer.End(); ++u_i)
  {
    file << "," << (*u_i)->GetId();
  }
  file << std::endl;
  LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
  file << (*l_j)->GetId();
  l_j++;
  for (; l_j != m_locationContainer.End(); ++l_j)
  {
    file << "," << (*l_j)->GetId();
  }
  file << std::endl;
  for (vector<vector<double>>::iterator i = custo.begin(); i != custo.end(); ++i) {
    vector<double>::iterator j = (*i).begin();
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

  for (vector<vector<double>>::iterator i = custo.begin(); i != custo.end(); ++i)
  {
    (*i).clear();
  }
  custo.clear();
}

void
ServerApplication::PrintMij (vector<vector<double>> m_ij, int print, double temp)
{
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/"<<m_scenarioName<<"/" << int(Simulator::Now().GetSeconds()) << "/mij_" << std::setfill ('0') << std::setw (7) << print << ".txt";
  std::ofstream file;
  file.open(os.str().c_str(), std::ofstream::out | std::ofstream::app);
  file << temp << std::endl;
  UavModelContainer::Iterator u_i = m_uavContainer.Begin();
  file << (*u_i)->GetId();
  u_i++;
  for (; u_i != m_uavContainer.End(); ++u_i)
  {
    file << "," << (*u_i)->GetId();
  }
  file << std::endl;
  LocationModelContainer::Iterator l_j = m_locationContainer.Begin();
  file << (*l_j)->GetId();
  l_j++;
  for (; l_j != m_locationContainer.End(); ++l_j)
  {
    file << "," << (*l_j)->GetId();
  }
  file << std::endl;
  for (vector<vector<double>>::iterator i = m_ij.begin(); i != m_ij.end(); ++i)
  {
    vector<double>::iterator j = (*i).begin();
    file << (*j);
    j++;
    for (; j != (*i).end(); ++j)
    {
      file << "," << (*j);
    }
    file << std::endl;
  }
  file.close();
  for (vector<vector<double>>::iterator i = m_ij.begin(); i != m_ij.end(); ++i)
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

} // namespace ns3
