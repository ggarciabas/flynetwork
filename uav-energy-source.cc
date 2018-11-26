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

#include "uav-energy-source.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/mobility-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavEnergySource");

NS_OBJECT_ENSURE_REGISTERED(UavEnergySource);

TypeId
UavEnergySource::GetTypeId(void)
{
  // DEFINIR VALOR PADRÃO PARA: considerando as informacoes do drone:  http://documents.4rgos.it/v1/static/6259381_R_D002 http://www.argos.co.uk/product/6259381?tag=ar:drones:6259381 - 3830mAh -> 1mAh = 3.6 mAs Voltage 11.4 V Energy 43.6 Wh
  // 43.6Wh * 3600J = 156960J --- https://www.rc-electronics-usa.com/battery-electronics-101.html
  static TypeId tid = TypeId("ns3::UavEnergySource")
                          .SetParent<EnergySource>()
                          .SetGroupName("Flynetwork-Energy")
                          .AddConstructor<UavEnergySource>()
                          .AddAttribute("UavEnergySourceInitialEnergy",
                                        "Initial energy stored in basic energy source.",
                                        DoubleValue(156960), // in Joules
                                        MakeDoubleAccessor(&UavEnergySource::SetInitialEnergy, &UavEnergySource::GetInitialEnergy),
                                        MakeDoubleChecker<double>())
                          .AddAttribute ("BasicEnergySupplyVoltageV",
                                         "Initial supply voltage for basic energy source.",
                                         DoubleValue (11.4), // in Volts
                                         MakeDoubleAccessor (&UavEnergySource::SetSupplyVoltage,
                                                             &UavEnergySource::GetSupplyVoltage),
                                         MakeDoubleChecker<double> ())
                                      // TODO: o valor de threshold deve ser dinamico em relacao do custo necessario para ele voltar a central de onde ele está
                          .AddAttribute("BasicEnergyLowBatteryThreshold",
                                        "Low battery threshold for basic energy source.",
                                        DoubleValue(0.10), // as a fraction of the initial energy
                                        MakeDoubleAccessor(&UavEnergySource::m_lowBatteryTh),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("BasicEnergyHighBatteryThreshold",
                                        "High battery threshold for basic energy source.",
                                        DoubleValue(0.15), // as a fraction of the initial energy
                                        MakeDoubleAccessor(&UavEnergySource::m_highBatteryTh),
                                        MakeDoubleChecker<double>())
                        .AddAttribute ("PeriodicEnergyUpdateInterval",
                                       "Time between two consecutive periodic energy updates.",
                                       TimeValue (Seconds (1.2)),
                                       MakeTimeAccessor (&UavEnergySource::SetEnergyUpdateInterval,
                                                         &UavEnergySource::GetEnergyUpdateInterval),
                                       MakeTimeChecker ())
                         .AddAttribute("ScenarioName",
                                       "Name of scenario",
                                       StringValue(""),
                                       MakeStringAccessor(&UavEnergySource::m_scenarioName),
                                       MakeStringChecker())
                         .AddTraceSource("RemainingEnergy",
                                          "Remaining energy at UavEnergySource (miliamperesecond - mAs).",
                                          MakeTraceSourceAccessor(&UavEnergySource::m_remainingEnergyJ),
                                          "ns3::TracedValueCallback::Double");
  return tid;
}

UavEnergySource::UavEnergySource()
{
  NS_LOG_FUNCTION(this);
  m_lastUpdateTime = Seconds (0.0);
  m_depleted = false;
  m_node = 0;
}

UavEnergySource::~UavEnergySource()
{
  NS_LOG_FUNCTION(this);
}

void UavEnergySource::SetInitialEnergy(double initialEnergyJ)
{
  NS_LOG_FUNCTION(this << initialEnergyJ);
  NS_ASSERT(initialEnergyJ >= 0);
  m_initialEnergyJ = initialEnergyJ;
  m_remainingEnergyJ = m_initialEnergyJ;
}

void
UavEnergySource::SetSupplyVoltage (double supplyVoltageV)
{
  NS_LOG_FUNCTION (this << supplyVoltageV);
  m_supplyVoltageV = supplyVoltageV;
}

void
UavEnergySource::SetEnergyUpdateInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_energyUpdateInterval = interval;
}

Time
UavEnergySource::GetEnergyUpdateInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_energyUpdateInterval;
}

double
UavEnergySource::GetInitialEnergy(void) const
{
  NS_LOG_FUNCTION(this);
  return m_initialEnergyJ;
}

double
UavEnergySource::GetSupplyVoltage (void) const
{
  NS_LOG_FUNCTION (this);
  return m_supplyVoltageV;
}

double
UavEnergySource::GetRemainingEnergy(void)
{
  NS_LOG_FUNCTION(this);
  // update energy source to get the latest remaining energy.
  UpdateEnergySource ();
  return m_remainingEnergyJ;
}

double
UavEnergySource::GetEnergyFraction (void)
{
  NS_LOG_FUNCTION (this);
  // update energy source to get the latest remaining energy.
  UpdateEnergySource ();
  return m_remainingEnergyJ / m_initialEnergyJ;
}

void
UavEnergySource::UpdateEnergySource (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("UavEnergySource:Updating remaining energy.");

  // do not update if simulation has finished
  if (Simulator::IsFinished ())
    {
      return;
    }

  m_energyUpdateEvent.Cancel ();

  double remainingEnergy = m_remainingEnergyJ;
  CalculateRemainingEnergy ();

  m_lastUpdateTime = Simulator::Now ();

  if (!m_depleted && m_remainingEnergyJ <= m_lowBatteryTh * m_initialEnergyJ)
    {
      m_depleted = true;
      HandleEnergyDrainedEvent ();
    }
  else if (m_depleted && m_remainingEnergyJ > m_highBatteryTh * m_initialEnergyJ)
    {
      m_depleted = false;
      HandleEnergyRechargedEvent ();
    }
  else if (m_remainingEnergyJ != remainingEnergy)
    {
      NotifyEnergyChanged ();
    }

  m_energyUpdateEvent = Simulator::Schedule (m_energyUpdateInterval, &UavEnergySource::UpdateEnergySource, this);

  // salvando historico do consumo de bateria por comunicacao
  if (m_node) {
    std::ostringstream os;
    os << "./scratch/flynetwork/data/output/" << m_scenarioName << "/uav_battery_" << m_node->GetId() << ".txt";
    m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
    m_file << Simulator::Now().GetSeconds() << "," << m_remainingEnergyJ / m_initialEnergyJ << std::endl;
    m_file.close();
  }
}


void UavEnergySource::UpdateEnergySourceMov (double energyToDecrease)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_INFO("UavEnergySource:Updating remaining energy.");

  if (m_remainingEnergyJ < energyToDecrease)
  {
    m_remainingEnergyJ = 0; // energy never goes below 0
  }
  else
  {
    m_remainingEnergyJ -= energyToDecrease;
  }

  if (!m_depleted && m_remainingEnergyJ <= m_lowBatteryTh * m_initialEnergyJ)
  {
    m_depleted = true;
    HandleEnergyDrainedEvent();
  }
  // salvando historico do consumo de bateria por movimentacao
  if (m_node) {
    std::ostringstream os;
    os << "./scratch/flynetwork/data/output/" << m_scenarioName << "/uav_battery_" << m_node->GetId() << ".txt";
    m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
    m_file << Simulator::Now().GetSeconds() << "," << m_remainingEnergyJ / m_initialEnergyJ << std::endl;
    m_file.close();
  }
}

void UavEnergySource::UpdateEnergySourceHover (double energyToDecrease)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_INFO("UavEnergySource:UpdateEnergySourceHover.");

  if (m_remainingEnergyJ < energyToDecrease)
  {
    m_remainingEnergyJ = 0; // energy never goes below 0
  }
  else
  {
    m_remainingEnergyJ -= energyToDecrease;
  }

  if (!m_depleted && m_remainingEnergyJ <= m_lowBatteryTh * m_initialEnergyJ)
  {
    m_depleted = true;
    HandleEnergyDrainedEvent();
  }
  // salvando historico do consumo de bateria por movimentacao
  if (m_node) {
    std::ostringstream os;
    os << "./scratch/flynetwork/data/output/" << m_scenarioName << "/uav_battery_hover_" << m_node->GetId() << ".txt";
    m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
    m_file << Simulator::Now().GetSeconds() << "," << m_remainingEnergyJ / m_initialEnergyJ << std::endl;
    m_file.close();
  }
}

/*
 * Private functions start here.
 */

void UavEnergySource::HandleEnergyDrainedEvent(void)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_INFO("UavEnergySource:Energy depleted!");
  NotifyEnergyDrained(); // notify DeviceEnergyModel objects
}

void
UavEnergySource::CalculateRemainingEnergy (void)
{
  NS_LOG_FUNCTION (this);
  double totalCurrentA = CalculateTotalCurrent (); // busca nos devices o total de 'current'
  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.IsPositive ());
  // energy = current * voltage * time
  double energyToDecreaseJ = (totalCurrentA * m_supplyVoltageV * duration.GetNanoSeconds ()) / 1e9;
  NS_ASSERT (m_remainingEnergyJ >= energyToDecreaseJ);
  m_remainingEnergyJ -= energyToDecreaseJ;
  NS_LOG_INFO ("UavEnergySource:Remaining energy = " << m_remainingEnergyJ);
}

void
UavEnergySource::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  UpdateEnergySource ();  // start periodic update
}

void UavEnergySource::DoDispose() {
  NS_LOG_DEBUG ("UavEnergySource::DoDispose REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
  m_energyUpdateEvent.Cancel();
}

void UavEnergySource::Reset () {
  NS_ASSERT(m_node != NULL);
  m_lastUpdateTime = Simulator::Now();
  m_depleted = false;
  m_lastPosition = m_node->GetObject<MobilityModel>()->GetPosition();
  m_remainingEnergyJ = m_initialEnergyJ;
  NotifyEnergyRecharged();
}

void UavEnergySource::Start () {
  NS_ASSERT(m_node != NULL);
  Ptr<MobilityModel> mob = m_node->GetObject<MobilityModel>();
  NS_ASSERT(mob != NULL);
  m_lastPosition = mob->GetPosition();
}

void UavEnergySource::Stop () {
  m_depleted = true;
  HandleEnergyDrainedEvent();
}

} // namespace ns3
