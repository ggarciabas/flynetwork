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
#include "uav-application.h"
#include "ns3/internet-apps-module.h"
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
                                        UintegerValue(9090),
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
  // incia source e atualiza threshold
  DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->Start();  
  m_uavDevice->SetFlying(false);
  m_uavDevice->StartHover();
  StartApplication();
  m_depletion = false;
  m_running = true;
  m_meanConsumption = 0.0;
  m_goto[0] = 0.0; // reiniciando posicionamento goto
  m_goto[1] = 0.0;
  // threshold do uav necessario calcular somente uma vez
  #ifdef DEV_WIFI
    DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->SetBasicEnergyLowBatteryThresholdUav(m_uavDevice->CalculateThreshold()+m_meanConsumption*2);
  #else
    DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->SetBasicEnergyLowBatteryThresholdUav(m_uavDevice->CalculateThreshold());
  #endif
}

void UavApplication::StartApplication(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::StartApplication [" << m_id << "]");
  #ifdef TCP_CLI
    m_socketClient = Socket::CreateSocket (GetNode(), SocketFactory::GetTypeId ());
  #else
    m_socketClient = Socket::CreateSocket (GetNode(), UdpSocketFactory::GetTypeId ());
  #endif

  #ifdef UDP
    m_sendSck = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
  #endif

  if (m_sendSck->Connect(InetSocketAddress(m_peer, m_serverPort))) {
    NS_FATAL_ERROR ("UAV - $$ [NÃO] conseguiu conectar com Servidor!");
  }
  NS_LOG_DEBUG ("---->   Programando: " << etapa-20);
  m_askCliPos = Simulator::Schedule(Seconds(etapa-20), &UavApplication::AskCliPosition, this);  
}

void UavApplication::Stop() 
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::Stop [" << m_id << "]");
  // SetStopTime(Simulator::Now());
  Simulator::Remove(m_stopEvent);
  m_running = false;
  StopApplication();

  // Para o hover
  m_uavDevice->StopHover();
  m_uavDevice->SetFlying(false);

  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_energy/uav_energy_" << m_node->GetId() << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  double rem = 0.;
  double iniE = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetInitialEnergy();
  if (m_depletion) {
    rem = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy();
  } else {
    rem = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy();
  }
  double we = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetWifiEnergy();
  double ce = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetClientEnergy();
  double me = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetMoveEnergy();
  double he = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetHoverEnergy();

  NS_ASSERT_MSG (we+ce+me+he == (iniE-rem), "Bateria consumida não bateu com o acumulado dos modos!");

  // TIME UAV_ID INITIAL_E ACTUAL_E SUM_E_MODE MODE DEPLETION?
  file << Simulator::Now().GetSeconds() << " " << m_id << " " << iniE << " " << rem << " " <<  we << " WIFI " << ((m_depletion)?"TRUE ":"FALSE ") << std::endl;
  file << Simulator::Now().GetSeconds() << " " << m_id << " " << iniE << " " << rem << " " <<  ce << " CLIENT " << ((m_depletion)?"TRUE ":"FALSE ") << std::endl;
  file << Simulator::Now().GetSeconds() << " " << m_id << " " << iniE << " " << rem << " " <<  me << " MOVE " << ((m_depletion)?"TRUE ":"FALSE ") << std::endl;
  file << Simulator::Now().GetSeconds() << " " << m_id << " " << iniE << " " << rem << " " <<  he << " HOVER " << ((m_depletion)?"TRUE ":"FALSE ") << std::endl;
  file.close();
  
  // para source!
  DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->Stop();  
}

void UavApplication::StopApplication()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::StopApplication [" << m_id << "]");
  m_meanConsumption = 0.0;
  Simulator::Remove(m_stopEvent);
  Simulator::Remove(m_sendEvent);
  Simulator::Remove(m_sendCliDataEvent);
  Simulator::Remove(m_packetDepletion);
  Simulator::Remove(m_askCliPos);
  if (m_sendSck)
  {
    m_sendSck->ShutdownRecv();
    m_sendSck->ShutdownSend();
    m_sendSck->Close();
    m_sendSck = 0;
  }
}

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
    m_uavDevice->SetFlying(false);
    m_uavDevice->StartHover();
    NS_LOG_DEBUG ("[" << m_id << "] Starting hovering in the location waiting central to remove @" << Simulator::Now().GetSeconds());
    return; // nao fazer topicos abaixo em estado de emergencia
  }

  // threshold do uav necessario calcular somente uma vez
  #ifdef DEV_WIFI
    DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->SetBasicEnergyLowBatteryThresholdUav(m_uavDevice->CalculateThreshold()+m_meanConsumption*2);
  #else
    DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->SetBasicEnergyLowBatteryThresholdUav(m_uavDevice->CalculateThreshold());
  #endif

  // ligar wifi quando chegar ao posicionamento correto
  // m_wifiDevice->HandleEnergyOn();

  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/course_changed/course_changed_" << m_id << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << "," <<  mob->GetPosition().x << "," << mob->GetPosition().y << std::endl;
  file.close();
  m_uavDevice->SetFlying(false);
  m_uavDevice->StartHover();
}

void UavApplication::SetWifiDevice (Ptr<WifiRadioEnergyModel> dev) {
  m_wifiDevice = dev;
}

void UavApplication::SetUavDevice (Ptr<UavDeviceEnergyModel> dev) {
  m_uavDevice = dev;
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
  NS_LOG_DEBUG("---- EnergyRechargedCallback #" << m_id << " @" << Simulator::Now().GetSeconds());

  // if (m_depletion) {
  //   // reiniciando aplicacao DHCP
  //   int app = GetNode()->GetNApplications()-1;
  //   Ptr<DhcpServer> dhcp = NULL;
  //   do {
  //     dhcp = DynamicCast<DhcpServer>(GetNode()->GetApplication(app));
  //     --app;
  //   } while (dhcp==NULL && app >= 0);
  //   NS_ASSERT (dhcp != NULL);
  //   dhcp->Resume();
  // }
}

void
UavApplication::EnergyDepletionCallback()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("---->>> EnergyDepletionCallback [" << m_id << "] going to " << m_central.at(0) << "," << m_central.at(1) << " @" << Simulator::Now().GetSeconds());
  m_depletion = true;

  // avisar central e mudar posição para central!
  m_packetDepletion = Simulator::ScheduleNow(&UavApplication::SendPacketDepletion, this);
  
  // Ir para central
  if (!m_uavDevice->IsFlying()) { // caso nao esteja voando
    m_uavDevice->StopHover();
    m_uavDevice->SetFlying(true);
  }
  GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(m_central.at(0), m_central.at(1), 1.0)); // Verficar necessidade de subir em no eixo Z

  // pausando aplicacao DHCP
  // int app = GetNode()->GetNApplications()-1;
  // Ptr<DhcpServer> dhcp = NULL;
  // do {
  //   dhcp = DynamicCast<DhcpServer>(GetNode()->GetApplication(app));
  //   --app;
  // } while (dhcp==NULL && app >= 0);
  // NS_ASSERT (dhcp != NULL);
  // dhcp->Pause();

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
    if (m_uavDevice->IsFlying()) // caso o uav estiver voando, se faz necessario enviar solicitacao para o novo UAV ir direto para a posicao destino
      m << "DEPLETION " << m_id << " " << m_goto.at(0) << " " << m_goto.at(1) << " " << pos.z << " " << '\0';
    else // caso contrário, necessario enviar solicitacao para a posicao atual do UAV
      m << "DEPLETION " << m_id << " " << pos.x << " " << pos.y << " " << pos.z << " " << '\0';

    int count = 0;
    for(std::map<Ipv4Address, Ptr<ClientModel> >::iterator i = m_mapClient.begin(); i != m_mapClient.end(); i++)
    {
      if ((i->second)->GetLogin().compare("NOPOSITION") != 0) { // cliente com posicionamento atualizado!
        count++;
      }
    }
    std::ostringstream os;
    os << global_path << "/" << m_pathData << "/uav_depletion/depletion_log.txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " " << m_id << " " << count 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetInitialEnergy() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetWifiAcum() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetMoveAcum() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetHoverAcum()
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetCliAcum() << std::endl; // TIME, UAV_ID, TOTAL_CLIENTES, remainingenergy, initialbattery, wifiacum, moveacum, hoveracum, cliacum
    file.close();

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
    NS_LOG_DEBUG("Uav not running " << this->m_id);
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
        vector<double> pa, pn;
        pn.push_back(std::stod(results.at(1), &sz));
        pn.push_back(std::stod(results.at(2), &sz));
        pa.push_back(GetNode()->GetObject<MobilityModel>()->GetPosition().x);
        pa.push_back(GetNode()->GetObject<MobilityModel>()->GetPosition().y);
        
        /// Validando para graficos de bateria, identificar a quantidade de bateria 'jogada fora' quando o UAV é solicitado de volta a central.
        if (pn.at(0) == m_central.at(0) && pn.at(1) == m_central.at(1))
        { // validando se a direção solicitada é a central
          int count = 0;
          for(std::map<Ipv4Address, Ptr<ClientModel> >::iterator i = m_mapClient.begin(); i != m_mapClient.end(); i++)
          {
            if ((i->second)->GetLogin().compare("NOPOSITION") != 0) { // cliente com posicionamento atualizado!
              count++;
            }
          }
          std::ostringstream os;
          os << global_path << "/" << m_pathData << "/uav_stop/stop_log.txt"; // uavs que foram retirados da rede
          std::ofstream file;
          file.open(os.str(), std::ofstream::out | std::ofstream::app);
          file << Simulator::Now().GetSeconds() << " " << m_id << " " << count 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetInitialEnergy() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetWifiAcum() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetMoveAcum() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetHoverAcum()
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetCliAcum() << std::endl; // TIME, UAV_ID, TOTAL_CLIENTES, ATUAL_BATERIA, FULL_BATERIA, wifiacum, moveacum, hoveracum, cliacum

          file.close();
        }

        // verificar se já não está na posicao        
        if (CalculateDistance(m_goto, pn) != 0) { // verificar se já nao receberu um pacote com este comando de posicionamento
          NS_LOG_DEBUG("UAV [" << m_id << "] recebeu novo posicionamento @" << Simulator::Now().GetSeconds());
          m_goto[0] = std::stod(results.at(1), &sz); // atualizando goto
          m_goto[1] = std::stod(results.at(2), &sz); // atualizando goto
          if (CalculateDistance(pa, pn) >= 1e-3) { // verificar se já nao está no posicionamento desejado
            // deixar wifi desligado ao se mover
            // m_wifiDevice->HandleEnergyOff();
            // mudar o posicionamento do UAV
            m_uavDevice->StopHover();
            m_uavDevice->SetFlying(true);
            GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(std::stod(results.at(1), &sz), std::stod(results.at(2), &sz), (z > 0) ? z : 0.0)); // Verficar necessidade de subir em no eixo Z
          } else {
            NS_LOG_DEBUG ("UAV [" << m_id << "] na posicao correta, atualizando servidor @" << Simulator::Now().GetSeconds());
            SendPacket();
          }
        } else {
          NS_LOG_DEBUG ("UAV [" << m_id << "] recebeu pacote com mesmo comando de posicionamento [" << pn.at(0) << "," << pn.at(1) << "] -> [" << pa.at(0) << "," << pa.at(1) << "] == " << CalculateDistance(pa, pn) << " @" << Simulator::Now().GetSeconds());          
          if (CalculateDistance(pa, pn) < 1e-3) { // verificar se já nao está no posicionamento desejado
            NS_LOG_DEBUG(" UAV [" << m_id << "] já no posicionamento! @" << Simulator::Now().GetSeconds());
            SendPacket(); // atualizando servidor, nao houve alteracao do posicionamento pelo servidor!
            // ligar wifi quando chegar ao posicionamento correto
            // m_wifiDevice->HandleEnergyOn();
          }
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
UavApplication::TracedCallbackRxOnOff (Ptr<const Packet> packet, const Address & address)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << packet << address);
  Ipv4Address ip = InetSocketAddress::ConvertFrom(address).GetIpv4();

  // NS_LOG_DEBUG (Simulator::Now().GetSeconds() << " RECEBIDO APP CLIENTE " << ip);

  // armazenar consumo do cliente de acordo com o uso e passar esta informacao para o servidor, para q o calculo do custo seja mais realistico com o consumo dos clientes
  std::map<Ipv4Address, Ptr<ClientModel> >::iterator it = m_mapClient.find(ip);
  if (it != m_mapClient.end()) {
    // 0.00126928 - custo por recebimento pego pelo wifidevenergymodel
    (it->second)->AddConsumption(0.00126928);
  }

  #ifdef LOG_CLIENT
    std::ostringstream os;
    os << global_path << "/" << m_pathData << "/wifi/" << ip << ".txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " RECEBIDO " << packet->GetSize () << std::endl;
    file.close();
  #endif
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

    NS_LOG_DEBUG ("UavApplication::TracedCallbackRxAppInfra " << m_id << " :: " << msg << " @" << Simulator::Now().GetSeconds());

    if (results.at(0).compare("CLIENT") == 0) { // received a message from a client
      Ipv4Address ip = InetSocketAddress::ConvertFrom(address).GetIpv4();
      NS_LOG_DEBUG ("CLIENT " << ip << " " << m_id << " @" << Simulator::Now().GetSeconds());
      std::map<Ipv4Address, Ptr<ClientModel> >::iterator it = m_mapClient.find(ip);
      if (it != m_mapClient.end()) {
        std::string::size_type sz; // alias of size_t
        std::vector<double> pos;
        pos.push_back(std::stod (results.at(1),&sz));
        pos.push_back(std::stod (results.at(2),&sz));
        (it->second)->SetLogin(results.at(3));
        (it->second)->SetPosition(pos.at(0), pos.at(1));
        (it->second)->SetUpdatePos (Simulator::Now());
        #ifdef PACKET_UAV_CLI
          std::ostringstream os;
          os << global_path << "/" << m_pathData << "/client/" << ip << ".txt";
          std::ofstream file;
          file.open(os.str(), std::ofstream::out | std::ofstream::app);
          file << Simulator::Now().GetSeconds() << " RECEBIDO UAV" << std::endl; // ENVIADO
          file.close();
        #endif
        // repply to client to stop sending position, cliente para de enviar posicionamento apos 10s
        if (m_socketClient && !m_socketClient->Connect (InetSocketAddress (ip, m_cliPort))) {
          std::ostringstream msg;
          msg << "CLIENTOK " << '\0';
          uint16_t packetSize = msg.str().length() + 1;
          Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
          if (m_socketClient && m_socketClient->Send(packet, 0) == packetSize)
          {
            NS_LOG_DEBUG("UavApplication::TracedCallbackRxAppInfra envio de confirmacao para  " << ip << " @" << Simulator::Now().GetSeconds());
          } else {
            NS_LOG_DEBUG("UavApplication::TracedCallbackRxAppInfra ERRO resposta para cliente " << ip << " @" << Simulator::Now().GetSeconds());
          }
        }
      } else {
        NS_LOG_DEBUG("Não encontrou o IP, problema com callback de envio de IPs. " << m_id);
      }
    }
    results.clear();
  }
}

void UavApplication::SetCliDevice (Ptr<ClientDeviceEnergyModel> dev) 
{
  m_cliDevice = dev;
}

void 
UavApplication::AskCliPosition()
{
  NS_LOG_FUNCTION (m_id << Simulator::Now().GetSeconds());
  for(std::map<Ipv4Address, Ptr<ClientModel> >::iterator i = m_mapClient.begin(); i != m_mapClient.end(); i++)
  {
    if (m_socketClient && !m_socketClient->Connect (InetSocketAddress (i->first, m_cliPort))) {
      std::ostringstream msg;
      msg << "CLIENTLOC " << '\0';
      uint16_t packetSize = msg.str().length() + 1;
      Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.str().c_str(), packetSize);
      if (m_socketClient && m_socketClient->Send(packet, 0) == packetSize)
      {
        NS_LOG_DEBUG("UavApplication::AskCliPosition envio pacote para " << i->first << " @" << Simulator::Now().GetSeconds());
      } else {
        NS_LOG_DEBUG("UavApplication::AskCliPosition erro ao enviar solicitacao de posicionamento ao cliente no endereco " << i->first << " @" << Simulator::Now().GetSeconds());
      }
    }
  }
  
  m_askCliPos = Simulator::Schedule(Seconds(etapa), &UavApplication::AskCliPosition, this);
}

void 
UavApplication::TracedCallbackExpiryLease (const Ipv4Address& ip)
{
  NS_LOG_FUNCTION (m_id << ip);
  // Remover informacoes do mapa ; m_mapClient
  std::map<Ipv4Address, Ptr<ClientModel> >::iterator it = m_mapClient.find(ip);
  if (m_mapClient.find(ip) == m_mapClient.end()) {
    NS_FATAL_ERROR("UavApplication::TracedCallbackExpiryLease IP nao encontrado no container!");
  }
  (it->second)->SetLogin("NOPOSITION"); // ignorado ao enviar informacoes para o servidor

  if (m_cliDevice != NULL)
    m_cliDevice->RemoveClient();
}

void UavApplication::TracedCallbackNewLease (const Ipv4Address& ip)
{
  NS_LOG_FUNCTION (m_id << ip);
  // adicionar IP no mapa
  std::map<Ipv4Address, Ptr<ClientModel> >::iterator it = m_mapClient.find(ip);
  if (it == m_mapClient.end()) { // nao tem cadastrado este IP
    ObjectFactory obj;
    obj.SetTypeId("ns3::ClientModel");
    obj.Set("Login", StringValue("NOPOSITION")); // id
    m_mapClient[ip] = obj.Create()->GetObject<ClientModel>();
  } else {
    (it->second)->SetLogin("NOPOSITION"); // ignorado ao enviar informacoes para o servidor, esperando atualizar
  }

  if (m_cliDevice != NULL)
    m_cliDevice->AddClient();
}

void UavApplication::SendCliData ()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  if (m_running && !m_depletion) {
    Simulator::Remove(m_sendCliDataEvent);
    std::ostringstream msg;
    msg << "DATA " << m_id << " " << m_uavDevice->GetEnergySource()->GetRealRemainingEnergy();
    for(std::map<Ipv4Address, Ptr<ClientModel> >::iterator i = m_mapClient.begin(); i != m_mapClient.end(); i++)
    {
      if ((i->second)->GetLogin().compare("NOPOSITION") != 0) { // cliente com posicionamento atualizado
        msg << " " << (i->second)->GetLogin();
        msg << " " << (i->second)->GetUpdatePos().GetSeconds();
        msg << " " << (i->second)->GetPosition().at(0) << " " << (i->second)->GetPosition().at(1) << " " << (i->second)->GetConsumption();
        (i->second)->SetConsumption(0.0); // cosiderando consumo por etapa, por isto zerando o valor!
      }
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
  msg << "UAV " << pos.x << " " << pos.y << " " << pos.z << " " << m_id << " ";
  if (m_depletion)
  { // para nao ocorrer conflitos com calculo do wifi energy module!
    msg << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy() << '\0';
  } else {
    msg << m_uavDevice->GetEnergySource()->GetRealRemainingEnergy() << '\0';
  }
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

void UavApplication::TotalEnergyConsumptionTrace (double oldV, double newV)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds()  << oldV << newV);
  m_meanConsumption += (newV - oldV);
  m_meanConsumption /= 2.0;
}

} // namespace ns3
