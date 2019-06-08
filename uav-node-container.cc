/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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

#include "uav-node-container.h"
#include "ns3/names.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavNodeContainer");

void UavNodeContainer::Clear() {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("UavNodeContainer::Clear");
  m_nodes.clear();
}

Ptr<Node> UavNodeContainer::RemoveAt (uint32_t pos) {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<pos);
  Ptr<Node> n = m_nodes[pos];
  m_nodes.erase(m_nodes.begin()+pos);
  return n;
}

Ptr<Node> UavNodeContainer::RemoveId (uint32_t id) {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<id);
  int c=0;
  for (Iterator i = m_nodes.begin(); i != m_nodes.end(); ++i, ++c) {
    if ((*i)->GetId() == id) {
      break;
    }
  }
  return RemoveAt(uint32_t(c));
}

uint32_t UavNodeContainer::CheckId (uint32_t id) {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<id);
  uint32_t c=0;
  Iterator i;
  for (i = m_nodes.begin(); i != m_nodes.end(); ++i, ++c) {
    if ((*i)->GetId() == id) {
      break;
    }
  }
  return c;
}

}
