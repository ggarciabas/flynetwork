/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 UPB
 * Copyright (c) 2017 NITK Surathkal
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
 * Author: Radu Lupu <rlupu@elcom.pub.ro>
 *         Ankit Deepak <adadeepak8@gmail.com>
 *         Deepti Rajagopal <deeptir96@gmail.com>
 * adaptado por ggarciabas
 *
 */

#include "dhcp-helper-uav.h"
#include "dhcp-server-uav.h"
#include "ns3/internet-apps-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DhcpHelperUav");

DhcpHelperUav::DhcpHelperUav ()
{
  m_clientFactory.SetTypeId (DhcpClient::GetTypeId ());
  m_serverFactory.SetTypeId (DhcpServerUav::GetTypeId ());
}

void DhcpHelperUav::SetClientAttribute (
  std::string name,
  const AttributeValue &value)
{
  NS_LOG_FUNCTION(this<<&value);
  m_clientFactory.Set (name, value);
}

void DhcpHelperUav::SetServerAttribute (
  std::string name,
  const AttributeValue &value)
{
  NS_LOG_FUNCTION(this<<&value);
  m_serverFactory.Set (name, value);
}

ApplicationContainer DhcpHelperUav::InstallDhcpClient (Ptr<NetDevice> netDevice) const
{
  NS_LOG_FUNCTION(this);
  return ApplicationContainer (InstallDhcpClientPriv (netDevice));
}

ApplicationContainer DhcpHelperUav::InstallDhcpClient (NetDeviceContainer netDevices) const
{
  NS_LOG_FUNCTION(this);
  ApplicationContainer apps;
  for (NetDeviceContainer::Iterator i = netDevices.Begin (); i != netDevices.End (); ++i)
    {
      apps.Add (InstallDhcpClientPriv (*i));
    }
  return apps;
}

Ptr<Application> DhcpHelperUav::InstallDhcpClientPriv (Ptr<NetDevice> netDevice) const
{
  NS_LOG_FUNCTION(this<<netDevice);
  Ptr<Node> node = netDevice->GetNode ();
  NS_ASSERT_MSG (node != 0, "DhcpClientHelper: NetDevice is not not associated with any node -> fail");

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4, "DhcpHelperUav: NetDevice is associated"
                 " with a node without IPv4 stack installed -> fail "
                 "(maybe need to use InternetStackHelper?)");

  int32_t interface = ipv4->GetInterfaceForDevice (netDevice);
  if (interface == -1)
    {
      interface = ipv4->AddInterface (netDevice);
    }
  NS_ASSERT_MSG (interface >= 0, "DhcpHelperUav: Interface index not found");

  ipv4->SetMetric (interface, 1);
  ipv4->SetUp (interface);

  // Install the default traffic control configuration if the traffic
  // control layer has been aggregated, if this is not
  // a loopback interface, and there is no queue disc installed already
  Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
  if (tc && DynamicCast<LoopbackNetDevice> (netDevice) == 0 && tc->GetRootQueueDiscOnDevice (netDevice) == 0)
    {
      NS_LOG_LOGIC ("DhcpHelperUav - Installing default traffic control configuration");
      TrafficControlHelper tcHelper = TrafficControlHelper::Default ();
      tcHelper.Install (netDevice);
    }

  Ptr<DhcpClient> app = DynamicCast <DhcpClient> (m_clientFactory.Create<DhcpClient> ());
  app->SetDhcpClientNetDevice (netDevice);
  node->AddApplication (app);

  return app;
}

ApplicationContainer DhcpHelperUav::InstallDhcpServerUav (Ptr<NetDevice> netDevice, Ipv4Address serverAddr,
                                                    Ipv4Address poolAddr, Ipv4Mask poolMask,
                                                    Ipv4Address minAddr, Ipv4Address maxAddr,
                                                    Ipv4Address gateway)
{
  NS_LOG_FUNCTION(this<<netDevice<<serverAddr<<poolAddr<<poolMask<<minAddr<<maxAddr<<gateway);
  m_serverFactory.Set ("PoolAddresses", Ipv4AddressValue (poolAddr));
  m_serverFactory.Set ("PoolMask", Ipv4MaskValue (poolMask));
  m_serverFactory.Set ("FirstAddress", Ipv4AddressValue (minAddr));
  m_serverFactory.Set ("LastAddress", Ipv4AddressValue (maxAddr));
  m_serverFactory.Set ("Gateway", Ipv4AddressValue (gateway));

  // std::cout << "DhcpHelperUav::InstallDhcpServerUav " << serverAddr << " " << poolAddr << " " << poolMask << " " << minAddr << " " << maxAddr << " " << gateway << std::endl;

  Ptr<Node> node = netDevice->GetNode ();
  NS_ASSERT_MSG (node != 0, "DhcpHelperUav: NetDevice is not not associated with any node -> fail");

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4, "DhcpHelperUav: NetDevice is associated"
                 " with a node without IPv4 stack installed -> fail "
                 "(maybe need to use InternetStackHelper?)");

  int32_t interface = ipv4->GetInterfaceForDevice (netDevice);
  if (interface == -1)
    {
      interface = ipv4->AddInterface (netDevice);
    }
  NS_ASSERT_MSG (interface >= 0, "DhcpHelperUav: Interface index not found");

  Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (serverAddr, poolMask);
  ipv4->AddAddress (interface, ipv4Addr);
  ipv4->SetMetric (interface, 1);
  ipv4->SetUp (interface);

  // Install the default traffic control configuration if the traffic
  // control layer has been aggregated, if this is not
  // a loopback interface, and there is no queue disc installed already
  Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
  if (tc && DynamicCast<LoopbackNetDevice> (netDevice) == 0 && tc->GetRootQueueDiscOnDevice (netDevice) == 0)
    {
      NS_LOG_LOGIC ("DhcpHelperUav - Installing default traffic control configuration");
      TrafficControlHelper tcHelper = TrafficControlHelper::Default ();
      tcHelper.Install (netDevice);
    }

  // check that the already fixed addresses are not in conflict with the pool
  std::list<Ipv4Address>::iterator iter;
  for (iter=m_fixedAddresses.begin (); iter!=m_fixedAddresses.end (); iter ++)
    {
      if (iter->Get () >= minAddr.Get () && iter->Get () <= maxAddr.Get ())
        {
          NS_ABORT_MSG ("DhcpHelperUav: Fixed address can not conflict with a pool: " << *iter << " is in [" << minAddr << ",  " << maxAddr << "]");
        }
    }
  m_addressPools.push_back (std::make_pair (minAddr, maxAddr));

  Ptr<Application> app = m_serverFactory.Create<DhcpServerUav> ();
  node->AddApplication (app);
  return ApplicationContainer (app);
}

Ipv4InterfaceContainer DhcpHelperUav::InstallFixedAddress (Ptr<NetDevice> netDevice, Ipv4Address addr, Ipv4Mask mask)
{
  NS_LOG_FUNCTION(this<<netDevice<<addr<<mask);
  Ipv4InterfaceContainer retval;

  Ptr<Node> node = netDevice->GetNode ();
  NS_ASSERT_MSG (node != 0, "DhcpHelperUav: NetDevice is not not associated with any node -> fail");

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4, "DhcpHelperUav: NetDevice is associated"
                 " with a node without IPv4 stack installed -> fail "
                 "(maybe need to use InternetStackHelper?)");

  int32_t interface = ipv4->GetInterfaceForDevice (netDevice);
  if (interface == -1)
    {
      interface = ipv4->AddInterface (netDevice);
    }
  NS_ASSERT_MSG (interface >= 0, "DhcpHelperUav: Interface index not found");

  Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (addr, mask);
  ipv4->AddAddress (interface, ipv4Addr);
  ipv4->SetMetric (interface, 1);
  ipv4->SetUp (interface);
  retval.Add (ipv4, interface);

  // Install the default traffic control configuration if the traffic
  // control layer has been aggregated, if this is not
  // a loopback interface, and there is no queue disc installed already
  Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
  if (tc && DynamicCast<LoopbackNetDevice> (netDevice) == 0 && tc->GetRootQueueDiscOnDevice (netDevice) == 0)
    {
      NS_LOG_LOGIC ("DhcpHelperUav - Installing default traffic control configuration");
      TrafficControlHelper tcHelper = TrafficControlHelper::Default ();
      tcHelper.Install (netDevice);
    }

  // check that the already fixed addresses are not in conflict with the pool
  std::list<std::pair<Ipv4Address, Ipv4Address> >::iterator iter;
  for (iter=m_addressPools.begin (); iter!=m_addressPools.end (); iter ++)
    {
      if (addr.Get () >= iter->first.Get () && addr.Get () <= iter->second.Get ())
        {
          NS_ABORT_MSG ("DhcpHelperUav: Fixed address can not conflict with a pool: " << addr << " is in [" << iter->first << ",  " << iter->second << "]");
        }
    }
  m_fixedAddresses.push_back (addr);
  return retval;
}

} // namespace ns3
