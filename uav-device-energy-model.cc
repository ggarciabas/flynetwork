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
                          .AddAttribute("ScenarioName",
                                        "Name of scenario",
                                        StringValue("none"),
                                        MakeStringAccessor(&UavDeviceEnergyModel::m_scenarioName),
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
                                        TimeValue(Seconds(0.1)), // s
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
  NS_LOG_FUNCTION(this);
  m_source = 0;
  m_energyCost = 0.0;
  m_totalEnergyConsumption = 0.0;
  m_hoverCost = 0.0;
}

UavDeviceEnergyModel::~UavDeviceEnergyModel()
{
  NS_LOG_FUNCTION(this);
}

double UavDeviceEnergyModel::GetEnergyCost ()
{
  NS_LOG_FUNCTION(this);
  return m_energyCost;
}

void UavDeviceEnergyModel::SetEnergyDepletionCallback(
   EnergyCallback callback)
{
  NS_LOG_FUNCTION(this);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyDepletionCallback:Setting NULL energy depletion callback!");
  }
  m_energyDepletionCallback = callback;
}

void UavDeviceEnergyModel::SetEnergyRechargedCallback(
   EnergyCallback callback)
{
  NS_LOG_FUNCTION(this);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyRechargedCallback:Setting NULL energy Recharged callback!");
  }
  m_energyRechargedCallback = callback;
}

void UavDeviceEnergyModel::ChangeThreshold () {
  Vector actual = m_node->GetObject<MobilityModel>()->GetPosition();
  double distance = std::sqrt(std::pow(m_xCentral - actual.x, 2) + std::pow(m_yCentral - actual.y, 2));
  NS_ASSERT(distance >= 0);
  double thr = (m_energyCost * distance)/ m_source->GetInitialEnergy(); // % necessaria para voltar a central de onde está!
  DynamicCast<UavEnergySource>(m_source)->SetBasicEnergyLowBatteryThreshold(thr*1.02); // +2%!
}

void UavDeviceEnergyModel::HandleEnergyRecharged (void)
{
  m_energyRechargedCallback();
  m_totalEnergyConsumption = 0.0;
  m_lastPosition = m_node->GetObject<MobilityModel>()->GetPosition();
  m_lastTime = Simulator::Now();
  m_hoverEvent = Simulator::Schedule(m_energyUpdateInterval,
                                          &UavDeviceEnergyModel::HoverConsumption,
                                            this);
}

void UavDeviceEnergyModel::HandleEnergyDepletion(void)
{
 Vector actual = m_node->GetObject<MobilityModel>()->GetPosition();
 double distance = std::sqrt(std::pow(m_xCentral - actual.x, 2) + std::pow(m_yCentral - actual.y, 2));
 NS_ASSERT(distance >= 0);
 // energy to decrease = energy cost * distance from last position to the actual
 double energy = m_energyCost * distance;
 std::ostringstream os;
 os << "./scratch/flynetwork/data/output/" << m_scenarioName << "/depletion_" << m_node->GetId() << ".txt";
 m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
 m_file << Simulator::Now().GetSeconds() << "," << m_source->GetRemainingEnergy() - energy << std::endl;
 m_file.close();
 m_energyDepletionCallback(); // avisa a aplicação, helper quem configura!
}

void UavDeviceEnergyModel::SetEnergyUpdateInterval(Time interval)
{
  NS_LOG_FUNCTION(this << interval);
  m_energyUpdateInterval = interval;
}

Time UavDeviceEnergyModel::GetEnergyUpdateInterval(void) const
{
  NS_LOG_FUNCTION(this);
  return m_energyUpdateInterval;
}

void UavDeviceEnergyModel::SetEnergySource(Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION(this << source);
  NS_ASSERT(source != NULL);
  m_source = source;
  m_energyCost = m_source->GetInitialEnergy() / (m_resistTime * m_avgVel); // joule/meter
  m_hoverCost = m_source->GetInitialEnergy() / m_resistTime;

  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_scenarioName << "/cost_energy.txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << "Energycost: " << m_energyCost << "\nHovercost: " << m_hoverCost << std::endl;
  m_file.close();
}

const Ptr<EnergySource>
UavDeviceEnergyModel::GetEnergySource()
{
  return m_source;
}

void UavDeviceEnergyModel::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION(this << node);
  NS_ASSERT(node != NULL);
  m_node = node;
  m_lastPosition = node->GetObject<MobilityModel>()->GetPosition();
  StartHover();
}

Ptr<Node>
UavDeviceEnergyModel::GetNode() const
{
  NS_LOG_FUNCTION(this);
  return m_node;
}

double
UavDeviceEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

void UavDeviceEnergyModel::HoverConsumption(void)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG("UavDeviceEnergyModel:HoverConsumption.");

  // do not update if simulation has finished
  if (Simulator::IsFinished())
  {
    return;
  }

  m_hoverEvent.Cancel();

  double diff_time = Simulator::Now().GetSeconds() - m_lastTime.GetSeconds();
  double energyToDecrease = m_hoverCost * diff_time;
  DynamicCast<UavEnergySource> (m_source)->UpdateEnergySourceHover(energyToDecrease);

  m_lastTime = Simulator::Now();
  m_hoverEvent = Simulator::Schedule(m_energyUpdateInterval,
                                          &UavDeviceEnergyModel::HoverConsumption,
                                            this);
}

void UavDeviceEnergyModel::CourseChange (Ptr<const MobilityModel> mob)
{
  NS_LOG_FUNCTION(this);
  Vector actual = m_node->GetObject<MobilityModel>()->GetPosition();
  double distance = std::sqrt(std::pow(m_lastPosition.x - actual.x, 2) + std::pow(m_lastPosition.y - actual.y, 2));
  NS_ASSERT(distance >= 0);
  // energy to decrease = energy cost * distance from last position to the actual
  double energyToDecrease = m_energyCost * distance;

  m_totalEnergyConsumption += energyToDecrease;

  DynamicCast<UavEnergySource> (m_source)->UpdateEnergySourceMov(energyToDecrease);

  // salvando historico do consumo de bateria por movimentacao
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_scenarioName << "/uav_moving_" << m_node->GetId() << ".txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << m_totalEnergyConsumption / m_source->GetInitialEnergy() << std::endl;
  m_file.close();
}

void UavDeviceEnergyModel::StopHover()
{
  m_hoverEvent.Cancel();
  HoverConsumption();
}

void UavDeviceEnergyModel::StartHover()
{
  m_lastTime = Simulator::Now();
  m_hoverEvent = Simulator::Schedule(m_energyUpdateInterval,
                                          &UavDeviceEnergyModel::HoverConsumption,
                                            this);
}

double
UavDeviceEnergyModel::DoGetEnergyCost(void) const
{
  NS_LOG_FUNCTION(this);
  return m_energyCost;
}

void UavDeviceEnergyModel::DoDispose (void) {
  NS_LOG_DEBUG("UavDeviceEnergyModel::DoDispose");
  m_source = 0;
  m_node = 0;
}

double
UavDeviceEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0; // calculado de forma diferenciada para movimentacao
}

} // namespace ns3
