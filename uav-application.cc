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
                          .AddAttribute("AdhocAddress", "The address of the adhoc interface node",
                                        Ipv4AddressValue(),
                                        MakeIpv4AddressAccessor(&UavApplication::m_addressAdhoc),
                                        MakeIpv4AddressChecker())
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
  m_goto.push_back(0.0); // inicializando para nao ocorrer conflito
  m_goto.push_back(0.0);
  m_central.push_back(0.0); // inicializando para nao ocorrer conflito
  m_central.push_back(0.0);
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
  // incia source e atualiza threshold
  DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->Start();  
  m_uavDevice->SetFlying(false);
  m_uavDevice->StartHover();
  StartApplication();
  m_depletion = false;
  m_running = true;
  m_goto[0] = 0.0; // reiniciando posicionamento goto
  m_goto[1] = 0.0;
  // threshold do uav necessario calcular somente uma vez
  DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->SetBasicEnergyLowBatteryThresholdUav(m_uavDevice->CalculateThreshold());
}

void UavApplication::StartApplication(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::StartApplication [" << m_id << "]");

  #ifdef UDP
    m_sendSck = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
  #endif

  if (m_sendSck->Connect(InetSocketAddress(m_peer, m_serverPort))) {
    NS_FATAL_ERROR ("UAV - $$ [NÃO] conseguiu conectar com Servidor!");
  } 
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
  m_uavDevice->SetFlying(true);

  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_energy/uav_energy.txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  double rem = 0.;
  double iniE = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetInitialEnergy();
  if (m_depletion) {
    rem = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy();
  } else {
    rem = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy();
  }
  // TODO_NEW: lembrar de armazenar o consumo total dos clientes para log
  double me = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetMoveEnergy();
  double he = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetHoverEnergy();  
  double we = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetWifiEnergy();  

  int count = 0;
  // TODO_NEW: caclular o total de cliente pelo tamanho do container de clientes e limpar o container!
  // for(std::map<Ipv4Address, Ptr<ClientModel> >::iterator i = m_mapClient.begin(); i != m_mapClient.end(); i++)
  // {
  //   if ((i->second)->GetLogin().compare("NOPOSITION") != 0) { // cliente com posicionamento atualizado!
  //     count++;
  //   }
  // }

  // TIME UAV_ID INITIAL_E ACTUAL_E WIFI_E CLIENT_E MOVE_E HOVER_E TOTAL_CLI
  file << Simulator::Now().GetSeconds() << " " << m_node->GetId() << " " << iniE << " " << rem << " " << we << " " << me << " " << he << " " << count << " " << ((m_depletion)?"TRUE ":"FALSE ") << std::endl;
  file.close();

  // para source!
  DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->Stop();  
}

void UavApplication::StopApplication()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavApplication::StopApplication [" << m_id << "]");
  Simulator::Remove(m_stopEvent);
  Simulator::Remove(m_sendEvent);
  Simulator::Remove(m_sendCliDataEvent);
  Simulator::Remove(m_packetAskUav);
  if (m_sendSck)
  {
    m_sendSck->ShutdownRecv();
    m_sendSck->ShutdownSend();
    m_sendSck->Close();
    m_sendSck = 0;
  }
}

void UavApplication::SendCliData ()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  if (m_running && !m_depletion) {
    Simulator::Remove(m_sendCliDataEvent);
    std::ostringstream msg, ss, slog;
    msg << "DATA " << m_id << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy();

    m_clientContainerLast.Add(m_clientContainer);

    if (int(m_clientContainer.GetN()) > global_ksize) { // somente agrupa se houver mais clientes do que o maximo para análise do DA de Localizacao
      // -----> filtrar os clientes
      // identify borders
      ClientModelContainer::Iterator it = m_clientContainer.Begin();
      double xmin, ymin, xmax, ymax, x, y;
      xmin = xmax = (*it)->GetPosition().at(0);
      ymin = ymax = (*it)->GetPosition().at(1);
      it++;
      for (; it != m_clientContainer.End(); ++it) {
        x = (*it)->GetPosition().at(0);
        y = (*it)->GetPosition().at(1);
        slog << x << " " << y << " ";
        if (x > xmax) {
          xmax = x;
        } else if (x < xmin) {
          xmin = x;
        }
        if (y > ymax) {
          ymax = y;
        } else if (y < ymin) {
          ymin = y;
        }
      }
      slog << "\n";
      // define position allocator
      ObjectFactory posObj;
      posObj.SetTypeId ("ns3::RandomRectanglePositionAllocator");
      ss.str("");
      ss << "ns3::UniformRandomVariable[Min=" << xmin << "|Max=" << xmax << "]";
      posObj.Set ("X", StringValue (ss.str().c_str()));
      ss.str("");
      ss << "ns3::UniformRandomVariable[Min=" << ymin << "|Max=" << ymax << "]";
      posObj.Set ("Y", StringValue (ss.str().c_str()));
      Ptr <PositionAllocator> pAlloc = posObj.Create ()->GetObject <PositionAllocator> ();

      // criando grupos
      std::vector<double> sumCluXPos; // para calcular o novo posicionamento do cluster!
      std::vector<double> sumCluYPos; // para calcular o novo posicionamento do cluster!
      std::vector<double> totCluPos; // total de clientes no cluster
      ClientModelContainer groupedCli;
      Ptr<ClientModel> cli;
      for (int i = 0; i < global_ksize; ++i) {
        ObjectFactory obj;
        obj.SetTypeId("ns3::ClientModel");
        obj.Set("Id", UintegerValue(i)); // id 
        cli = obj.Create()->GetObject<ClientModel>();
        cli->SetUpdatePos(Simulator::Now()); // time
        Vector v = pAlloc->GetNext();
        slog << v.x << " " << v.y << " ";
        cli->SetPosition(v.x, v.y);
        groupedCli.Add(cli);
        sumCluXPos.push_back(0.0);
        sumCluYPos.push_back(0.0);
        totCluPos.push_back(0.0);
      }
      slog << "\n";
      // encontrar clientes que pertençam ao grupo   
      ClientModelContainer::Iterator min, j;
      double min_dist, d;
      int min_pos, pos;
      for (ClientModelContainer::Iterator i = m_clientContainer.Begin(); i != m_clientContainer.End(); ++i) 
      {
        j = groupedCli.Begin();
        min = j; // cluster mais perto é o primeiro
        min_pos = 0;
        min_dist = std::sqrt(std::pow((*i)->GetPosition().at(0) - (*min)->GetPosition().at(0), 2) + std::pow((*i)->GetPosition().at(1) - (*min)->GetPosition().at(1), 2));
        j++;
        pos = 1;
        for (; j != groupedCli.End(); ++j, ++pos) 
        {
          d = std::sqrt(std::pow((*i)->GetPosition().at(0) - (*j)->GetPosition().at(0), 2) + std::pow((*i)->GetPosition().at(1) - (*j)->GetPosition().at(1), 2));
          if (d < min_dist) {
            min = j;
            min_pos = pos;
          }
        }
        (*min)->AddConsumption((*i)->GetConsumption()); // atualizando consumo do cluster
        sumCluXPos.at(min_pos) += (*i)->GetPosition().at(0);
        sumCluYPos.at(min_pos) += (*i)->GetPosition().at(1);
        totCluPos.at(min_pos) += 1.0;
        slog << min_pos << " ";
      }
      slog << "\n";

      // construindo mensagem para o servidor
      pos = 0;
      for (ClientModelContainer::Iterator it = groupedCli.Begin(); it != groupedCli.End(); ++it, ++pos) {
        if (totCluPos.at(pos)>0) {
          msg << " " << (*it)->GetId();
          msg << " " << (*it)->GetUpdatePos().GetSeconds();
          // atualiza posicao com média dos clientes
          (*it)->SetPosition(sumCluXPos.at(pos)/totCluPos.at(pos), sumCluYPos.at(pos)/totCluPos.at(pos));
          msg << " " << (*it)->GetPosition().at(0) << " " << (*it)->GetPosition().at(1) << " " << (*it)->GetConsumption() << " " << totCluPos.at(pos);
          slog << pos << " " << (*it)->GetPosition().at(0) << " " << (*it)->GetPosition().at(1) << " " << (*it)->GetConsumption() << " " << totCluPos.at(pos) << "\n";
        }
      }    
      msg << " \0";

    } else {
      for (ClientModelContainer::Iterator it = m_clientContainer.Begin(); it != m_clientContainer.End(); ++it) {
        msg << " " << (*it)->GetId();
        msg << " " << (*it)->GetUpdatePos().GetSeconds();
        msg << " " << (*it)->GetPosition().at(0) << " " << (*it)->GetPosition().at(1) << " " << (*it)->GetConsumption() << " 1.0";
        slog << (*it)->GetPosition().at(0) << " " << (*it)->GetPosition().at(1) << " " << (*it)->GetConsumption() << " 1.0\n";
      }    
      msg << " \0";
    }

    std::ostringstream os;
    os << global_path << "/" << m_pathData << "/uav_client/uav_" << m_id << ".txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    Ptr<const MobilityModel> mob = GetNode()->GetObject<MobilityModel>();
    file << Simulator::Now().GetSeconds() << " " <<  mob->GetPosition().x << " " << mob->GetPosition().y << "\n" << slog.str();
    file.close();

    this->SendCliDataMsg(msg.str());
  } 
}

void 
UavApplication::SendCliDataMsg(std::string msg)
{
  uint16_t packetSize = msg.length() + 1;
  Ptr<Packet> packet = Create<Packet>((uint8_t *)msg.c_str(), packetSize);
  NS_LOG_DEBUG ("UavApplication::SendCliData " << msg << " @" << Simulator::Now().GetSeconds());
  if (m_sendSck->Send(packet) == packetSize)
  {
    NS_LOG_INFO("UAV [" << m_id << "] @" << Simulator::Now().GetSeconds() << " - DATA to Server.");
  }

  m_sendCliDataEvent = Simulator::Schedule (Seconds(5.0), &UavApplication::SendCliDataMsg, this, msg);
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
  // TODO_NEW: considerar o consumo dos clientes como threshold superior e somente a quatidade de energia para ir a central para o threshold inferior
  DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->SetBasicEnergyLowBatteryThresholdUav(m_uavDevice->CalculateThreshold()); 

  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/course_changed/course_changed_" << m_id << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  file << Simulator::Now().GetSeconds() << "," <<  mob->GetPosition().x << "," << mob->GetPosition().y << std::endl;
  file.close();

  m_uavDevice->SetFlying(false);
  m_uavDevice->StartHover();
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
}

void
UavApplication::EnergyDepletionCallback() // TODO_NEW: criar um aviso de carga para o servidor já providenciar um novo UAV
{
  Simulator::Remove(m_programDepletion);
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("---->>> EnergyDepletionCallback [" << m_id << "] going to " << m_central.at(0) << "," << m_central.at(1) << " @" << Simulator::Now().GetSeconds());
  m_depletion = true; 
  EraseClientContainerLast(); 
  
  // Ir para central
  if (!m_uavDevice->IsFlying()) { // caso nao esteja voando
    m_uavDevice->StopHover();
    m_uavDevice->SetFlying(true);
  }
  GetNode()->GetObject<MobilityModel>()->SetPosition(Vector(m_central.at(0), m_central.at(1), 1.0)); // Verficar necessidade de subir em no eixo Z
}
void UavApplication::EnergyAskUavCallback()
{
  // avisar central e mudar posição para central!
  m_packetAskUav = Simulator::ScheduleNow(&UavApplication::SendPacketNewUav, this);
}
/*
  Pacote para envio de informacao de onde o UAV estava afim de se repor aquela localizacao, o UAV será redirecionado para a central pois este só tem bateria para voltar!
*/
void UavApplication::SendPacketNewUav(void)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  m_packetAskUav.Cancel();

  if (m_running) {
    std::ostringstream m;
    Vector pos = GetNode()->GetObject<MobilityModel>()->GetPosition();    
    if (m_uavDevice->IsFlying()) // caso o uav estiver voando, se faz necessario enviar solicitacao para o novo UAV ir direto para a posicao destino
      m << "NEWUAV " << m_id << " " << m_goto.at(0) << " " << m_goto.at(1) << " " << pos.z << " " << '\0';
    else // caso contrário, necessario enviar solicitacao para a posicao atual do UAV
      m << "NEWUAV " << m_id << " " << pos.x << " " << pos.y << " " << pos.z << " " << '\0';

    std::ostringstream os;
    os << global_path << "/" << m_pathData << "/uav_depletion/newuav_log.txt";
    std::ofstream file;
    file.open(os.str(), std::ofstream::out | std::ofstream::app);
    file << Simulator::Now().GetSeconds() << " " << m_id
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetInitialEnergy() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetWifiAcum() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetMoveAcum() 
    << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetHoverAcum() << std::endl; // TIME, UAV_ID, TOTAL_CLIENTES, remainingenergy, initialbattery, wifiacum, moveacum, hoveracum, cliacum
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
      m_packetAskUav = Simulator::Schedule(Seconds(0.01), &UavApplication::SendPacketNewUav, this);
      return;
    }
  } else {
    NS_LOG_DEBUG("Uav not running " << this->m_id);
  }
  
  m_packetAskUav = Simulator::Schedule(Seconds(0.05), &UavApplication::SendPacketNewUav, this);
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
        m_clientContainer.Clear(); // limpando container de informacoes de clientes apos enviadas as informacoes para o servidor
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
          // TODO_NEW: utilizar o tamanho do container de clientes
          // for(std::map<Ipv4Address, Ptr<ClientModel> >::iterator i = m_mapClient.begin(); i != m_mapClient.end(); i++)
          // {
          //   if ((i->second)->GetLogin().compare("NOPOSITION") != 0) { // cliente com posicionamento atualizado!
          //     count++;
          //   }
          // }
          std::ostringstream os;
          os << global_path << "/" << m_pathData << "/uav_stop/stop_log.txt"; // uavs que foram retirados da rede
          std::ofstream file;
          file.open(os.str(), std::ofstream::out | std::ofstream::app);
          file << Simulator::Now().GetSeconds() << " " << m_id << " " << count 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetInitialEnergy() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetWifiAcum() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetMoveAcum() 
          << " " << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetHoverAcum() << std::endl; // TIME, UAV_ID, TOTAL_CLIENTES, ATUAL_BATERIA, FULL_BATERIA, wifiacum, moveacum, hoveracum, cliacum

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
      } else if (results.at(0).compare("NEWUAVOK") == 0)
        {
          m_packetAskUav.Cancel();
          std::ostringstream mm;
          mm << "UAV\t" << m_id << "\tRECEIVED\t" << Simulator::Now().GetSeconds() << "\tNEWUAVOK";
          //m_packetTrace(mm.str());
          NS_LOG_DEBUG(mm.str() << " @" << Simulator::Now().GetSeconds());   
          // programando para este UAV retornar a central no tempo necessario para o novo UAV chegar
          m_programDepletion = Simulator::Schedule(Seconds(m_uavDevice->GetTimeToCentral()), &UavApplication::EnergyDepletionCallback, this);       
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
    msg << DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->GetRealRemainingEnergy() << '\0';
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

void UavApplication::ClientConsumption (double time, double px, double py, uint32_t id) 
{
  double cons = DynamicCast<UavEnergySource>(m_uavDevice->GetEnergySource())->UpdateEnergySourceClient(time);
  Ptr<ClientModel> cli = m_clientContainer.FindClientModel(id); // id
  if (cli != NULL) { // update se existir
    cli->SetUpdatePos(Simulator::Now()); // atualiza tempo
    cli->SetPosition(px, py); // atualiza pos
    cli->AddConsumption(cons);
  } else { // criar novo
    ObjectFactory obj;
    obj.SetTypeId("ns3::ClientModel");
    obj.Set("Id", UintegerValue(id)); // id
    cli = obj.Create()->GetObject<ClientModel>();
    cli->SetUpdatePos(Simulator::Now()); // time
    cli->SetPosition(px, py);
    cli->AddConsumption(cons);
    m_clientContainer.Add(cli);
  }
}

} // namespace ns3
