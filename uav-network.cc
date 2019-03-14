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

#include "uav-network.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/wifi-radio-energy-model.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/dsr-module.h"

#include "uav-energy-source-helper.h"
#include "uav-device-energy-model-helper.h"
#include "client-device-energy-model-helper.h"
#include "uav-model.h"
#include "smartphone-application.h"
// #include "dhcp-helper-uav.h"
// #include "dhcp-server-uav.h"

#include <fstream>
#include <cstdlib>
#include <string>

#define ETAPA 300

// permite os clientes enviarem aplicacoes para o servidor
#define APP_CLI

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavNetwork");

NS_OBJECT_ENSURE_REGISTERED(UavNetwork);

TypeId
UavNetwork::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::UavNetwork")
                          .SetParent<Object>()
                          .SetGroupName("Flynetwork-Main")
                          .AddConstructor<UavNetwork>()
                          .AddAttribute("Frequency",
                                        "Frequency UAV communication",
                                        DoubleValue(5.15),
                                        MakeDoubleAccessor(&UavNetwork::m_frequency),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("PowerLevel",
                                        "Transmitter power level",
                                        DoubleValue(10.0),
                                        MakeDoubleAccessor(&UavNetwork::m_powerLevel),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("RxGain",
                                        "Receiver gain",
                                        DoubleValue(3.0),
                                        MakeDoubleAccessor(&UavNetwork::m_rxGain),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("ZValue",
                                        "Z value",
                                        DoubleValue(10.0),
                                        MakeDoubleAccessor(&UavNetwork::m_zValue),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("ScheduleServer", "Time to schedule server method.",
                                        DoubleValue(ETAPA),
                                        MakeDoubleAccessor(&UavNetwork::m_scheduleServer),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("TxGain",
                                        "Transmitter gain",
                                        DoubleValue(3.0),
                                        MakeDoubleAccessor(&UavNetwork::m_txGain),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("SimulationTime",
                                        "Simulation time",
                                        DoubleValue(300.0),
                                        MakeDoubleAccessor(&UavNetwork::m_simulationTime),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("PropagationLossCli",
                                        "Client propagation loss",
                                        StringValue("ns3::TwoRayGroundPropagationLossModel"),
                                        MakeStringAccessor(&UavNetwork::m_propagationLossCli),
                                        MakeStringChecker())
                          .AddAttribute("ServerPort",
                                        "Communication port number of server",
                                        UintegerValue(8082),
                                        MakeUintegerAccessor(&UavNetwork::m_serverPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("ClientPort",
                                        "Communication port number of client",
                                        UintegerValue(8080),
                                        MakeUintegerAccessor(&UavNetwork::m_cliPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("LocationUpdateCli",
                                        "Time to update the location of the client on the server.",
                                        DoubleValue(3.0),
                                        MakeDoubleAccessor(&UavNetwork::m_updateTimeCli),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("SpeedUav",
                                        "Speed of the UAV node.",
                                        DoubleValue(5.0),
                                        MakeDoubleAccessor(&UavNetwork::m_speedUav),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("LocationUpdateUav",
                                        "Time to update the location of the UAV on the server.",
                                        DoubleValue(15.0),
                                        MakeDoubleAccessor(&UavNetwork::m_updateTimeUav),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("Environment",
                                        "The environment use to simulate de DA.",
                                        UintegerValue(2),
                                        MakeUintegerAccessor(&UavNetwork::m_environment),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Custo",
                                        "Custo, metrica de análise do Da de posicionamento.",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&UavNetwork::m_custo),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Protocol",
                                        "Protocol.",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&UavNetwork::m_protocol),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("Scenario",
                                        "Minimum x of the scenario",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&UavNetwork::m_scenario),
                                        MakeUintegerChecker<uint32_t>());
  return tid;
}

UavNetwork::UavNetwork() //: m_filePacketServer("./scratch/flynetwork/data/output/packet_trace_server.txt"), m_filePacketUav("./scratch/flynetwork/data/output/packet_trace_uav.txt"), m_filePacketClient("./scratch/flynetwork/data/output/packet_trace_client.txt")
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_iniX = m_iniY = -2000;
}

void UavNetwork::DoDispose ()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_INFO ("UavNetwork::DoDispose REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
  m_palcoPos.clear();
  m_uavNode.Clear();
  m_uavNodeActive.Clear();
  m_uavAppContainer.Clear();
  m_serverApp->Unref();
  m_positionAlloc->Unref();
}

UavNetwork::~UavNetwork()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void UavNetwork::Run()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );

  std::ifstream scenario;
  std::ostringstream ss, ss_;
  switch (m_scenario)
  {
    case 0: // teste
      ss << "teste";
      break;
    case 1:
      ss << "lollapalooza_30";
      break;
    case 2: //Lollapalooza
      ss << "lollapalooza_50";
      break;
    case 3:
      ss << "lollapalooza_100";
      break;
    case 4: //Rock in Rio Lisboa
      ss << "rockinriolisboa_30";
      break;
    case 5: 
      ss << "rockinriolisboa_50";
      break;
    case 6: 
      ss << "rockinriolisboa_100";
      break;
    case 7: // austin_30
      ss << "austin_30";
      break;
    case 8: // austin
      ss << "austin_50";
      break;
    case 9: // austin
      ss << "austin_100";
      break;
    default:
      NS_LOG_ERROR("Não foi possivel identificar o cenario!");
      exit(-1);
  }
  m_scenarioName = ss.str();
  // ler informacoes do arquivo
  m_PathData = ss.str();
  ss_ << "./scratch/flynetwork/data/scenarios/" << m_PathData << ".txt";
  scenario.open(ss_.str());
  if (scenario.is_open())
  {
    std::string line;
    getline(scenario, line);
    sscanf(line.c_str(), "%d\n", &m_totalCli);
    m_xmin = m_ymin = 0;
    getline(scenario, line);
    sscanf(line.c_str(), "%lf,%lf\n", &m_xmax, &m_ymax);
    getline(scenario, line);
    sscanf(line.c_str(), "%lf,%lf\n", &m_cx, &m_cy);
    scenario.close();
  }
  else
  {
    string s = ss_.str();
    ss_.str("");
    ss_ << "Não foi possível abrir o arquivo: " << s;
    NS_LOG_ERROR(ss_.str().c_str());
    exit(-1);
  }

  ss << "/custo_" << m_custo;
  m_pathData = ss.str();
  ss.str("");
  ss << "rm -Rf ./scratch/flynetwork/data/output/" << m_pathData;
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData;
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/course_changed";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_remaining_energy";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_hover";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_move";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_energy_threshold";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/etapa";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_recharged";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_depletion";
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_client";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/uav_stop";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/python";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/dhcp";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/client";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p ./scratch/flynetwork/data/output/" << m_pathData << "/compare";
  system(ss.str().c_str());
  // ss.str("");
  // ss << "./scratch/flynetwork/data/output/"<< m_pathData<<"/packet_trace_server.txt";
  // m_filePacketServer = ss.str().c_str();
  // ss.str("");
  // ss << "./scratch/flynetwork/data/output/"<<m_pathData<<"/packet_trace_uav.txt";
  // m_filePacketUav = ss.str().c_str();
  // ss.str("");
  // ss << "./scratch/flynetwork/data/output/"<<m_pathData<<"/packet_trace_client.txt";
  // m_filePacketClient = ss.str().c_str();

  // configure variables
  NS_LOG_INFO("Configurando variaveis");
  Configure();

  // Configure Server
  NS_LOG_INFO("Configurando Servidor");
  ConfigureServer();

  // Configure Client
  NS_LOG_INFO("Configurando Palcos");
  ConfigurePalcos();

  // Configure UAV
  NS_LOG_INFO("Configurando UAV");
  ConfigureUav(5);

  // Configure Client
  NS_LOG_INFO("Configurando Cliente");
  ConfigureCli();

  Simulator::Stop(Seconds(m_simulationTime));
  NS_LOG_DEBUG("Iniciando Simulador");
  Simulator::Run();
  NS_LOG_DEBUG("Finalizando Simulador");
  Simulator::Destroy();
  NS_LOG_DEBUG("Finalizando Destroy");

  // m_filePacketServer.close(); // fechando arquivo de trace (pacotes)
  // m_filePacketUav.close();    // fechando arquivo de trace (pacotes)
  // m_filePacketClient.close(); // fechando arquivo de trace (pacotes)

}

void UavNetwork::ConfigureServer()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_serverNode.Create(1);

  /// Net device
  NetDeviceContainer adhoc = m_adhocHelper.Install(m_phyHelper, m_macAdHocHelper, m_serverNode);

  // configurando internet
  m_stack.Install(m_serverNode);

  // configurando pilha de protocolos
  m_serverAddress.Add(m_addressHelper.Assign(adhoc));

  // configurando mobilidade na central
  Ptr<ConstantPositionMobilityModel> serverMobility = CreateObject<ConstantPositionMobilityModel>();
  serverMobility->SetPosition(Vector(m_cx, m_cy, m_zValue));
  m_serverNode.Get(0)->AggregateObject(serverMobility);

  std::cout << "Server Address: " << m_serverAddress.GetAddress(0) << std::endl;

  // configurando PacketSink
  ObjectFactory packFacAdhoc;
  packFacAdhoc.SetTypeId ("ns3::PacketSink");
  packFacAdhoc.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
  packFacAdhoc.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), m_serverPort))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appAdhoc = packFacAdhoc.Create<Application> ();
  appAdhoc->SetStartTime(Seconds(10.0));
  appAdhoc->SetStopTime(Seconds(m_simulationTime));
  m_serverNode.Get(0)->AddApplication (appAdhoc);

  ObjectFactory packVoice;
  packVoice.SetTypeId ("ns3::PacketSink");
  packVoice.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
  packVoice.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), 5060))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appVoice = packVoice.Create<Application> ();
  appVoice->SetStartTime(Seconds(10.0));
  appVoice->SetStopTime(Seconds(m_simulationTime));
  m_serverNode.Get(0)->AddApplication (appVoice);

  ObjectFactory packVideo;
  packVideo.SetTypeId ("ns3::PacketSink");
  packVideo.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
  packVideo.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), 5070))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appVideo = packVideo.Create<Application> ();
  appVideo->SetStartTime(Seconds(10.0));
  appVideo->SetStopTime(Seconds(m_simulationTime));
  m_serverNode.Get(0)->AddApplication (appVideo);

  ObjectFactory packWww;
  packWww.SetTypeId ("ns3::PacketSink");
  packWww.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
  packWww.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), 8080))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appWww = packWww.Create<Application> ();
  appWww->SetStartTime(Seconds(10.0));
  appWww->SetStopTime(Seconds(m_simulationTime));
  m_serverNode.Get(0)->AddApplication (appWww);

  // create server app
  ObjectFactory obj;
  obj.SetTypeId("ns3::ServerApplication");
  obj.Set("DataRate", DataRateValue(DataRate("2Mbps")));
  obj.Set("Environment", UintegerValue(m_environment));
  obj.Set("Ipv4Address", Ipv4AddressValue(m_serverAddress.GetAddress(0)));
  obj.Set("ServerPort", UintegerValue(m_serverPort));
  obj.Set("ClientPort", UintegerValue(m_cliPort));
  obj.Set("MaxX", DoubleValue(m_xmax));
  obj.Set("MaxY", DoubleValue(m_ymax));
  obj.Set("PathData", StringValue(m_pathData));
  obj.Set("ScenarioName", StringValue(m_scenarioName));
  obj.Set("ScheduleServer", DoubleValue(m_scheduleServer));
  obj.Set("Custo", UintegerValue(m_custo));

  m_serverApp = obj.Create()->GetObject<ServerApplication>();
  m_serverApp->SetStartTime(Seconds(0.0));
  m_serverApp->SetStopTime(Seconds(m_simulationTime));

  m_serverApp->TraceConnectWithoutContext("NewUav", MakeCallback(&UavNetwork::NewUav, this));// adicionando callback para criar UAVs
  m_serverApp->TraceConnectWithoutContext("PrintUavEnergy", MakeCallback(&UavNetwork::PrintUavEnergy, this));
  m_serverApp->TraceConnectWithoutContext("RemoveUav", MakeCallback(&UavNetwork::RemoveUav, this));
  // m_serverApp->TraceConnectWithoutContext("PacketTrace", MakeCallback(&UavNetwork::PacketServer, this));
  m_serverApp->TraceConnectWithoutContext("ClientPositionTrace", MakeCallback(&UavNetwork::ClientPosition, this));

  // trace do PacketSink RX
  appAdhoc->TraceConnectWithoutContext ("Rx", MakeCallback (&ServerApplication::TracedCallbackRxApp, m_serverApp));
  appVoice->TraceConnectWithoutContext ("Rx", MakeCallback (&ServerApplication::TracedCallbackRxApp, m_serverApp));
  appVideo->TraceConnectWithoutContext ("Rx", MakeCallback (&ServerApplication::TracedCallbackRxApp, m_serverApp));
  appWww->TraceConnectWithoutContext ("Rx", MakeCallback (&ServerApplication::TracedCallbackRxApp, m_serverApp));

  // aggregate to node
  m_serverNode.Get(0)->AddApplication(m_serverApp);
}

void UavNetwork::NewUav(int total, int update)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<total<<update);
  NS_LOG_DEBUG ("UavNetwork::NewUav " << total << " " << update << " @" << Simulator::Now().GetSeconds());
  // validar se ainda existem UAVs
  uint32_t uav_livre = 0;
  NS_LOG_DEBUG("Uavs: (" << m_iniX << "," << m_iniY <<")");
  for (UavNodeContainer::Iterator i = m_uavNode.Begin(); i != m_uavNode.End(); ++i) {
    Ptr<MobilityModel> mob = (*i)->GetObject<MobilityModel>();
    NS_LOG_DEBUG("\t"<<(*i)->GetId() << " (" << mob->GetPosition().x << "," << mob->GetPosition().y << ")");
    if (mob->GetPosition().x == m_iniX && mob->GetPosition().y == m_iniY) { // somente se estiver na "posicao inicial"
      uav_livre++;
    }
  }
  NS_LOG_DEBUG ("Uav livre: " << uav_livre);
  if (uav_livre < uint32_t(total)) { // Caso nao, configurar um novo
    ConfigureUav(total - uav_livre); // diferenca
  }
  while (total--) {
    int p = -1;
    Ptr<MobilityModel> mob = 0;
    do {
      mob = m_uavNode.Get(++p)->GetObject<MobilityModel>();
    } while (!(mob->GetPosition().x == m_iniX && mob->GetPosition().y == m_iniY)); // somente se estiver na "posicao inicial"

    NS_LOG_DEBUG("Id " << m_uavNode.Get(p)->GetId() << " REF " << m_uavNode.Get(p)->GetReferenceCount());
    Ptr<Node> n = m_uavNode.RemoveAt(p);

    NS_LOG_DEBUG("->REF " << n->GetReferenceCount());
    m_uavNodeActive.Add(n);

    // modificando posicionamento
    Vector v = m_positionAlloc->GetNext();
    v.z = m_zValue;
    Ptr<UavMobilityModel> model = n->GetObject<UavMobilityModel>();
    model->SetFirstPosition(v); // manda para perto da central!

    // Aumentar carga de bateria para máximo!
    Ptr<UavEnergySource> source = DynamicCast<UavEnergySource>(n->GetObject<UavDeviceEnergyModel>()->GetEnergySource());
    source->Reset(); // recarregando

    // Start application
    NS_LOG_DEBUG ("Apps " << n->GetNApplications());
    int app = n->GetNApplications()-1;
    Ptr<UavApplication> uavApp = NULL;
    do {
      uavApp = DynamicCast<UavApplication>(n->GetApplication(app));
      --app;
    } while (uavApp == NULL && app >= 0);
    NS_ASSERT (uavApp != NULL);
    uavApp->Start(m_simulationTime);
    uavApp->Reset(); // atualizando depletion=false e atualizando goto

    // Adicionando informacoes na aplicacao servidor!
    Ipv4Address addr = n->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
    NS_LOG_DEBUG("IP " << addr << " -------------");
    if (update == 1 || update == 2) {
      // envia ao servidor informacoes do UAV substituto
      m_serverApp->AddSupplyUav(n->GetId(), addr, source->GetRemainingEnergy(), n->GetObject<UavDeviceEnergyModel>()->GetEnergyCost(), n->GetObject<UavDeviceEnergyModel>()->GetHoverCost(), source->GetInitialEnergy(), n->GetObject<MobilityModel>());
    } else { // adiciona um novo UAV no servidor
      m_serverApp->AddNewUav(n->GetId(), addr, source->GetRemainingEnergy(), n->GetObject<UavDeviceEnergyModel>()->GetEnergyCost(), n->GetObject<UavDeviceEnergyModel>()->GetHoverCost(), source->GetInitialEnergy(), n->GetObject<MobilityModel>()); // tell the server to create a new model of UAV, used to identify the actual location of those UAV nodes
    }

    std::ostringstream os;
    os << "./scratch/flynetwork/data/output/" << m_pathData << "/uav_network_log.txt";
    m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
    if (update == 2) {
      m_file << Simulator::Now().GetSeconds() << "," << n->GetId() << ",2" << std::endl;
    } else  {
      m_file << Simulator::Now().GetSeconds() << "," << n->GetId() << ",1" << std::endl;
    }
    m_file.close();

    n = 0;
    NS_LOG_DEBUG (" ------------------------------------- ");
  }
  NS_LOG_DEBUG("Id " << m_uavNodeActive.Get(m_uavNodeActive.GetN()-1)->GetId() << " REF " << m_uavNodeActive.Get(m_uavNodeActive.GetN()-1)->GetReferenceCount() << " ------------");
}

void UavNetwork::RemoveUav(int id, int step)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<id);
  NS_LOG_DEBUG ("UavNetwork::RemoveUav [" << id << "]");
  Ptr<Node> n = m_uavNodeActive.RemoveId(id);
  m_uavNode.Add(n);

  // Stop application
  NS_LOG_DEBUG ("Apps " << n->GetNApplications());
  int app = n->GetNApplications()-1;
  Ptr<UavApplication> uavApp = NULL;
  do {
    uavApp = DynamicCast<UavApplication>(n->GetApplication(app));
    --app;
  } while (uavApp==NULL && app >= 0);
  NS_ASSERT (uavApp != NULL);
  uavApp->Stop();

  // modificando posicionamento para fora do cenario
  Vector v (m_iniX, m_iniY,0);
  Ptr<UavMobilityModel> model = n->GetObject<UavMobilityModel>();
  model->SetFirstPosition(v); // manda para perto da central!

  // Parar Uav
  Ptr<UavEnergySource> source = DynamicCast<UavEnergySource>(n->GetObject<UavDeviceEnergyModel>()->GetEnergySource());
  source->Stop(); // recarregando

  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/uav_network_log.txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << n->GetId() << ",0" << std::endl;
  m_file.close();

  os.str("");
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << step << "/uav_removed_energy.txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  Ptr<UavDeviceEnergyModel> dev = n->GetObject<UavDeviceEnergyModel>();
  m_file << dev->GetEnergySource()->GetRemainingEnergy() / dev->GetEnergySource()->GetInitialEnergy() << std::endl;
  m_file.close();

  n = 0;
}

void UavNetwork::ConfigureUav(int total)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<total);
  NodeContainer uav;
  uav.Create(total);
  m_uavNode.Add(uav);

  // configura comunicação adhoc
  NetDeviceContainer adhoc = m_adhocHelper.Install(m_phyHelper, m_macAdHocHelper, uav);

  // configura comunicação wifi
  NetDeviceContainer wifi = m_wifiHelper.Install(m_phyHelperCli, m_macWifiHelper, uav);
  // configurando internet
  m_stack.Install(uav);

  // configurando pilha de protocolos
  Ipv4InterfaceContainer addContainer(m_addressHelper.Assign(adhoc));

  // install mobility -- and static Routing
  Ipv4StaticRoutingHelper routingHelper;

  for (NodeContainer::Iterator i = uav.Begin(); i != uav.End(); ++i)
  {
    ObjectFactory objFacMobUAV;
    objFacMobUAV.SetTypeId("ns3::UavMobilityModel");
    objFacMobUAV.Set("Speed", DoubleValue(m_speedUav));
    Vector v(m_iniX, m_iniY,0); // longe, bem longe! Para nao influenciar nos resultados!
    objFacMobUAV.Set("FirstPosition", VectorValue(v));// modificando posicionamento inicial do UAV
    Ptr<MobilityModel> model = objFacMobUAV.Create()->GetObject<MobilityModel>();
    Ptr<Object> object = (*i);
    object->AggregateObject(model);
  }

  /** Energy Model **/
  /* energy source */
  UavEnergySourceHelper sourceHelper;
  sourceHelper.Set("PathData", StringValue(m_pathData));
  // install source
  EnergySourceContainer sources = sourceHelper.Install(uav);

  /* uav device energy model */
  UavDeviceEnergyModelHelper energyHelper;
  energyHelper.Set("AverageVelocity", DoubleValue(18)); // m/s
  energyHelper.Set("ResistTime", DoubleValue(27*60)); // s
  energyHelper.Set("PathData", StringValue(m_pathData));
  energyHelper.Set("xCentral", DoubleValue(m_cx)); // utilizado para calcular o threshold dinamicamente
  energyHelper.Set("yCentral", DoubleValue(m_cy));
  // install device model
  DeviceEnergyModelContainer uavEnergyModels = energyHelper.Install(uav, sources);

  /* client device energy model*/
  ClientDeviceEnergyModelHelper cliHelper;
  DeviceEnergyModelContainer cliEnergyModels = cliHelper.Install(uav, sources);

  /* device energy model */
  // WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  // radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174));
  // install device model
  // DeviceEnergyModelContainer deviceModelsWifi = radioEnergyHelper.Install (wifi, sources);
  // install device model
  // DeviceEnergyModelContainer deviceModelsAdhoc = radioEnergyHelper.Install (adhoc, sources);

  // create and configure UAVApp and Sink application
  int c = 0;
  std::ostringstream oss, poolAddr, minAddr, maxAddr, serverAddr;
  for (NodeContainer::Iterator i = uav.Begin(); i != uav.End(); ++i, ++c)
  {
    // configure PacketSink
    ObjectFactory packFac;
    packFac.SetTypeId ("ns3::PacketSink");
    packFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
    packFac.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), m_serverPort))); // OPS: utilizam Ipv4Address::GetAny ()
    Ptr<Application> app = packFac.Create<Application> ();
    app->SetStartTime(Seconds(0.0));
    app->SetStopTime(Seconds(m_simulationTime));
    (*i)->AddApplication (app);

    ObjectFactory packFacInfra;
    packFacInfra.SetTypeId ("ns3::PacketSink");
    packFacInfra.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
    packFacInfra.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), m_cliPort))); // OPS: utilizam Ipv4Address::GetAny ()
    Ptr<Application> appInfra = packFacInfra.Create<Application> ();
    appInfra->SetStartTime(Seconds(0.0));
    appInfra->SetStopTime(Seconds(m_simulationTime));
    (*i)->AddApplication (appInfra);

    ObjectFactory obj; // create UavApplication
    obj.SetTypeId("ns3::UavApplication");
    obj.Set("Id", UintegerValue((*i)->GetId()));
    Ptr<UniformRandomVariable> uptime = CreateObject<UniformRandomVariable>();
    obj.Set("UpdateTime", DoubleValue(m_updateTimeUav+uptime->GetValue()));
    obj.Set("Remote", Ipv4AddressValue(m_serverAddress.GetAddress(0)));
    obj.Set("DataRate", DataRateValue(DataRate("11Mbps")));
    obj.Set("AdhocAddress", Ipv4AddressValue(addContainer.GetAddress(c)));
    obj.Set("ServerPort", UintegerValue(m_serverPort));
    obj.Set("ClientPort", UintegerValue(m_cliPort));
    obj.Set("PathData", StringValue(m_pathData));

    std::cout << "Uav #" << (*i)->GetId() << " IP " << addContainer.GetAddress(c) << std::endl;

    Ptr<UavApplication> uavApp = obj.Create()->GetObject<UavApplication>();
    uavApp->SetStartTime(Seconds(0.0));
    uavApp->SetStopTime(Seconds(0.01));

    // uavApp->TraceConnectWithoutContext("PacketTrace", MakeCallback(&UavNetwork::PacketUav, this));
    Ptr<WifiNetDevice> wifiDeviceAdhoc = DynamicCast<WifiNetDevice> (adhoc.Get(c));
    Ptr<WifiPhy> wifiPhyAdhoc = wifiDeviceAdhoc->GetPhy ();
    Ptr<WifiNetDevice> wifiDeviceInfra = DynamicCast<WifiNetDevice> (wifi.Get(c));
    Ptr<WifiPhy> wifiPhyInfra = wifiDeviceInfra->GetPhy ();
    // uavApp->SetTurnOffWifiPhyCallback(MakeCallback(&WifiPhy::SetOffMode, wifiPhyAdhoc),MakeCallback(&WifiPhy::SetOffMode, wifiPhyInfra));
    // // trace do PacketSink RX
    app->TraceConnectWithoutContext ("Rx", MakeCallback (&UavApplication::TracedCallbackRxApp, uavApp));
    appInfra->TraceConnectWithoutContext ("Rx", MakeCallback (&UavApplication::TracedCallbackRxAppInfra, uavApp));

    // aggregate to the node
    (*i)->AddApplication(uavApp);

    // Configure TotalEnergyConsumption
    // deviceModelsWifi.Get(c)->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback(&UavApplication::TotalEnergyConsumptionTrace,  uavApp));

    // Mobility
    (*i)->GetObject<MobilityModel>()->TraceConnectWithoutContext ("CourseChange", MakeCallback (&UavApplication::CourseChange, uavApp));
    Ptr<UavDeviceEnergyModel> dev = (*i)->GetObject<UavDeviceEnergyModel>();
    DynamicCast<UavMobilityModel>((*i)->GetObject<MobilityModel>())->TraceConnectWithoutContext ("CourseChangeDevice", MakeCallback (&UavDeviceEnergyModel::CourseChange, dev));

    // energy start
    Ptr<UavEnergySource> source = DynamicCast<UavEnergySource>((*i)->GetObject<UavDeviceEnergyModel>()->GetEnergySource());
    source->Start();

    // Configure DHCP
    // The router must have a fixed IP.
    poolAddr.str(""); minAddr.str(""); maxAddr.str(""); serverAddr.str("");
    serverAddr << "192.168." << (*i)->GetId() << ".1";
    minAddr << "192.168." << (*i)->GetId() << ".2";
    maxAddr << "192.168." << (*i)->GetId() << ".254";
    poolAddr << "192.168." << (*i)->GetId() << ".0";
    DhcpHelper dhcpHelper;
    Ipv4InterfaceContainer fixedNodes = dhcpHelper.InstallFixedAddress (wifi.Get (c), Ipv4Address (serverAddr.str().c_str()), Ipv4Mask ("/24"));
    // Not really necessary, IP forwarding is enabled by default in IPv4.
    fixedNodes.Get (0).first->SetAttribute ("IpForward", BooleanValue (true));
    // DHCP server
    ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer (wifi.Get (c),
                        Ipv4Address (serverAddr.str().c_str()),
                        Ipv4Address (poolAddr.str().c_str()), Ipv4Mask ("/24"),
                        Ipv4Address (minAddr.str().c_str()), Ipv4Address (maxAddr.str().c_str()),
                        Ipv4Address (serverAddr.str().c_str()));
    dhcpServerApp.Get(0)->TraceConnectWithoutContext ("TotalLeased", MakeCallback(&UavApplication::TotalLeasedTrace, uavApp));
    dhcpServerApp.Start (Seconds (0.0));
    dhcpServerApp.Stop (Seconds(m_simulationTime));
    m_uavAppContainer.Add(uavApp); // armazenando informacoes das aplicacoes dos UAVs para que os clientes possam obter informacoes necessarias para se conectar no UAV mais proximo!
  }
}

void UavNetwork::ConfigureCli()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  if (m_totalCli == 0)
    return;

  NetDeviceContainer devices;
  InternetStackHelper stack;

  std::ostringstream ss;
  std::ifstream scenario;
  ss << "./scratch/flynetwork/data/scenarios/" << m_PathData << ".txt";
  scenario.open(ss.str());
  // ler informacoes dos arquivos
  if (scenario.is_open())
  {
    double x, y;
    std::string line;
    getline(scenario, line);
    getline(scenario, line);
    getline(scenario, line); // liberando as primeiras informacoes
    int update_total = 0;
    Ptr<UniformRandomVariable> app_rand = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
    app_rand->SetAttribute ("Min", DoubleValue (1));
    app_rand->SetAttribute ("Max", DoubleValue (m_totalCli));
    while (getline(scenario, line))
    {
      if (line.at(0) != '#')
      {
        sscanf(line.c_str(), "%lf,%lf\n", &x, &y);
        NS_LOG_INFO(line.c_str());
        NodeContainer nodes;
        int t = app_rand->GetValue();
        nodes.Create(t);
        update_total += t;
        devices.Add(m_wifiHelper.Install(m_phyHelperCli, m_macWifiHelperCli, nodes));
        stack.Install(nodes);
        MobilityHelper mobilityCLI;
        Ptr<PositionAllocator> positionAlloc = CreateObjectWithAttributes<RandomDiscPositionAllocator>
                            ("X", DoubleValue (x),
                              "Y", DoubleValue (y),
                            "Rho", StringValue("ns3::ConstantRandomVariable[Constant=8.0]"));
        mobilityCLI.SetPositionAllocator(positionAlloc);
        mobilityCLI.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                     "Bounds", RectangleValue(Rectangle(m_xmin, m_xmax, m_ymin, m_ymax)),
                                      "Speed", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=10.0]")); // xmin, xmax, ymin, ymax
        mobilityCLI.Install(nodes);
        m_clientNode.Add(nodes);
      }
    }
    scenario.close();
    m_totalCli = update_total;
  }
  else
  {
    string s = ss.str();
    ss.str("");
    ss << "Não foi possível abrir o arquivo: " << s;
    NS_LOG_ERROR(ss.str().c_str());
    exit(-1);
  }

  Ptr<UniformRandomVariable> app_rand = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
  app_rand->SetAttribute ("Min", DoubleValue (0));
  app_rand->SetAttribute ("Max", DoubleValue (3)); // MODIFICADO

  std::ofstream cliLogin;
  ss << "./scratch/flynetwork/data/output/" << m_pathData << "/client_login.txt";
  cliLogin.open(ss.str().c_str());
  
  // aggregate SmartphoneApp on node and configure it!
  Ptr<UniformRandomVariable> e_ai = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
  int c = 0;
  std::ostringstream oss;
  for (NodeContainer::Iterator i = m_clientNode.Begin(); i != m_clientNode.End(); ++i, ++c)
  {
    ss.str("");
    ss << "login-" << (*i)->GetId();

    // configure PacketSink
    ObjectFactory packFac;
    packFac.SetTypeId ("ns3::PacketSink");
    packFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
    packFac.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), m_cliPort))); // OPS: utilizam Ipv4Address::GetAny ()
    Ptr<Application> app = packFac.Create<Application> ();
    app->SetStartTime(Seconds(0.0));
    app->SetStopTime(Seconds(m_simulationTime));
    (*i)->AddApplication (app);

    DhcpHelper dhcpHelper;
    ApplicationContainer dhcpClients = dhcpHelper.InstallDhcpClient (devices.Get(c));
    dhcpClients.Start (Seconds (10.0));
    dhcpClients.Stop (Seconds(m_simulationTime));
    uint32_t id = (*i)->GetNApplications()-1;

    // configurando Smartphone
    ObjectFactory factory; // create SmartphoneApplication
    factory.SetTypeId("ns3::SmartphoneApplication");
    factory.Set("Id", UintegerValue((*i)->GetId()));
    factory.Set("Login", StringValue(ss.str()));
    factory.Set("Start", DoubleValue(e_ai->GetValue()));
    factory.Set("PacketSink", PointerValue(StaticCast<PacketSink> (app)));
    factory.Set("ChangePosition", DoubleValue(5.0)); // maximo de movimentacao para notificar o servidor
    factory.Set("DataRate", DataRateValue(DataRate("11Mbps")));
    factory.Set("Port", UintegerValue(m_cliPort));
    factory.Set("IdDhcp", UintegerValue(id));
    factory.Set("PathData", StringValue(m_pathData));

    Ptr<SmartphoneApplication> smart = factory.Create()->GetObject<SmartphoneApplication>();
    smart->SetStartTime(Seconds(10.0));
    smart->SetStopTime(Seconds(m_simulationTime));

    // smart->TraceConnectWithoutContext("PacketTrace", MakeCallback(&UavNetwork::PacketClient, this));

    (*i)->GetObject<MobilityModel>()->TraceConnectWithoutContext("CourseChange", MakeCallback(&SmartphoneApplication::CourseChange, smart));

    Ptr<WifiNetDevice> d = DynamicCast<WifiNetDevice>(devices.Get(c));
    StaticCast<StaWifiMac>(d->GetMac())->TraceConnectWithoutContext("Assoc", MakeCallback(&SmartphoneApplication::TracedCallbackAssocLogger, smart));
    StaticCast<StaWifiMac>(d->GetMac())->TraceConnectWithoutContext("DeAssoc", MakeCallback(&SmartphoneApplication::TracedCallbackDeAssocLogger, smart));

    // DynamicCast<DhcpClient> (dhcpClients.Get (c))->TraceConnectWithoutContext("NewLease", MakeCallback(&SmartphoneApplication::TracedCallbackNewLease, smart));
    (*i)->GetApplication(id)->TraceConnectWithoutContext("NewLease", MakeCallback(&SmartphoneApplication::TracedCallbackNewLease, smart));
    (*i)->GetApplication(id)->TraceConnectWithoutContext("ExpireLease", MakeCallback(&SmartphoneApplication::TracedCallbackExpiryLease, smart));

    // configurando trace Packetsink
    // app->TraceConnectWithoutContext ("Rx", MakeCallback (&SmartphoneApplication::TracedCallbackRxApp, smart));
    // appOnOff->TraceConnectWithoutContext ("TxWithAddresses", MakeCallback (&SmartphoneApplication::TracedCallbackTxApp, smart));

    // aggregate to the node
    (*i)->AddApplication(smart);

    #ifdef APP_CLI
      // configure OnOff application para server
      int app_code = app_rand->GetValue();
      int port = 0;
      ObjectFactory onoffFac;
      Ptr<Application> appOnOff = 0;
      if (app_code < 1) { // VOICE
          smart->SetApp ("VOICE");
          onoffFac.SetTypeId ("ns3::OnOffApplication");
          onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
          onoffFac.Set ("PacketSize", UintegerValue (50));
          onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
          onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          // P.S.: offTime + DataRate/PacketSize = next packet time
          onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.024Mbps")));
          port = 5060;
          onoffFac.Set ("Remote", AddressValue (InetSocketAddress (m_serverAddress.GetAddress(0), port)));
          appOnOff = onoffFac.Create<Application> ();
          appOnOff->SetStartTime(Seconds(10.0));
          appOnOff->SetStopTime(Seconds(m_simulationTime));
          (*i)->AddApplication (appOnOff);
          cliLogin << ss.str() << " VOICE" << std::endl;
      } else if (app_code < 2) { // VIDEO
          smart->SetApp ("VIDEO");
          onoffFac.SetTypeId ("ns3::OnOffApplication");
          onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
          onoffFac.Set ("PacketSize", UintegerValue (429));
          onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
          onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          // P.S.: offTime + DataRate/PacketSize = next packet time
          onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.128Mbps")));
          port = 5070;
          onoffFac.Set ("Remote", AddressValue (InetSocketAddress (m_serverAddress.GetAddress(0), port)));
          appOnOff = onoffFac.Create<Application> ();
          appOnOff->SetStartTime(Seconds(10.0));
          appOnOff->SetStopTime(Seconds(m_simulationTime));
          (*i)->AddApplication (appOnOff);
          cliLogin << ss.str() << " VIDEO" << std::endl;
      } else if (app_code < 3) { // WWW
          smart->SetApp ("WWW");
          onoffFac.SetTypeId ("ns3::OnOffApplication");
          onoffFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
          onoffFac.Set ("PacketSize", UintegerValue (429));
          onoffFac.Set ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=120]"));
          onoffFac.Set ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.04]"));
          // P.S.: offTime + DataRate/PacketSize = next packet time
          onoffFac.Set ("DataRate", DataRateValue (DataRate ("0.128Mbps")));
          port = 8080;
          onoffFac.Set ("Remote", AddressValue (InetSocketAddress (m_serverAddress.GetAddress(0), port)));
          appOnOff = onoffFac.Create<Application> ();
          appOnOff->SetStartTime(Seconds(10.0));
          appOnOff->SetStopTime(Seconds(m_simulationTime));
          (*i)->AddApplication (appOnOff);
          cliLogin << ss.str() << " WWW" << std::endl;
      } else if (app_code < 4) { // NOTHING
          smart->SetApp ("NOTHING");
          cliLogin << ss.str() << " NOTHING" << std::endl;
      } else NS_FATAL_ERROR ("UavNetwork .. application error");
    #endif
  }
  cliLogin.close();

}

void UavNetwork::ConfigurePalcos() // TODO: poderia ser otimizada a leitura do arquivo colocando esta estrutura na configuração do cliente, mas isso tbm poderia confundir! Pensar!
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  std::ostringstream ss;
  std::ifstream scenario;
  ss << "./scratch/flynetwork/data/scenarios/" << m_PathData << ".txt";
  string file = ss.str();
  scenario.open(file);
  // ler informacoes dos arquivos
  if (scenario.is_open())
  {
    double x, y;
    int i = 0;
    std::string line;
    getline(scenario, line);
    getline(scenario, line);
    getline(scenario, line); // liberando as primeiras informacoes
    while (getline(scenario, line))
    {
      if (line.at(0) != '#')
      {
        ss.str("");
        ss << "fixed-" << i++;
        sscanf(line.c_str(), "%lf,%lf\n", &x, &y);
        NS_LOG_INFO(line.c_str());
        m_ssgnuPalcos << x << "\t" << y << "\n";
        m_palcoPos.push_back(x);
        m_palcoPos.push_back(y);
        // Set fixed nodes on server!
        m_serverApp->AddNewFixedClient(ss.str(), x, y);
      }
    }
    m_ssgnuPalcos << "e\n";
    scenario.close();
  }
  else
  {
    ss << "Não foi possível abrir o arquivo: " << file;
    NS_LOG_ERROR(ss.str().c_str());
    exit(-1);
  }
}

void UavNetwork::Configure()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  std::ostringstream ss;
  // Routing
  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  Ipv4StaticRoutingHelper staticR;
  m_list.Add(staticR, 100);
  switch (m_protocol)
  {
  case 1:
    m_list.Add(olsr, 100);
    m_protocolName = "OLSR";
    break;
  case 2:
    m_list.Add(aodv, 100);
    m_protocolName = "AODV";
    break;
  case 3:
    m_list.Add(dsdv, 100);
    m_protocolName = "DSDV";
    break;
  default:
    NS_FATAL_ERROR("No such protocol:" << m_protocol);
  }

  // configurando INTERNET
  m_stack.SetRoutingHelper(m_list);

  // IMPORTANTE: Information similar to the following appears for the show mesh config command:  fala o low SNR e high SNR https://www.cisco.com/c/en/us/td/docs/wireless/technology/mesh/8-0/design/guide/mesh80.pdf
  // Low Link SNR.................................. 12
  // High Link SNR................................. 60
  // Max Association Number........................ 10

  // Considerar o Cisco Aironet 1570 Series
  // -- https://www.cisco.com/c/en/us/td/docs/wireless/controller/technotes/8-0/1570-DG/b_Aironet_AP1570_DG.pdf
  // DataSheet: https://www.cisco.com/c/dam/en/us/products/collateral/wireless/aironet-1570-series/datasheet-c78-732348.pdf

  // ns3 exemplo: https://www.nsnam.org/doxygen/vht-wifi-network_8cc_source.html
  // usar taxa constante para funcionar como o DA. MCS0 para todos!

  // Ad Hoc
  m_adhocHelper.SetStandard(WIFI_PHY_STANDARD_80211ac); // https://en.wikipedia.org/wiki/IEEE_802.11ac
  m_adhocHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("VhtMcs0"),
                                          "ControlMode", StringValue ("VhtMcs0"));
  m_phyHelper = YansWifiPhyHelper::Default();
  m_channelHelper = YansWifiChannelHelper::Default();

  // TODO: change ChannelWidth = 20 and Frequency = 5180Hz ac e 2.4GHz

  m_channelHelper.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // ---> isto permite comunicacao entre UAVs!
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  double rss = -80;  // -dBm
  m_channelHelper.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  m_phyHelper.SetChannel(m_channelHelper.Create());
  // Set it to adhoc mode
  m_macAdHocHelper.SetType("ns3::AdhocWifiMac");
  m_addressHelper.SetBase("10.1.1.0", "255.255.255.0"); // adhoc address

  // Mobility UAV
  m_positionAlloc = CreateObjectWithAttributes<RandomDiscPositionAllocator>
                      ("X", DoubleValue (m_cx),
                        "Y", DoubleValue (m_cy),
                        "Rho", StringValue("ns3::ConstantRandomVariable[Constant=3.0]"));

  // Considerar o Ponto de Acesso Cisco Aironet 1550 Series
  // 16/01/2019 DataSheet: https://www.cisco.com/c/en/us/products/collateral/wireless/aironet-1550-series/data_sheet_c78-641373.pdf
  // Relacao MCS e SINR: https://www.cisco.com/c/en/us/td/docs/wireless/technology/mesh/8-0/design/guide/mesh80.pdf
  // If we consider only 802.11n rates, then Table 13: Requirements for LinkSNR with AP1552 for 2.4 and 5 GHz, on page 48 shows LinkSNR requirements with AP1552 for 2.4 and 5 GHz.
  // Wifi
  m_phyHelperCli = YansWifiPhyHelper::Default();
  m_channelHelperCli = YansWifiChannelHelper::Default();
  m_phyHelperCli.SetChannel(m_channelHelperCli.Create());
  m_wifiHelper.SetStandard(WIFI_PHY_STANDARD_80211n_2_4GHZ); // https://en.wikipedia.org/wiki/IEEE_802.11n-2009
  m_wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs0"),
                                          "ControlMode", StringValue ("HtMcs0"));
  m_macWifiHelperCli.SetType("ns3::StaWifiMac",
                             "Ssid", SsidValue(Ssid("flynetwork")),
                             "ActiveProbing", BooleanValue(false)); // configuração de scanning passivo

  m_macWifiHelper.SetType("ns3::ApWifiMac",
                          "Ssid", SsidValue(Ssid("flynetwork")));

  m_addressHelperCli.SetBase("192.168.1.0", "255.255.255.0"); // wifi address
}

void UavNetwork::PrintUavEnergy (int t)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<t);
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/etapa/" << t << "/uav_energy.txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  for (UavNodeContainer::Iterator it = m_uavNodeActive.Begin(); it != m_uavNodeActive.End(); ++it) {
    Ptr<UavDeviceEnergyModel> dev = (*it)->GetObject<UavDeviceEnergyModel>();
    file << (*it)->GetId()  << "," << dev->GetEnergySource()->GetRemainingEnergy() << "," << dev->GetEnergySource()->GetInitialEnergy() << std::endl;
  }
  file.close();
}

void UavNetwork::ClientPosition (string name)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<name);
  std::ofstream file;
  file.open(name, std::ofstream::out | std::ofstream::app);
  for (vector<double>::iterator i = m_palcoPos.begin(); i != m_palcoPos.end();)
  {
    if (i == m_palcoPos.begin()) {
      file << (*i) << ",";
      i++;
      file << (*i) << ",fixed,0";
    } else {
      file << "," << (*i) << ",";
      i++;
      file << (*i) << ",fixed,0";
    }

    i++;
  }
  for (NodeContainer::Iterator i = m_clientNode.Begin(); i != m_clientNode.End(); ++i)
  {
    int app = (*i)->GetNApplications()-1;
    Ptr<SmartphoneApplication> smart = NULL;
    do {
      smart = DynamicCast<SmartphoneApplication>((*i)->GetApplication(app));
      --app;
    } while (smart==NULL && app >= 0);
    NS_ASSERT (smart != NULL);
    Vector current = (*i)->GetObject<MobilityModel>()->GetPosition();
    file << "," << current.x << "," << current.y << "," << smart->GetLogin() << "," << smart->GetApp();
  }
  file.close();
}


} // namespace ns3
