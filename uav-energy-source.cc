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
                                        DoubleValue(156960), // 156960 in Joules
                                        MakeDoubleAccessor(&UavEnergySource::SetInitialEnergy, &UavEnergySource::GetInitialEnergy),
                                        MakeDoubleChecker<double>())
                          .AddAttribute ("BasicEnergySupplyVoltageV",
                                         "Initial supply voltage for basic energy source.",
                                         DoubleValue (11.4), // in Volts
                                         MakeDoubleAccessor (&UavEnergySource::SetSupplyVoltage,
                                                             &UavEnergySource::GetSupplyVoltage),
                                         MakeDoubleChecker<double> ())
                                      // TODO: o valor de threshold deve ser dinamico em relacao do custo necessario para ele voltar a central de onde ele está
                          .AddAttribute ("BasicEnergyLowBatteryThresholdUav",
                                        "Low battery threshold uav device for basic energy source.",
                                        DoubleValue(0.1), // as a fraction of the initial energy
                                        MakeDoubleAccessor(&UavEnergySource::m_lowBatteryThUav),
                                        MakeDoubleChecker<double>())
                          .AddAttribute ("BasicEnergyHighBatteryThresholdUav",
                                        "HIgh battery threshold uav device for basic energy source.",
                                        DoubleValue(0.1), // as a fraction of the initial energy
                                        MakeDoubleAccessor(&UavEnergySource::m_highBatteryTh),
                                        MakeDoubleChecker<double>())
                        .AddAttribute ("PeriodicEnergyUpdateInterval",
                                       "Time between two consecutive periodic energy updates.",
                                       TimeValue (Seconds (1.2)),
                                       MakeTimeAccessor (&UavEnergySource::SetEnergyUpdateInterval,
                                                         &UavEnergySource::GetEnergyUpdateInterval),
                                       MakeTimeChecker ())
                         .AddAttribute("PathData",
                                       "Name of scenario",
                                       StringValue(""),
                                       MakeStringAccessor(&UavEnergySource::m_pathData),
                                       MakeStringChecker())
                         .AddTraceSource("RemainingEnergy",
                                          "Remaining energy at UavEnergySource (miliamperesecond - mAs).",
                                          MakeTraceSourceAccessor(&UavEnergySource::m_remainingEnergyJ),
                                          "ns3::TracedValueCallback::Double");
  return tid;
}

UavEnergySource::UavEnergySource()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_lastUpdateTime = Seconds (0.0);
  m_depleted = false;
  m_node = 0;

  m_movAcum = 0.0;
  m_hoverAcum = 0.0;
  m_wifiAcum = 0.0;
  m_wifiEnergy = m_moveEnergy = m_hoverEnergy = 0.0;
  m_wifiTE = m_moveTE = m_hoverTE = 0.0;

  m_wifiMean = 0.0;
}

UavEnergySource::~UavEnergySource()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void UavEnergySource::SetInitialEnergy(double initialEnergyJ)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << initialEnergyJ);
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

void
UavEnergySource::SetBasicEnergyLowBatteryThresholdUav (double thr)
{
  NS_LOG_FUNCTION (this << thr);
  NS_LOG_DEBUG ("UavEnergySource::SetBasicEnergyLowBatteryThreshold [" << m_node->GetId() << "] thr: " << thr << "% in joules: " << thr*m_initialEnergyJ << "J @" << Simulator::Now().GetSeconds());
  m_lowBatteryThUav = thr;
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
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
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
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_initialEnergyJ*2; // problemas com wifi 
}

double
UavEnergySource::GetRealRemainingEnergy()
{
  return m_remainingEnergyJ; // metodo fake para evitar problemas com wifi
}

double
UavEnergySource::GetEnergyFraction (void)
{
  NS_LOG_FUNCTION (this);
  return m_remainingEnergyJ / m_initialEnergyJ;
}

void
UavEnergySource::UpdateEnergySource (void) 
{
  NS_LOG_FUNCTION (this);
}

double UavEnergySource::UpdateEnergySourceClient (double time)
{  
  double energyToDecrease = 0.0;
  if (m_onoff && !m_depleted) { // calcula somente se estiver ligada e fora de depletion
    // TODO_NEW: considerar calculo que o wifi utiliza: double energyToDecreaseJ = (totalCurrentA * m_supplyVoltageV * duration.GetNanoSeconds ()) / 1e9;
    // utilizar os valores especificos para o wifi

    energyToDecrease = (time * global_tx_current * m_supplyVoltageV);

    m_wifiMean = (energyToDecrease + m_wifiMean) / 2.0;
    m_highBatteryTh = m_lowBatteryThUav + (((m_wifiMean*0.5*100)/m_initialEnergyJ)/global_cli_cons_update)/* gasto por segundo */*DynamicCast<UavDeviceEnergyModel>(m_uavDev)->GetTimeToCentral();  // 50% do consumo medio considerado para o high multiplicado pelo tempo de deslocamento da central para o ponto do UAV avaliado. No high é enviado um pedido para a central de um novo UAV.

    m_remainingEnergyJ -= energyToDecrease;

    m_wifiEnergy += energyToDecrease;
    m_wifiTE += energyToDecrease;

    if (m_remainingEnergyJ <= (m_highBatteryTh)*m_initialEnergyJ && !m_highThr) {
      HandleAskUav();
      m_highThr = true;
    }

    if (!m_depleted && m_remainingEnergyJ <= (m_lowBatteryThUav)*m_initialEnergyJ)
    { // no LOW o UAV deixa a posicao 
      NS_LOG_DEBUG("UavEnergySource::UpdateEnergySourceClient DEPLETION [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav) * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
      m_depleted = true;
      HandleEnergyDrainedEvent();
    }
  }
  return energyToDecrease;
}

void UavEnergySource::UpdateEnergySourceMove (double energyToDecrease)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << energyToDecrease);
  if (m_onoff) { // calcula somente se estiver ligada
    m_remainingEnergyJ -= energyToDecrease;

    m_moveEnergy += energyToDecrease;
    m_moveTE += energyToDecrease;

    if (m_remainingEnergyJ <= (m_highBatteryTh)*m_initialEnergyJ && !m_highThr) {
      NS_LOG_DEBUG("High Threshold");
      HandleAskUav();
      m_highThr = true;
    }

    if (!m_depleted && m_remainingEnergyJ <= (m_lowBatteryThUav)*m_initialEnergyJ)
    {
      NS_LOG_DEBUG("UavEnergySource::UpdateEnergySourceMove DEPLETION [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav) * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
      m_depleted = true;
      HandleEnergyDrainedEvent();
    }
  }
}

void UavEnergySource::SetDeviceEnergyModel (Ptr<DeviceEnergyModel> dev) {
  m_uavDev = dev;
  m_uavDevModel = DynamicCast<UavDeviceEnergyModel>(dev);
}

void UavEnergySource::UpdateEnergySourceHover (double energyToDecrease)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << energyToDecrease);
  if (m_onoff) { // calcula somente se estiver ligada
    m_remainingEnergyJ -= energyToDecrease;

    m_hoverEnergy += energyToDecrease;
    m_hoverTE += energyToDecrease;

    if (m_remainingEnergyJ <= (m_highBatteryTh)*m_initialEnergyJ && !m_highThr) {
      HandleAskUav();
      m_highThr = true;
    }

    if (!m_depleted && m_remainingEnergyJ <= (m_lowBatteryThUav)*m_initialEnergyJ)
    {
      NS_LOG_DEBUG("UavEnergySource::UpdateEnergySourceHover DEPLETION  [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav) * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
      m_depleted = true;
      HandleEnergyDrainedEvent();
    }
  }
}

/*
 * Private functions start here.
 */

void UavEnergySource::HandleAskUav ()
{
  NS_LOG_DEBUG ("Pedindo novo uav");
  if (m_uavDev != NULL)
    DynamicCast<UavDeviceEnergyModel>(m_uavDev)->HandleAskUav();
}

void UavEnergySource::HandleEnergyDrainedEvent(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_ASSERT(m_onoff);
  NS_LOG_INFO("UavEnergySource:Energy depleted!");
  // NotifyEnergyDrained(); // notify DeviceEnergyModel objects ??? retirar mesmo? para nao desligar wifi?
  if (m_uavDev != NULL)
    m_uavDev->HandleEnergyDepletion(); // deveria se utilizar o energysource container, porem ocorre um erro não analisado
}

void
UavEnergySource::CalculateRemainingEnergy (void)
{
  NS_LOG_FUNCTION (this);
}

void
UavEnergySource::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
}

void UavEnergySource::DoDispose() {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("UavEnergySource::DoDispose REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
  m_energyUpdateEvent.Cancel();
}

void UavEnergySource::Start () {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_onoff = true;
  NS_ASSERT(m_node != NULL);
  m_lastUpdateTime = Simulator::Now();
  m_depleted = false;
  m_highThr = false;
  m_wifiMean = 0.0;
  m_lastPosition = m_node->GetObject<MobilityModel>()->GetPosition();
  m_remainingEnergyJ = m_initialEnergyJ;
  m_movAcum = 0.0;
  m_wifiAcum = 0.0;
  m_hoverAcum = 0.0;
  m_wifiEnergy = m_moveEnergy = m_hoverEnergy = 0.0;
  m_lowBatteryThUav = 0.1;
  if (m_uavDev != NULL)
    m_uavDev->HandleEnergyRecharged(); // deveria se utilizar o energy source container, porem erro!
  // NotifyEnergyRecharged();
}

void UavEnergySource::TimeEnergy (double next) {
  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_energy/uav_timing_energy_" << m_node->GetId() << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  
  file << Simulator::Now().GetSeconds() << " " << m_node->GetId() << " " << m_initialEnergyJ << " " << m_remainingEnergyJ << " " <<  m_wifiTE << " " << m_moveTE << " " << m_hoverTE << " " << ((m_depleted)?"TRUE ":"FALSE ") << " " << m_wifiMean << " " << m_lowBatteryThUav << " " << m_highBatteryTh << std::endl;
  file.close();

  m_wifiTE = m_moveTE = m_hoverTE = 0.0;
  m_timeEnergy = Simulator::Schedule (Seconds(next), &UavEnergySource::TimeEnergy, this, next);
}

void UavEnergySource::Stop () {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_onoff = false;
  if (m_uavDev != NULL)
    m_uavDev->HandleEnergyOff(); // deveria se utilizar o energy source container, porem erro!

  m_wifiEnergy = m_moveEnergy = m_hoverEnergy = 0.0;

}

} // namespace ns3
