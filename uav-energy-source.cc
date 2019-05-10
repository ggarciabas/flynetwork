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
                                        DoubleValue(200), // 156960 in Joules
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
                          .AddAttribute ("BasicEnergyLowBatteryThresholdCli",
                                        "Low battery threshold client device for basic energy source.",
                                        DoubleValue(0.1), // as a fraction of the initial energy
                                        MakeDoubleAccessor(&UavEnergySource::m_lowBatteryThCli),
                                        MakeDoubleChecker<double>())
                        .AddAttribute ("PeriodicEnergyUpdateInterval",
                                       "Time between two consecutive periodic energy updates.",
                                       TimeValue (Seconds (1.2)),
                                       MakeTimeAccessor (&UavEnergySource::SetEnergyUpdateInterval,
                                                         &UavEnergySource::GetEnergyUpdateInterval),
                                       MakeTimeChecker ())
                        .AddAttribute ("UpdateThresholdInterval",
                                       "Time between two consecutive periodic threshold updates.",
                                       TimeValue (Seconds (0.5)),
                                       MakeTimeAccessor (&UavEnergySource::SetUpdateThresholdInterval,
                                                         &UavEnergySource::GetUpdateThresholdInterval),
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
  m_cliAcum = 0.0;
  m_hoverAcum = 0.0;
  m_wifiAcum = 0.0;
  m_wifiEnergy = m_clientEnergy = m_moveEnergy = m_hoverEnergy = 0.0;
  m_wifiTE = m_clientTE = m_moveTE = m_hoverTE = 0.0;
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
UavEnergySource::SetUpdateThresholdInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_updateThrTime = interval;
}

void 
UavEnergySource::UpdateThreshold ()
{
  NS_LOG_FUNCTION (this);
  m_lowBatteryThCli = m_cliDevModel->CalculateThreshold(m_uavDevModel->GetTimeToCentral());
  // std::ostringstream os;
  // os << global_path << "/" << m_pathData << "/uav_energy_threshold/uav_energy_threshold_" << m_node->GetId() << ".txt";
  // m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  // m_file << Simulator::Now().GetSeconds() << "," << m_lowBatteryTh << std::endl;
  // m_file.close();
  m_updateThr = Simulator::Schedule(m_updateThrTime, &UavEnergySource::UpdateThreshold, this);
}

void
UavEnergySource::SetBasicEnergyLowBatteryThresholdUav (double thr)
{
  NS_LOG_FUNCTION (this << thr);
  NS_LOG_DEBUG ("UavEnergySource::SetBasicEnergyLowBatteryThreshold [" << m_node->GetId() << "] thr: " << thr << "% in joules: " << thr*m_initialEnergyJ << "J @" << Simulator::Now().GetSeconds());
  m_lowBatteryThUav = thr;
  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_energy_threshold/uav_energy_threshold_" << m_node->GetId() << ".txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << m_lowBatteryThUav << std::endl;
  m_file.close();
}

Time
UavEnergySource::GetEnergyUpdateInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_energyUpdateInterval;
}

Time
UavEnergySource::GetUpdateThresholdInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_updateThrTime;
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
UavEnergySource::UpdateEnergySource (void) // chamado pelo device wifi-radio-energy-model
{
  NS_LOG_FUNCTION (this);
  
  if (m_onoff && !m_depleted) { // calcula somente se estiver ligada    
    double remainingEnergy = m_remainingEnergyJ;
    CalculateRemainingEnergy ();

    m_wifiEnergy += remainingEnergy-m_remainingEnergyJ;
    m_wifiTE += remainingEnergy-m_remainingEnergyJ;
    m_hoverTE += remainingEnergy-m_remainingEnergyJ;

    m_lastUpdateTime = Simulator::Now ();

    // if (m_remainingEnergyJ <= 0)
    // {
    //   NS_FATAL_ERROR("UavEnergySource::UpdateEnergySource energy bellow ZERO! [" << m_node->GetId() << "] ed: " << remainingEnergy-m_remainingEnergyJ << " re: " << remainingEnergy << "J @" << Simulator::Now().GetSeconds());
    // }

    if (!m_depleted && m_remainingEnergyJ <= (m_lowBatteryThUav+m_lowBatteryThCli)*2*m_initialEnergyJ)
    {
      NS_LOG_DEBUG("UavEnergySource::UpdateEnergySource DEPLETION [" << m_node->GetId() << "] ed: " << remainingEnergy-m_remainingEnergyJ << " re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav+m_lowBatteryThCli)*2*m_initialEnergyJ << "J @" << Simulator::Now().GetSeconds());
      m_depleted = true;
      HandleEnergyDrainedEvent();
    }
    
    // salvando historico do consumo de bateria por wifidev
    if (m_node) {
      std::ostringstream os;
      #ifdef LOG_ENERGY
        os << global_path << "/" << m_pathData << "/uav_wifi/uav_wifi_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_wifiAcum += (remainingEnergy-m_remainingEnergyJ);
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << remainingEnergy-m_remainingEnergyJ << " " << m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ACUM
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_wifi/uav_wifi_acum_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_wifiAcum += (remainingEnergy-m_remainingEnergyJ);
        m_file << Simulator::Now().GetSeconds() << "," << m_wifiAcum/m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ENERGY_ALL
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_remaining_energy/uav_remaining_energy_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << remainingEnergy-m_remainingEnergyJ  << " wifi " << m_initialEnergyJ << " " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 << std::endl;
        m_file.close();
      #endif
    }
  } 
}

void UavEnergySource::UpdateEnergySourceClient (double energyToDecrease)
{  
  if (m_onoff && !m_depleted) { // calcula somente se estiver ligada e fora de depletion
    // if (m_remainingEnergyJ < energyToDecrease)
    // {
    //   NS_FATAL_ERROR("UavEnergySource::UpdateEnergySourceClient energy bellow ZERO! [" << m_node->GetId() << "] " << m_remainingEnergyJ << "J ____  " << energyToDecrease << "J @" << Simulator::Now().GetSeconds());
    // }
    // else
    {
      m_remainingEnergyJ -= energyToDecrease;
    }

    m_clientEnergy += energyToDecrease;
    m_clientTE += energyToDecrease;

    if (!m_depleted && m_remainingEnergyJ <= (m_lowBatteryThUav+m_lowBatteryThCli)*2*m_initialEnergyJ)
    {
      NS_LOG_DEBUG("UavEnergySource::UpdateEnergySourceClient DEPLETION [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
      m_depleted = true;
      HandleEnergyDrainedEvent();
    }

    // salvando historico do consumo de bateria por movimentacao
    if (m_node) {
      std::ostringstream os;
      #ifdef LOG_ENERGY
        os << global_path << "/" << m_pathData << "/uav_client/uav_client_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_cliAcum += energyToDecrease;
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << energyToDecrease << " " << m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ACUM
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_client/uav_client_acum_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_cliAcum += energyToDecrease;
        m_file << Simulator::Now().GetSeconds() << "," << m_cliAcum / m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ENERGY_ALL
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_remaining_energy/uav_remaining_energy_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << energyToDecrease  << " client " << m_initialEnergyJ << " " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 << std::endl;
        m_file.close();
      #endif
    }
  }
}

void UavEnergySource::UpdateEnergySourceMove (double energyToDecrease)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << energyToDecrease);
  if (m_onoff) { // calcula somente se estiver ligada
    // if (m_remainingEnergyJ < energyToDecrease)
    // {
    //   NS_FATAL_ERROR("UavEnergySource::UpdateEnergySourceMove energy bellow ZERO! [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
    // }
    // else
    {
      m_remainingEnergyJ -= energyToDecrease;
    }

    m_moveEnergy += energyToDecrease;
    m_moveTE += energyToDecrease;

    if (!m_depleted && m_remainingEnergyJ <= (m_lowBatteryThUav+m_lowBatteryThCli)*2*m_initialEnergyJ)
    {
      NS_LOG_DEBUG("UavEnergySource::UpdateEnergySourceMove DEPLETION [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
      m_depleted = true;
      HandleEnergyDrainedEvent();
    }
    // salvando historico do consumo de bateria por movimentacao
    if (m_node) {
      std::ostringstream os;
      #ifdef LOG_ENERGY
        os << global_path << "/" << m_pathData << "/uav_move/uav_move_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_movAcum += energyToDecrease;
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << energyToDecrease << " " << m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ACUM
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_move/uav_move_acum_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_movAcum += energyToDecrease;
        m_file << Simulator::Now().GetSeconds() << "," << m_movAcum / m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ENERGY_ALL 
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_remaining_energy/uav_remaining_energy_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << energyToDecrease  << " move " << m_initialEnergyJ << " " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 << std::endl;
        m_file.close();
      #endif
    }
  }
}

void UavEnergySource::SetDeviceEnergyModel (Ptr<DeviceEnergyModel> dev) {
  m_uavDev = dev;
  m_uavDevModel = DynamicCast<UavDeviceEnergyModel>(dev);
}

void UavEnergySource::SetCliDeviceEnergyModel (Ptr<DeviceEnergyModel> dev) {
  m_cliDev = dev;
  m_cliDevModel = DynamicCast<ClientDeviceEnergyModel>(dev);
  m_updateThr = Simulator::Schedule(m_updateThrTime, &UavEnergySource::UpdateThreshold, this);
}

void UavEnergySource::UpdateEnergySourceHover (double energyToDecrease)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << energyToDecrease);
  if (m_onoff) { // calcula somente se estiver ligada
    // if (m_remainingEnergyJ < energyToDecrease)
    // {
    //   NS_FATAL_ERROR("UavEnergySource::UpdateEnergySourceHover energy bellow ZERO! [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
    // }
    // else
    {
      m_remainingEnergyJ -= energyToDecrease;
    }

    m_hoverEnergy += energyToDecrease;
    m_hoverTE += energyToDecrease;

    if (!m_depleted && m_remainingEnergyJ <= (m_lowBatteryThUav+m_lowBatteryThCli)*2*m_initialEnergyJ)
    {
      NS_LOG_DEBUG("UavEnergySource::UpdateEnergySourceHover DEPLETION  [" << m_node->GetId() << "] ed: " << energyToDecrease << "J re: " << m_remainingEnergyJ << "J thr: " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 * m_initialEnergyJ<< "J @" << Simulator::Now().GetSeconds());
      m_depleted = true;
      HandleEnergyDrainedEvent();
    }

    // salvando historico do consumo de bateria por movimentacao
    if (m_node) {
      std::ostringstream os;
      #ifdef LOG_ENERGY
        os << global_path << "/" << m_pathData << "/uav_hover/uav_hover_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_hoverAcum += energyToDecrease;
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << energyToDecrease << " " << m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ACUM
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_hover/uav_hover_acum_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_hoverAcum += energyToDecrease;
        m_file << Simulator::Now().GetSeconds() << "," << m_hoverAcum / m_initialEnergyJ << std::endl;
        m_file.close();
      #endif
      #ifdef LOG_ENERGY_ALL
        os.str("");
        os << global_path << "/" << m_pathData << "/uav_remaining_energy/uav_remaining_energy_" << m_node->GetId() << ".txt";
        m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
        m_file << Simulator::Now().GetSeconds() << " " << m_remainingEnergyJ << " " << energyToDecrease  << " hover " << m_initialEnergyJ << " " << (m_lowBatteryThUav+m_lowBatteryThCli)*2 << std::endl;
        m_file.close();
      #endif
    }
  }
}

/*
 * Private functions start here.
 */

void UavEnergySource::HandleEnergyDrainedEvent(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_ASSERT(m_onoff);
  NS_LOG_INFO("UavEnergySource:Energy depleted!");
  // NotifyEnergyDrained(); // notify DeviceEnergyModel objects ??? retirar mesmo? para nao desligar wifi?
  if (m_cliDev != NULL)
    m_cliDev->HandleEnergyDepletion();
  if (m_uavDev != NULL)
    m_uavDev->HandleEnergyDepletion(); // deveria se utilizar o energysource container, porem ocorre um erro não analisado
}

void
UavEnergySource::CalculateRemainingEnergy (void)
{
  NS_LOG_FUNCTION (this);
  if (m_onoff) { // calcula somente se estiver ligada
    double totalCurrentA = CalculateTotalCurrent (); // busca nos devices o total de 'current'
    Time duration = Simulator::Now () - m_lastUpdateTime;
    NS_ASSERT (duration.IsPositive ());
    // energy = current * voltage * time
    double energyToDecreaseJ = (totalCurrentA * m_supplyVoltageV * duration.GetNanoSeconds ()) / 1e9;
    if (m_remainingEnergyJ <= energyToDecreaseJ) 
      NS_LOG_DEBUG ("UavEnergySource:CalculateRemainingEnergy rem energy = " << m_remainingEnergyJ << " energytoDecrease " << energyToDecreaseJ << " node " << m_node->GetId());
    // NS_ASSERT (m_remainingEnergyJ >= energyToDecreaseJ);
    m_remainingEnergyJ -= energyToDecreaseJ;
  }
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
  m_lastPosition = m_node->GetObject<MobilityModel>()->GetPosition();
  m_remainingEnergyJ = m_initialEnergyJ;
  m_movAcum = 0.0;
  m_cliAcum = 0.0;
  m_wifiAcum = 0.0;
  m_hoverAcum = 0.0;
  m_wifiEnergy = m_clientEnergy = m_moveEnergy = m_hoverEnergy = 0.0;
  m_lowBatteryThUav = 0.1;
  m_lowBatteryThCli = 0.1;
  if (m_cliDev != NULL)
    m_cliDev->HandleEnergyRecharged(); // deveria se utilizar o energy source container, porem erro!
  if (m_uavDev != NULL)
    m_uavDev->HandleEnergyRecharged(); // deveria se utilizar o energy source container, porem erro!
  // NotifyEnergyRecharged();

  m_timeEnergy = Simulator::Schedule (Seconds(10.0), &UavEnergySource::TimeEnergy, this);
}

void UavEnergySource::TimeEnergy () {
  m_timeEnergy.Cancel();
  std::ostringstream os;
  os << global_path << "/" << m_pathData << "/uav_energy/uav_timing_energy_" << m_node->GetId() << ".txt";
  std::ofstream file;
  file.open(os.str(), std::ofstream::out | std::ofstream::app);
  
  file << Simulator::Now().GetSeconds() << " " << m_node->GetId() << " " << m_initialEnergyJ << " " << m_remainingEnergyJ << " " <<  m_wifiTE << " " << m_clientTE << " " << m_moveTE << " " << m_hoverTE << " " << ((m_depleted)?"TRUE ":"FALSE ") << std::endl;
  file.close();

  // if (m_wifiTE+m_clientTE+m_moveTE+m_hoverTE != (m_initialEnergyJ-m_remainingEnergyJ))
  //   std::cout << "(TE) Bateria consumida não bateu com o acumulado dos modos! node=" << m_node->GetId() << " m_initialEnergyJ=" << m_initialEnergyJ << " m_remainingEnergyJ=" << m_remainingEnergyJ << " (m_initialEnergyJ-m_remainingEnergyJ) = " << (m_initialEnergyJ-m_remainingEnergyJ) << " m_wifiTE=" <<  m_wifiTE << " m_clientTE=" << m_clientTE << " m_moveTE=" << m_moveTE << " m_hoverTE=" << m_hoverTE << std::endl;

  m_wifiTE = m_clientTE = m_moveTE = m_hoverTE = 0.0;
  m_timeEnergy = Simulator::Schedule (Seconds(10.0), &UavEnergySource::TimeEnergy, this);
}

void UavEnergySource::Stop () {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_onoff = false;
  // NotifyEnergyOff();
  if (m_cliDev != NULL)
    m_cliDev->HandleEnergyOff();
  if (m_uavDev != NULL)
    m_uavDev->HandleEnergyOff(); // deveria se utilizar o energy source container, porem erro!

  m_wifiEnergy = m_clientEnergy = m_moveEnergy = m_hoverEnergy = 0.0;

  Simulator::Remove(m_updateThr);
  Simulator::Remove(m_timeEnergy);
}

} // namespace ns3
