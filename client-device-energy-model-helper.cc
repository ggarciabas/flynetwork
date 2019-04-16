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

#include "client-device-energy-model-helper.h"
#include "ns3/config.h"
#include "ns3/names.h"
#include "uav-application.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ClientDeviceEnergyModelHelper");
NS_OBJECT_ENSURE_REGISTERED(ClientDeviceEnergyModelHelper);

TypeId
ClientDeviceEnergyModelHelper::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ClientDeviceEnergyModelHelper")
                          .SetParent<Object>()
                          .SetGroupName("Flynetwork-Energy")
                          .AddConstructor<ClientDeviceEnergyModelHelper>();
  return tid;
}

ClientDeviceEnergyModelHelper::ClientDeviceEnergyModelHelper()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_energyModel.SetTypeId("ns3::ClientDeviceEnergyModel");
}

ClientDeviceEnergyModelHelper::~ClientDeviceEnergyModelHelper()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

void ClientDeviceEnergyModelHelper::Set(std::string name, const AttributeValue &v)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<name<<&v);
  m_energyModel.Set(name, v);
}

DeviceEnergyModelContainer
ClientDeviceEnergyModelHelper::Install(Ptr<NetDevice> device,
                                    Ptr<EnergySource> source) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() << device << source);
  NS_ASSERT(device != NULL);
  NS_ASSERT(source != NULL);
  // check to make sure source and net node are on the same node
  NS_ASSERT(device->GetNode() == source->GetNode());
  DeviceEnergyModelContainer container(DoInstall(device, source));
  return container;
}

DeviceEnergyModelContainer
ClientDeviceEnergyModelHelper::Install(NetDeviceContainer devContainer,
                                    EnergySourceContainer sourceContainer) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_ASSERT(devContainer.GetN() <= sourceContainer.GetN());
  DeviceEnergyModelContainer container;
  NetDeviceContainer::Iterator dev = devContainer.Begin();
  EnergySourceContainer::Iterator src = sourceContainer.Begin();
  while (dev != devContainer.End())
  {
    // check to make sure source and net node are on the same node
    NS_ASSERT((*dev)->GetNode() == (*src)->GetNode());
    Ptr<ClientDeviceEnergyModel> model = DoInstall(*dev, *src);
    container.Add(model);
    dev++;
    src++;
  }
  return container;
}

Ptr<ClientDeviceEnergyModel>
ClientDeviceEnergyModelHelper::DoInstall(Ptr<NetDevice> device, Ptr<EnergySource> source) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() << device << source);
  NS_ASSERT (device != NULL);
  NS_ASSERT (source != NULL);
  // check if device is WifiNetDevice
  std::string deviceName = device->GetInstanceTypeId ().GetName ();
  if (deviceName.compare ("ns3::WifiNetDevice") != 0)
    {
      NS_FATAL_ERROR ("NetDevice type is not WifiNetDevice!");
    }
  Ptr<Node> node = device->GetNode ();
  Ptr<ClientDeviceEnergyModel> model = m_energyModel.Create()->GetObject<ClientDeviceEnergyModel>();
  NS_ASSERT(model != NULL);

  // set energy source
  model->SetEnergySource(source);
  DynamicCast<UavEnergySource>(source)->SetCliDeviceEnergyModel (model); // deveria se utilizar o appendDeviceEnergymodel para agregar ao energy source, mas ocorre um erro!
  model->SetNode(node);
  return model;
}

} // namespace ns3
