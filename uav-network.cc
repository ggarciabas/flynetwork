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
#include "uav-model.h"

#include <fstream>
#include <cstdlib>
#include <string>

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
                          .AddAttribute("ClientUpdateCons",
                                        "Tempo para atualziar consumo de energi",
                                        DoubleValue(1.0),
                                        MakeDoubleAccessor(&UavNetwork::m_clientUpdateCons),
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
                                        DoubleValue(300),
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
                          .AddAttribute("UavTimingNext",
                                        "Uav Timing Next",
                                        DoubleValue(150.0),
                                        MakeDoubleAccessor(&UavNetwork::m_uavTimingNext),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("ServerPort",
                                        "Communication port number of server",
                                        UintegerValue(8082),
                                        MakeUintegerAccessor(&UavNetwork::m_serverPort),
                                        MakeUintegerChecker<uint16_t>())
                          .AddAttribute("ClientPort",
                                        "Communication port number of client",
                                        UintegerValue(9090),
                                        MakeUintegerAccessor(&UavNetwork::m_cliPort),
                                        MakeUintegerChecker<uint16_t>())
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
                          .AddAttribute("Seed",
                                        "Seed",
                                        UintegerValue(1),
                                        MakeUintegerAccessor(&UavNetwork::m_seed),
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

UavNetwork::UavNetwork() //: m_filePacketServer(global_path << "/packet_trace_server.txt"), m_filePacketUav(global_path << "/packet_trace_uav.txt"), m_filePacketClient(global_path << "/packet_trace_client.txt")
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
  ss_ << "./scratch/wifi/data/scenarios/" << m_PathData << ".txt";
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

  ss << "/" << m_seed << "/" << m_protocol << "/custo_" << m_custo; // adicionando seed
  m_pathData = ss.str();
  ss.str("");
  ss << "rm -Rf " << global_path << "/" << m_pathData;
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData;
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/course_changed";
  system(ss.str().c_str());
  // ss.str("");
  // ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_remaining_energy";
  // system(ss.str().c_str());
  #ifdef HOVER
    ss.str("");
    ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_hover";
    system(ss.str().c_str());
  #endif
  #ifdef MOVE
    ss.str("");
    ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_move";
    system(ss.str().c_str());
  #endif
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_energy";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_energy_threshold";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/etapa";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_recharged";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_depletion";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_client";
  system(ss.str().c_str());
  // ss.str("");
  // ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_wifi";
  // system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/uav_stop";
  system(ss.str().c_str());
  ss.str("");
  // ss << "mkdir -p " << global_path << "/" << m_pathData << "/python";
  // system(ss.str().c_str());
  // ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/dhcp";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/client";
  system(ss.str().c_str());
  ss.str("");
  ss << "mkdir -p " << global_path << "/" << m_pathData << "/compare";
  system(ss.str().c_str());
  // ss.str("");
  // ss << global_path << "/"<< m_pathData<<"/packet_trace_server.txt";
  // m_filePacketServer = ss.str().c_str();
  // ss.str("");
  // ss << global_path << "/"<<m_pathData<<"/packet_trace_uav.txt";
  // m_filePacketUav = ss.str().c_str();
  // ss.str("");
  // ss << global_path << "/"<<m_pathData<<"/packet_trace_client.txt";
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
  // NS_LOG_INFO("Configurando UAV");
  // ConfigureUav();

  // Configure Client
  NS_LOG_INFO("Configurando Cliente");
  ConfigureCli();


  Simulator::Schedule(Seconds(m_simulationTime-0.001), &UavNetwork::PrintFinalUavEnergy, this);
  Simulator::Stop(Seconds(m_simulationTime));


  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_network_log.txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << etapa << " " << global_speed << " " << global_nc << " " << global_ec_persec << std::endl;
  m_file.close();

  NS_LOG_DEBUG("Iniciando Simulador");
  Simulator::Run();
  NS_LOG_DEBUG("Finalizando Simulador");  

  Simulator::Destroy();
  NS_LOG_DEBUG("Finalizando Destroy");

  // m_filePacketServer.close(); // fechando arquivo de trace (pacotes)
  // m_filePacketUav.close();    // fechando arquivo de trace (pacotes)
  // m_filePacketClient.close(); // fechando arquivo de trace (pacotes)

}

void UavNetwork::PrintFinalUavEnergy ()
{
  // parando os UAVs para gerar relatorios
  for (UavNodeContainer::Iterator i = m_uavNodeActive.Begin(); i != m_uavNodeActive.End(); ++i) {
    int app = (*i)->GetNApplications()-1;
    Ptr<UavApplication> uavApp = NULL;
    do {
      uavApp = DynamicCast<UavApplication>((*i)->GetApplication(app));
      --app;
    } while (uavApp==NULL && app >= 0);
    NS_ASSERT (uavApp != NULL);
    uavApp->Stop(); // para gravar informacoes energia !
  }
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

  // configurando PacketSink
  ObjectFactory packFacAdhoc;
  packFacAdhoc.SetTypeId ("ns3::PacketSink");
  #ifdef TCP
    packFacAdhoc.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
  #endif
  #ifdef UDP
    packFacAdhoc.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
  #endif
  packFacAdhoc.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), m_serverPort))); // OPS: utilizam Ipv4Address::GetAny ()
  Ptr<Application> appAdhoc = packFacAdhoc.Create<Application> ();
  appAdhoc->SetStartTime(Seconds(10.0));
  appAdhoc->SetStopTime(Seconds(m_simulationTime));
  m_serverNode.Get(0)->AddApplication (appAdhoc);

  // create server app
  ObjectFactory obj;
  obj.SetTypeId("ns3::ServerApplication");
  obj.Set("DataRate", DataRateValue(DataRate("2Mbps")));
  obj.Set("Environment", UintegerValue(m_environment));
  obj.Set("Ipv4Address", Ipv4AddressValue(m_serverAddress.GetAddress(0)));
  obj.Set("ServerPort", UintegerValue(m_serverPort));
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
  m_serverApp->TraceConnectWithoutContext("ClientPositionTrace", MakeCallback(&UavNetwork::ClientPosition, this));

  // trace do PacketSink RX
  appAdhoc->TraceConnectWithoutContext ("Rx", MakeCallback (&ServerApplication::TracedCallbackRxApp, m_serverApp));

  // aggregate to node
  m_serverNode.Get(0)->AddApplication(m_serverApp);
}

void UavNetwork::NewUav(int total, int update) // update = 0- normal 1- supply 2-depletion
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

    // Adicionando informacoes na aplicacao servidor!
    Ipv4Address addr = n->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
    NS_LOG_DEBUG("IP " << addr << " -------------");
    Ptr<UavEnergySource> source = DynamicCast<UavEnergySource>(uavApp->GetUavDevice()->GetEnergySource());
    if (update == 1) {
      // envia ao servidor informacoes do UAV substituto
      m_serverApp->AddSupplyUav(n->GetId(), addr, DynamicCast<UavEnergySource>(source)->GetRealRemainingEnergy(), uavApp->GetUavDevice()->GetEnergyCost(), uavApp->GetUavDevice()->GetHoverCost(), source->GetInitialEnergy(), n->GetObject<MobilityModel>());
    } else { // adiciona um novo UAV no servidor
      m_serverApp->AddNewUav(n->GetId(), addr, DynamicCast<UavEnergySource>(source)->GetRealRemainingEnergy(), uavApp->GetUavDevice()->GetEnergyCost(), uavApp->GetUavDevice()->GetHoverCost(), source->GetInitialEnergy(), n->GetObject<MobilityModel>()); // tell the server to create a new model of UAV, used to identify the actual location of those UAV nodes
    }
    source = 0;

    std::ostringstream os;
    os << global_path << "/" << m_pathData << "/uav_network_log.txt";
    m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
    m_file << Simulator::Now().GetSeconds() << "," << n->GetId() << "," << update << std::endl;
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
  uavApp->Stop(); // aqui já limpa o container de clientes

  // modificando posicionamento para fora do cenario
  Vector v (m_iniX, m_iniY,0);
  Ptr<UavMobilityModel> model = n->GetObject<UavMobilityModel>();
  model->SetFirstPosition(v); // manda para perto da central!

  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_network_log.txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << n->GetId() << ",0" << std::endl;
  m_file.close();

  os.str("");
  os << global_path << "/" << m_pathData << "//uav_removed_energy.txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  Ptr<UavDeviceEnergyModel> dev = uavApp->GetUavDevice();
  m_file << Simulator::Now().GetSeconds() << " " << DynamicCast<UavEnergySource>(dev->GetEnergySource())->GetRealRemainingEnergy() / dev->GetEnergySource()->GetInitialEnergy() << std::endl;
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

  // configurando internet
  m_stack.Install(uav);

  // configurando pilha de protocolos
  Ipv4InterfaceContainer addContainer(m_addressHelper.Assign(adhoc));

  // install mobility
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
  sourceHelper.Set("UavEnergySourceInitialEnergy", DoubleValue(total_battery));
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

  // create and configure UAVApp and Sink application
  int c = 0;
  std::ostringstream oss, poolAddr, minAddr, maxAddr, serverAddr;
  for (NodeContainer::Iterator i = uav.Begin(); i != uav.End(); ++i, ++c)
  {
    // configure PacketSink
    ObjectFactory packFac;
    packFac.SetTypeId ("ns3::PacketSink");
    #ifdef TCP
      packFac.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
    #endif
    #ifdef UDP
      packFac.Set ("Protocol", StringValue ("ns3::UdpSocketFactory"));
    #endif
    packFac.Set ("Local", AddressValue (InetSocketAddress (Ipv4Address::GetAny (), m_serverPort))); // OPS: utilizam Ipv4Address::GetAny ()
    Ptr<Application> app = packFac.Create<Application> ();
    app->SetStartTime(Seconds(0.0));
    app->SetStopTime(Seconds(m_simulationTime));
    (*i)->AddApplication (app);

    ObjectFactory obj; // create UavApplication
    obj.SetTypeId("ns3::UavApplication");
    obj.Set("Id", UintegerValue((*i)->GetId()));
    Ptr<UniformRandomVariable> uptime = CreateObject<UniformRandomVariable>();
    obj.Set("UpdateTime", DoubleValue(m_updateTimeUav+uptime->GetValue()));
    obj.Set("Remote", Ipv4AddressValue(m_serverAddress.GetAddress(0)));
    obj.Set("DataRate", DataRateValue(DataRate("11Mbps")));
    obj.Set("AdhocAddress", Ipv4AddressValue(addContainer.GetAddress(c)));
    obj.Set("ServerPort", UintegerValue(m_serverPort));
    obj.Set("PathData", StringValue(m_pathData));

    Ptr<UavApplication> uavApp = obj.Create()->GetObject<UavApplication>();
    uavApp->SetStartTime(Seconds(0.0));
    uavApp->SetStopTime(Seconds(0.01));

    Ptr<WifiNetDevice> wifiDeviceAdhoc = DynamicCast<WifiNetDevice> (adhoc.Get(c));
    Ptr<WifiPhy> wifiPhyAdhoc = wifiDeviceAdhoc->GetPhy ();
    app->TraceConnectWithoutContext ("Rx", MakeCallback (&UavApplication::TracedCallbackRxApp, uavApp));

    // aggregate to the node
    (*i)->AddApplication(uavApp);

    // install device model
    energyHelper.SetEnergyDepletionCallback(MakeCallback (&UavApplication::EnergyDepletionCallback, uavApp));
    energyHelper.SetEnergyRechargedCallback(MakeCallback (&UavApplication::EnergyRechargedCallback, uavApp));
    DeviceEnergyModelContainer uavEnergyModels = energyHelper.Install((*i), sources.Get(c));
    Ptr<UavDeviceEnergyModel> dev = DynamicCast<UavDeviceEnergyModel>(uavEnergyModels.Get(0));

    // adicionando devices no UAVApp
    uavApp->SetUavDevice(dev);

    // Mobility    
    (*i)->GetObject<MobilityModel>()->TraceConnectWithoutContext ("CourseChange", MakeCallback (&UavApplication::CourseChange, uavApp));
    DynamicCast<UavMobilityModel>((*i)->GetObject<MobilityModel>())->TraceConnectWithoutContext ("CourseChangeDevice", MakeCallback (&UavDeviceEnergyModel::CourseChange, dev));

    // energy start
    DynamicCast<UavEnergySource>(sources.Get(c))->Start();
    DynamicCast<UavEnergySource>(sources.Get(c))->TimeEnergy(m_uavTimingNext);

    m_nodeUavApp[(*i)->GetId()] = m_uavAppContainer.GetN(); // pego antes para ficar com a posicao real, sem necessidade de pegar o tamanho e fazer operação de redução de uma unidade
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
  ss << "./scratch/wifi/data/scenarios/" << m_PathData << ".txt";
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
        // devices.Add(m_wifiHelper.Install(m_phyHelperCli, m_macWifiHelperCli, nodes));
        // stack.Install(nodes);
        // TODO_NEW: verificar se existe algum problema executar simulacoes com nós sem devices!
        MobilityHelper mobilityCLI;
        Ptr<PositionAllocator> positionAlloc = CreateObjectWithAttributes<RandomDiscPositionAllocator>
                            ("X", DoubleValue (x),
                              "Y", DoubleValue (y),
                            "Rho", StringValue("ns3::ConstantRandomVariable[Constant=20.0]"));
        mobilityCLI.SetPositionAllocator(positionAlloc);
        mobilityCLI.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                                     "Bounds", RectangleValue(Rectangle(m_xmin, m_xmax, m_ymin, m_ymax)),
                                      "Speed", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=5.0]")); // xmin, xmax, ymin, ymax
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

  for (NodeContainer::Iterator it = m_clientNode.Begin(); it != m_clientNode.End(); ++it) 
  {
    m_cliEvent.push_back(EventId());
  }

  global_nc = m_totalCli;
}

void UavNetwork::ClientBehaviour (int posCli) {
  // 1- desligar evento anteior
  Simulator::Remove (m_cliEvent.at(posCli));
  std::ostringstream ss;
  std::ofstream file;
  ss << global_path << "/" << m_pathData << "/client/app_log.txt";
  file.open(ss.str(), std::ofstream::out | std::ofstream::app);
  Ptr<UniformRandomVariable> startTime = CreateObject<UniformRandomVariable>(); // Padrão [0,1]
  // 2- atualizar evento para o novo app 
  // 3- criar evento para reprogramar o cliente (rechamar esta funcao no tempo final)
  switch ((int)m_randApp->GetValue()) {
    case 0: // VOICE // considerando 111 minutos mensal, 3.7 diario - http://www.teleco.com.br/comentario/com631.asp
      file << Simulator::Now().GetSeconds() << "," << m_clientNode.Get(posCli)->GetId() << ",VOICE" << std::endl;
      m_cliEvent[posCli] = Simulator::Schedule(Seconds(startTime->GetValue()), &UavNetwork::ClientConsumption, this, posCli); // start
      Simulator::Schedule (Seconds(222), &UavNetwork::ClientBehaviour, this, posCli); // stop
      break;
    case 1: // VIDEO
      file << Simulator::Now().GetSeconds() << "," << m_clientNode.Get(posCli)->GetId() << ",VIDEO" << std::endl;
      m_cliEvent[posCli] = Simulator::Schedule(Seconds(startTime->GetValue()), &UavNetwork::ClientConsumption, this, posCli); // start
      Simulator::Schedule (Seconds(240), &UavNetwork::ClientBehaviour, this, posCli); // stop - sem referencia (4 min)
      break;
    case 2: // WWW
      file << Simulator::Now().GetSeconds() << "," << m_clientNode.Get(posCli)->GetId() << ",WWW" << std::endl;
      m_cliEvent[posCli] = Simulator::Schedule(Seconds(startTime->GetValue()), &UavNetwork::ClientConsumption, this, posCli); // start
      Simulator::Schedule (Seconds(180), &UavNetwork::ClientBehaviour, this, posCli); // stop - sem referencia (3 min)
      break;
    default: // NONE - qt tempo sem fazer nada?!
      file << Simulator::Now().GetSeconds() << "," << m_clientNode.Get(posCli)->GetId() << ",NONE" << std::endl;
      Simulator::Schedule (Seconds(120), &UavNetwork::ClientBehaviour, this, posCli); // stop
  }
  file.close();  
}

void UavNetwork::ClientConsumption (int posCli)
{
  // 1-  verificar qual Uav está mais próximo
  Vector cliVec = m_clientNode.Get(posCli)->GetObject<MobilityModel>()->GetPosition();
  double min = numeric_limits<double>::max();
  double dist;
  uint32_t id = -1;
  for (UavNodeContainer::Iterator i = m_uavNodeActive.Begin(); i != m_uavNodeActive.End(); ++i){
    dist = CalculateDistance((*i)->GetObject<MobilityModel>()->GetPosition(), cliVec);
    if (dist < min && dist <= global_cli_cob) { // TODO: colocar um valor aleatorio para quando chegar em duas opcoes diferenciar com 50% de chance para cada lado? Criar variavel como atributo da classe e distribuido uniformemente
      min = dist;
      id = (*i)->GetId();
    }
  }
  // caso tenha encontrado algum UAV, consumir
  if ((int)id != -1) {    
    m_uavAppContainer.Get(m_nodeUavApp[id])->ClientConsumption(m_clientUpdateCons);
  } else {
    std::ostringstream ss;
    std::ofstream file;
    ss << global_path << "/" << m_pathData << "/client/desc_log.txt";
    file.open(ss.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << "," << m_clientUpdateCons << "," << posCli << std::endl;
    file.close();
  }

  m_cliEvent[posCli] = Simulator::Schedule(Seconds(m_clientUpdateCons), &UavNetwork::ClientConsumption, this, posCli);
}

void UavNetwork::ConfigurePalcos() // TODO: poderia ser otimizada a leitura do arquivo colocando esta estrutura na configuração do cliente, mas isso tbm poderia confundir! Pensar!
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  std::ostringstream ss;
  std::ifstream scenario;
  ss << "./scratch/wifi/data/scenarios/" << m_PathData << ".txt";
  string file = ss.str();
  ss.str("");
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
        // m_ssgnuPalcos << x << "\t" << y << "\n";
        m_palcoPos.push_back(x);
        m_palcoPos.push_back(y);
        // Set fixed nodes on server!
        m_serverApp->AddNewFixedClient(ss.str(), x, y);
      }
    }
    // m_ssgnuPalcos << "e\n";
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

  // configurando estrutura de geracao de aplicacoes para clientes
  m_randApp->SetAttribute ("Min", DoubleValue (0));
  m_randApp->SetAttribute ("Max", DoubleValue (5)); // 0- voice 1- video 2- www 3->5 nothing

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
}

void UavNetwork::PrintUavEnergy (int t)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<t);
  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_energy.txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  for (UavApplicationContainer::Iterator it = m_uavAppContainer.Begin(); it != m_uavAppContainer.End(); ++it) {
    Ptr<UavDeviceEnergyModel> dev = (*it)->GetUavDevice();
    file << Simulator::Now().GetSeconds() << "," << (*it)->GetId()  << "," << DynamicCast<UavEnergySource>(dev->GetEnergySource())->GetRealRemainingEnergy() << "," << dev->GetEnergySource()->GetInitialEnergy() << std::endl;
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
    Vector current = (*i)->GetObject<MobilityModel>()->GetPosition();
    file << "," << current.x << "," << current.y;
  }
  file.close();
}


} // namespace ns3
