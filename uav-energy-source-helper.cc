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

#include "uav-energy-source-helper.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("UavEnergySourceHelper");

UavEnergySourceHelper::UavEnergySourceHelper()
{
  NS_LOG_FUNCTION(this);
  m_uavEnergySource.SetTypeId("ns3::UavEnergySource");
}

UavEnergySourceHelper::~UavEnergySourceHelper()
{
  NS_LOG_FUNCTION(this);
}

void UavEnergySourceHelper::Set(std::string name, const AttributeValue &v)
{
  NS_LOG_FUNCTION(this << name << &v);
  m_uavEnergySource.Set(name, v);
}

Ptr<EnergySource>
UavEnergySourceHelper::DoInstall(Ptr<Node> node) const
{
  NS_LOG_FUNCTION(this << node);
  NS_ASSERT(node != NULL);
  Ptr<EnergySource> source = m_uavEnergySource.Create()->GetObject<EnergySource>();
  NS_ASSERT(source != NULL);
  source->SetNode(node);
  return source;
}

} // namespace ns3
