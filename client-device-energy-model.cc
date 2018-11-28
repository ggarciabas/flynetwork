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
                          .AddAttribute("ScenarioName",
                                        "Name of scenario",
                                        StringValue("none"),
                                        MakeStringAccessor(&ClientDeviceEnergyModel::m_scenarioName),
                                        MakeStringChecker())
                          .AddAttribute("ClientCost",
                                        "Cost for each client by seconds",
                                        DoubleValue(0.34), // J/s
                                        MakeDoubleAccessor(&ClientDeviceEnergyModel::m_clientCost),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("PeriodicEnergyUpdateInterval",
                                        "",
                                        TimeValue(Seconds(1)), // s
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
  NS_LOG_FUNCTION(this);
  m_totalEnergyConsumption = 0.0;
  m_clientCount = 0;
}

ClientDeviceEnergyModel::~ClientDeviceEnergyModel()
{
  NS_LOG_FUNCTION(this);
}

void ClientDeviceEnergyModel::SetEnergyDepletionCallback(EnergyCallback callback)
{
  NS_LOG_FUNCTION(this);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyDepletionCallback:Setting NULL energy depletion callback!");
  }
  m_energyDepletionCallback = callback;
}

void ClientDeviceEnergyModel::SetEnergyRechargedCallback(EnergyCallback callback)
{
  NS_LOG_FUNCTION(this);
  if (callback.IsNull())
  {
    NS_LOG_DEBUG("EnergyRechargedCallback:Setting NULL energy Recharged callback!");
  }
  m_energyRechargedCallback = callback;
}

void ClientDeviceEnergyModel::HandleEnergyRecharged (void)
{
  m_energyRechargedCallback();
}

void ClientDeviceEnergyModel::HandleEnergyDepletion(void)
{
  m_energyDepletionCallback();
}

void ClientDeviceEnergyModel::SetEnergyUpdateInterval(Time interval)
{
  NS_LOG_FUNCTION(this << interval);
  m_energyUpdateInterval = interval;
}

Time ClientDeviceEnergyModel::GetEnergyUpdateInterval(void) const
{
  NS_LOG_FUNCTION(this);
  return m_energyUpdateInterval;
}

void ClientDeviceEnergyModel::SetEnergySource(Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION(this << source);
  NS_ASSERT(source != NULL);
  m_source = source;
}

void ClientDeviceEnergyModel::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION(this << node);
  NS_ASSERT(node != NULL);
  m_node = node;
}

Ptr<Node>
ClientDeviceEnergyModel::GetNode() const
{
  NS_LOG_FUNCTION(this);
  return m_node;
}

void ClientDeviceEnergyModel::UpdateConsumption () {
  Time duration = Simulator::Now () - m_lastUpdateTime;
  double energyToDecrease = duration.GetSeconds () * (m_clientCost * m_clientCount);
  DynamicCast<UavEnergySource> (m_source)->UpdateEnergySource (energyToDecrease);
  // update last update time stamp
  this->m_lastUpdateTime = Simulator::Now ();
  m_totalEnergyConsumption += energyToDecrease;
}

double
ClientDeviceEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

void ClientDeviceEnergyModel::AddClient ()
{
  m_cliEvent.Cancel();
  ClientConsumption(); // update battery
  m_clientCount++;
}

void ClientDeviceEnergyModel::RemoveClient()
{
  m_cliEvent.Cancel();
  ClientConsumption(); // update battery
  m_clientCount--;
}

void ClientDeviceEnergyModel::ClientConsumption ()
{
  NS_LOG_FUNCTION(this);
  m_cliEvent.Cancel();
  UpdateConsumption();
  std::ostringstream os;
  os << "./scratch/flynetwork/data/output/" << m_scenarioName << "/client_consumption_uav_" << m_node->GetId() << ".txt";
  m_file.open(os.str(), std::ofstream::out | std::ofstream::app);
  m_file << Simulator::Now().GetSeconds() << "," << m_totalEnergyConsumption << std::endl;
  m_file.close();
  m_cliEvent = Simulator::Schedule(Seconds(m_energyUpdateInterval), &ClientDeviceEnergyModel::ClientConsumption, this);
}

void ClientDeviceEnergyModel::Reset()
{
  NS_LOG_FUNCTION (this);
  m_totalEnergyConsumption = 0;
}

void ClientDeviceEnergyModel::DoDispose (void)
{
  NS_LOG_DEBUG("ClientDeviceEnergyModel::DoDispose");
  m_source = 0;
  m_cliEvent.Cancel();
}

void ClientDeviceEnergyModel::DoInitialize (void)
{
  NS_LOG_FUNCTION(this);
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
