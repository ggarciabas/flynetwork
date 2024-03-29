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
#include "uav-device-energy-model.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "uav-energy-source.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavDeviceEnergyModel");

NS_OBJECT_ENSURE_REGISTERED(UavDeviceEnergyModel);

TypeId
UavDeviceEnergyModel::GetTypeId(void)
{
  // http://documents.4rgos.it/v1/static/6259381_R_D002 http://www.argos.co.uk/product/6259381?tag=ar:drones:6259381
  // http://www.dronesglobe.com/guide/long-flight-time/
  static TypeId tid = TypeId("ns3::UavDeviceEnergyModel")
                          .SetParent<DeviceEnergyModel>()
                          .SetGroupName("Flynetwork-Energy")
                          .AddConstructor<UavDeviceEnergyModel>()
                          .AddAttribute("ResistTime",
                                        "Resist time of the UAV in seconds",
                                        DoubleValue(27*60), // s
                                        MakeDoubleAccessor(&UavDeviceEnergyModel::m_resistTime),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("PathData",
                                        "Name of scenario",
                                        StringValue("none"),
                                        MakeStringAccessor(&UavDeviceEnergyModel::m_pathData),
                                        MakeStringChecker())
                          .AddAttribute("xCentral",
                                        "",
                                        DoubleValue(0),
                                        MakeDoubleAccessor(&UavDeviceEnergyModel::m_xCentral),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("yCentral",
                                        "",
                                        DoubleValue(0),
                                        MakeDoubleAccessor(&UavDeviceEnergyModel::m_yCentral),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("AverageVelocity",
                                        "Average velocity of uav in m/s",
                                        DoubleValue(18), // 18 m/s
                                        MakeDoubleAccessor(&UavDeviceEnergyModel::m_avgVel),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("PeriodicEnergyUpdateInterval",
                                        "Time between two consecutive periodic energy updates - Hover time.",
                                        TimeValue(Seconds(0.5)), // s
                                        MakeTimeAccessor(&UavDeviceEnergyModel::m_energyUpdateInterval),
                                        MakeTimeChecker())
                         .AddTraceSource ("TotalEnergyConsumption",
                                          "Total energy consumption of the radio device.",
                                          MakeTraceSourceAccessor (&UavDeviceEnergyModel::m_totalEnergyConsumption),
                                          "ns3::TracedValueCallback::Double");
  return tid;
}

UavDeviceEnergyModel::UavDeviceEnergyModel()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_source = 0;
  m_energyCost = 0.0;
  m_totalEnergyConsumption = 0.0;
  m_hoverCost = 0.0;
  m_flying = true;
  m_timeToCentral = 0.0;
}

UavDeviceEnergyModel::~UavDeviceEnergyModel()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

double UavDeviceEnergyModel::GetHoverCost() {
  return m_hoverCost;
}

double UavDeviceEnergyModel::GetEnergyCost ()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_energyCost;
}

void UavDeviceEnergyModel::SetEnergyDepletionCallback(
   EnergyCallback callback)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << &callback);
  if (callback.IsNull())
  {
    NS_FATAL_ERROR("EnergyDepletionCallback:Setting NULL energy depletion callback!");
  }
  m_energyDepletionCallback = callback;
}

void UavDeviceEnergyModel::SetEnergyRechargedCallback(EnergyCallback callback)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << &callback);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyRechargedCallback:Setting NULL energy Recharged callback!");
  }
  m_energyRechargedCallback = callback;
}

void UavDeviceEnergyModel::SetEnergyAskUavCallback(EnergyCallback callback)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << &callback);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyAskUavCallback:Setting NULL energy Recharged callback!");
  }
  m_energyAskUavCallback = callback;
}


double UavDeviceEnergyModel::CalculateThreshold () {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  Vector actual = m_source->GetNode()->GetObject<MobilityModel>()->GetPosition();
  double distance = std::sqrt(std::pow(m_xCentral - actual.x, 2) + std::pow(m_yCentral - actual.y, 2));
  m_timeToCentral = distance / 5.0; // 5m/s
  NS_ASSERT(distance >= 0);
  double thr = ((m_energyCost * distance) + m_energyUpdateInterval.GetSeconds()*m_hoverCost*2) / m_source->GetInitialEnergy(); // % necessaria para voltar a central de onde está, mais o custo de hover durante o intervalo de atualização tanto para sair quanto quando chegar ao local, para enviar informacao a central de que chegou e que necessita se retirado
  // NS_LOG_DEBUG("UavDeviceEnergyModel::CalculateThreshold distance: " << distance << "m edist: " << (m_energyCost * distance) << "J");
  return thr*1.5;
}

void UavDeviceEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavDeviceEnergyModel::HandleEnergyRecharged [" << m_source->GetNode()->GetId() << "] @" << Simulator::Now().GetSeconds());
  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_recharged/uav_recharged.txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << m_source->GetNode()->GetId() << "," << DynamicCast<UavEnergySource>(m_source)->GetRealRemainingEnergy() << std::endl;
  m_file.close();

  m_totalEnergyConsumption = 0.0;
  m_lastPosition = m_source->GetNode()->GetObject<MobilityModel>()->GetPosition();
  m_lastTime = Simulator::Now();
  
  m_energyRechargedCallback();
}

void UavDeviceEnergyModel::HandleEnergyOff (void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void UavDeviceEnergyModel::HandleAskUav ()
{
  m_energyAskUavCallback();
}

void UavDeviceEnergyModel::HandleEnergyOn (void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void UavDeviceEnergyModel::HandleEnergyChanged(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void UavDeviceEnergyModel::HandleEnergyDepletion(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  Vector actual = m_source->GetNode()->GetObject<MobilityModel>()->GetPosition();
  double distance = std::sqrt(std::pow(m_xCentral - actual.x, 2) + std::pow(m_yCentral - actual.y, 2));
  NS_ASSERT(distance >= 0);
  // energy to decrease = energy cost * distance from last position to the actual
  double energy = m_energyCost * distance;
  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_depletion/uav_depletion_" << m_source->GetNode()->GetId() << ".txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << DynamicCast<UavEnergySource>(m_source)->GetRealRemainingEnergy() - energy << std::endl;
  m_file.close();
  NS_LOG_DEBUG("UavDeviceEnergyModel::HandleEnergyDepletion " << m_source->GetNode()->GetId() << " @" << Simulator::Now().GetSeconds());
  if (m_energyDepletionCallback.IsNull())
  {
    NS_FATAL_ERROR ("Energy depletion callback is null\n");
  } else {
    m_energyDepletionCallback(); // avisa a aplicação, helper quem configura!
  }
}

void UavDeviceEnergyModel::SetEnergyUpdateInterval(Time interval)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << interval);
  m_energyUpdateInterval = interval;
}

Time UavDeviceEnergyModel::GetEnergyUpdateInterval(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_energyUpdateInterval;
}

void UavDeviceEnergyModel::SetEnergySource(Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << source);
  NS_ASSERT(source != NULL);
  m_source = source;
  m_energyCost = m_source->GetInitialEnergy() / (m_resistTime * m_avgVel); // joule/meter
  m_hoverCost = m_source->GetInitialEnergy() / m_resistTime; // j/s

  // std::ostringstream os;
  // os << global_path << "/" << m_pathData << "/cost_energy.txt";
  // m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  // m_file << m_energyCost << "\n" << m_hoverCost << std::endl;
  // m_file.close();
  // os.str("");
  // os << "cp " << global_path << "/" << m_pathData << "/cost_energy.txt " << global_path << "/" << m_pathData << "/compare/cost_energy.txt";
  // system(os.str().c_str());
}

const Ptr<EnergySource>
UavDeviceEnergyModel::GetEnergySource()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_source;
}

void UavDeviceEnergyModel::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << node);
  NS_ASSERT(node != NULL);
  m_source->GetNode() = node;
  m_lastPosition = node->GetObject<MobilityModel>()->GetPosition();
  StartHover();
}

double
UavDeviceEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

void UavDeviceEnergyModel::HoverConsumption(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );

  if (m_flying) {
    NS_LOG_DEBUG ("UavDeviceEnergyModel::HoverConsumption problema flying == " << m_flying << " [" << m_source->GetNode()->GetId() << "] @" << Simulator::Now().GetSeconds());
    return;
  }

  // do not update if simulation has finished
  if (Simulator::IsFinished())
  {
    return;
  }

  double diff_time = Simulator::Now().GetSeconds() - m_lastTime.GetSeconds();
  double energyToDecrease = m_hoverCost * diff_time;
  m_totalEnergyConsumption += energyToDecrease;
  // NS_LOG_DEBUG("UavDeviceEnergyModel:HoverConsumption diff: " << diff_time << " energy: " << energyToDecrease/ m_source->GetInitialEnergy() << " @" << Simulator::Now().GetSeconds());
  DynamicCast<UavEnergySource> (m_source)->UpdateEnergySourceHover(energyToDecrease);

  // salvando historico do consumo de bateria
  #ifdef HOVER
    std::ostringstream os;
    os << global_path << "/" << m_pathData << "/uav_hover/uav_hover_" << m_source->GetNode()->GetId() << ".txt";
    m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
    m_file << Simulator::Now().GetSeconds() << "," << energyToDecrease / m_source->GetInitialEnergy() << std::endl;
    m_file.close();
  #endif

  m_lastTime = Simulator::Now();
  m_hoverEvent = Simulator::Schedule(m_energyUpdateInterval,
                                          &UavDeviceEnergyModel::HoverConsumption,
                                            this);
}

void UavDeviceEnergyModel::CourseChange (Ptr<const MobilityModel> mob) // Chamado pelo Mobility a cada movimento, não representa que chegou ao final! Somente a callback do Uav que representa isto!
{
  // NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  Vector actual = m_source->GetNode()->GetObject<MobilityModel>()->GetPosition();
  double distance = std::sqrt(std::pow(m_lastPosition.x - actual.x, 2) + std::pow(m_lastPosition.y - actual.y, 2));
  NS_ASSERT(distance >= 0);
  // energy to decrease = energy cost * distance from last position to the actual
  double energyToDecrease = m_energyCost * distance;
  m_totalEnergyConsumption += energyToDecrease;
  // NS_LOG_DEBUG("UavDeviceEnergyModel::CourseChange [" << m_source->GetNode()->GetId() << "] last: (" << m_lastPosition.x << "," << m_lastPosition.y << ") actual: (" << actual.x << "," << actual.y << ") energy: " << energyToDecrease << "J distance: " << distance);
  m_lastPosition.x = actual.x;
  m_lastPosition.y = actual.y;
  DynamicCast<UavEnergySource> (m_source)->UpdateEnergySourceMove(energyToDecrease);

  // salvando historico do consumo de bateria por movimentacao
  #ifdef MOVE
    std::ostringstream os;
    os << global_path << "/" << m_pathData << "/uav_move/uav_move_" << m_source->GetNode()->GetId() << ".txt";
    m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
    m_file << Simulator::Now().GetSeconds() << "," << energyToDecrease / m_source->GetInitialEnergy() << std::endl;
    m_file.close();  
  #endif
}

void UavDeviceEnergyModel::SetFlying(bool f) {
  m_flying = f;
  NS_LOG_DEBUG("UavDeviceEnergyModel::SetFlying [" << m_source->GetNode()->GetId() << "] " << m_flying << " @" << Simulator::Now().GetSeconds());
}

void UavDeviceEnergyModel::StopHover()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  // NS_ASSERT_MSG (!m_flying, "UavDeviceEnergyModel::StopHover [" << m_source->GetNode()->GetId() << "] @" << Simulator::Now().GetSeconds()); // m_flying corresponde a estar voando e não parado
  NS_LOG_DEBUG("UavDeviceEnergyModel::StopHover [" << m_source->GetNode()->GetId() << "] lasttime: " << m_lastTime.GetSeconds() << " @" << Simulator::Now().GetSeconds());
  Simulator::Remove(m_hoverEvent);
  m_hoverEvent = Simulator::ScheduleNow(&UavDeviceEnergyModel::HoverConsumption, this);
  Simulator::Remove(m_hoverEvent); /// removendo a programacao 
}

void UavDeviceEnergyModel::StartHover()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_lastTime = Simulator::Now();
  NS_LOG_DEBUG("UavDeviceEnergyModel::StartHover [" << m_source->GetNode()->GetId() << "] lasttime: " << m_lastTime.GetSeconds() << " @" << Simulator::Now().GetSeconds());
  m_hoverEvent = Simulator::Schedule(m_energyUpdateInterval,
                                          &UavDeviceEnergyModel::HoverConsumption,
                                            this);
}

double
UavDeviceEnergyModel::DoGetEnergyCost(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_energyCost;
}

void UavDeviceEnergyModel::DoDispose (void) {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("UavDeviceEnergyModel::DoDispose");
  m_source = 0;
  m_source->GetNode() = 0;
}

double
UavDeviceEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0; // calculado de forma diferenciada para movimentacao
}

} // namespace ns3
