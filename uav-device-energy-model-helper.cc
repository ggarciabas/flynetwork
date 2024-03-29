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

#include "uav-device-energy-model-helper.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "uav-application.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavDeviceEnergyModelHelper");

NS_OBJECT_ENSURE_REGISTERED(UavDeviceEnergyModelHelper);

TypeId
UavDeviceEnergyModelHelper::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::UavDeviceEnergyModelHelper")
                          .SetParent<Object>()
                          .SetGroupName("Flynetwork-Energy")
                          .AddConstructor<UavDeviceEnergyModelHelper>();
  return tid;
}

UavDeviceEnergyModelHelper::UavDeviceEnergyModelHelper()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_energyModel.SetTypeId("ns3::UavDeviceEnergyModel");
}

UavDeviceEnergyModelHelper::~UavDeviceEnergyModelHelper()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void UavDeviceEnergyModelHelper::Set(std::string name, const AttributeValue &v)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << name << &v);
  m_energyModel.Set(name, v);
}

DeviceEnergyModelContainer
UavDeviceEnergyModelHelper::Install(Ptr<Node> node,
                                    Ptr<EnergySource> source) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << node << source);
  NS_ASSERT(node != NULL);
  NS_ASSERT(source != NULL);
  // check to make sure source and net node are on the same node
  NS_ASSERT(node == source->GetNode());
  DeviceEnergyModelContainer container(DoInstall(node, source));
  return container;
}

DeviceEnergyModelContainer
UavDeviceEnergyModelHelper::Install(NodeContainer nodeContainer,
                                    EnergySourceContainer sourceContainer) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_ASSERT(nodeContainer.GetN() <= sourceContainer.GetN());
  DeviceEnergyModelContainer container;
  NodeContainer::Iterator node = nodeContainer.Begin();
  EnergySourceContainer::Iterator src = sourceContainer.Begin();
  while (node != nodeContainer.End())
  {
    // check to make sure source and net node are on the same node
    NS_ASSERT(*node == (*src)->GetNode());
    Ptr<UavDeviceEnergyModel> model = DoInstall(*node, *src);
    container.Add(model);
    node++;
    src++;
  }
  return container;
}

void UavDeviceEnergyModelHelper::SetEnergyDepletionCallback (UavDeviceEnergyModel::EnergyCallback cb) 
{
  m_depletionCallback = cb;
}

void UavDeviceEnergyModelHelper::SetEnergyRechargedCallback (UavDeviceEnergyModel::EnergyCallback cb)
{
  m_rechargedCallback = cb;
}

Ptr<UavDeviceEnergyModel>
UavDeviceEnergyModelHelper::DoInstall(Ptr<Node> node, Ptr<EnergySource> source) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << node << source);
  NS_ASSERT(node != NULL);
  NS_ASSERT(source != NULL);
  Ptr<UavDeviceEnergyModel> model = m_energyModel.Create()->GetObject<UavDeviceEnergyModel>();
  NS_ASSERT(model != NULL);
  // set energy source pointer
  // buscar o objeto UavApplication do nó
  if (!m_depletionCallback.IsNull()) 
    model->SetEnergyDepletionCallback(m_depletionCallback);
  else 
    NS_FATAL_ERROR("Callback Depletion is null");
  if (!m_rechargedCallback.IsNull()) 
    model->SetEnergyRechargedCallback(m_rechargedCallback);
  else 
    NS_FATAL_ERROR("Callback Recharged is null");

  // add model to device model list in energy source
  // source->AppendDeviceEnergyModel(model);
  DynamicCast<UavEnergySource>(source)->SetDeviceEnergyModel (model); // deveria se utilizar o appendDeviceEnergymodel para agregar ao energy source, mas ocorre um erro! por conta da agregacao abaixo, mas esta é necessaria para que se possa obter os devices de energia e fazer configuracoes!
  // set energy source
  model->SetEnergySource(source);
  return model;
}

} // namespace ns3
