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

#include "client-device-energy-model.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "uav-energy-source.h"
#include "ns3/log.h"

#include "uav-energy-source.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ClientDeviceEnergyModel");

NS_OBJECT_ENSURE_REGISTERED(ClientDeviceEnergyModel);

TypeId
ClientDeviceEnergyModel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ClientDeviceEnergyModel")
                          .SetParent<DeviceEnergyModel>()
                          .SetGroupName("Flynetwork-Energy")
                          .AddConstructor<ClientDeviceEnergyModel>()
                          .AddAttribute("PathData",
                                        "Name of scenario",
                                        StringValue("none"),
                                        MakeStringAccessor(&ClientDeviceEnergyModel::m_pathData),
                                        MakeStringChecker())
                          .AddAttribute("ClientCost",
                                        "Cost for each client by seconds",
                                        DoubleValue(0.34), // J/s
                                        MakeDoubleAccessor(&ClientDeviceEnergyModel::m_clientCost),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("PeriodicEnergyUpdateInterval",
                                        "",
                                        TimeValue(Seconds(0.01)), // s
                                        MakeTimeAccessor(&ClientDeviceEnergyModel::m_energyUpdateInterval),
                                        MakeTimeChecker())
                         .AddTraceSource ("TotalEnergyConsumption",
                                          "Total energy consumption of the radio device.",
                                          MakeTraceSourceAccessor (&ClientDeviceEnergyModel::m_totalEnergyConsumption),
                                          "ns3::TracedValueCallback::Double");
  return tid;
}

ClientDeviceEnergyModel::ClientDeviceEnergyModel()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_totalEnergyConsumption = 0.0;
  m_clientCount = 0;
}

ClientDeviceEnergyModel::~ClientDeviceEnergyModel()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void ClientDeviceEnergyModel::SetEnergyDepletionCallback(EnergyCallback callback)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<&callback);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyDepletionCallback:Setting NULL energy depletion callback!");
  }
  m_energyDepletionCallback = callback;
}

void ClientDeviceEnergyModel::SetEnergyRechargedCallback(EnergyCallback callback)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<&callback);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyRechargedCallback:Setting NULL energy Recharged callback!");
  }
  m_energyRechargedCallback = callback;
}

void ClientDeviceEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  if (!m_energyRechargedCallback.IsNull()) m_energyRechargedCallback();
  m_cliEvent.Cancel();
  m_totalEnergyConsumption = 0;
  m_clientCount = 0;
  this->m_lastUpdateTime = Simulator::Now ();
  m_cliEvent = Simulator::Schedule(Seconds(m_energyUpdateInterval), &ClientDeviceEnergyModel::ClientConsumption, this);
}

void ClientDeviceEnergyModel::HandleEnergyOff(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void ClientDeviceEnergyModel::HandleEnergyOn (void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void ClientDeviceEnergyModel::HandleEnergyChanged(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void ClientDeviceEnergyModel::HandleEnergyDepletion(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  if (!m_energyDepletionCallback.IsNull())  m_energyDepletionCallback();
  m_clientCount = 0;
  m_cliEvent.Cancel();
  ClientConsumption(); // update battery
  m_cliEvent.Cancel(); // cancel again, to do not allow any consumption from clients
}

void ClientDeviceEnergyModel::SetEnergyUpdateInterval(Time interval)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << interval);
  m_energyUpdateInterval = interval;
}

Time ClientDeviceEnergyModel::GetEnergyUpdateInterval(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_energyUpdateInterval;
}

void ClientDeviceEnergyModel::SetEnergySource(Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << source);
  NS_ASSERT(source != NULL);
  m_source = source;
}

void ClientDeviceEnergyModel::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << node);
  NS_ASSERT(node != NULL);
  m_node = node;
}

Ptr<Node>
ClientDeviceEnergyModel::GetNode() const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_node;
}

double ClientDeviceEnergyModel::UpdateConsumption () {
  // NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  Time duration = Simulator::Now () - m_lastUpdateTime;
  double energyToDecrease = duration.GetSeconds () * (m_clientCost * m_clientCount);
  DynamicCast<UavEnergySource> (m_source)->UpdateEnergySourceClient (energyToDecrease);
  // update last update time stamp
  this->m_lastUpdateTime = Simulator::Now ();
  m_totalEnergyConsumption += energyToDecrease;
  return energyToDecrease;
}

double 
ClientDeviceEnergyModel::CalculateThreshold () {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return ((m_clientCost * m_clientCount)*m_energyUpdateInterval.GetSeconds()) / m_source->GetInitialEnergy(); // % necessaria para suprir os clientes durante o intervalo
}

double
ClientDeviceEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

void ClientDeviceEnergyModel::AddClient ()
{
  // NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_cliEvent.Cancel();
  ClientConsumption(); // update battery
  m_clientCount++;
}

void ClientDeviceEnergyModel::RemoveClient()
{
  // NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_cliEvent.Cancel();
  ClientConsumption(); // update battery
  m_clientCount--;
}

void ClientDeviceEnergyModel::ClientConsumption ()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() << m_clientCount << m_node->GetId());
  m_cliEvent.Cancel();
  double energyToDecrease =  UpdateConsumption();
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_pathData << "/uav_client_" << m_node->GetId() << ".txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << energyToDecrease / m_source->GetInitialEnergy() << std::endl;
  m_file.close();
  m_cliEvent = Simulator::Schedule(Seconds(m_energyUpdateInterval), &ClientDeviceEnergyModel::ClientConsumption, this);
}

void ClientDeviceEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG("ClientDeviceEnergyModel::DoDispose");
  m_source = 0;
  m_cliEvent.Cancel();
}

void ClientDeviceEnergyModel::DoInitialize (void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();
  m_cliEvent = Simulator::Schedule(Seconds(m_energyUpdateInterval), &ClientDeviceEnergyModel::ClientConsumption, this);
}

double
ClientDeviceEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return 0.0; // calculado de forma diferenciada para movimentacao
}

} // namespace ns3
