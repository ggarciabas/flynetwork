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

#include "client-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ClientModel");

NS_OBJECT_ENSURE_REGISTERED(ClientModel);

TypeId
ClientModel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ClientModel")
                          .SetParent<Object>()
                          .SetGroupName("Flynetwork-Model")
                          .AddConstructor<ClientModel>()
                          .AddAttribute("Login",
                                        "Login of the client node.",
                                        StringValue("none"),
                                        MakeStringAccessor(&ClientModel::m_login),
                                        MakeStringChecker());
  return tid;
}

ClientModel::ClientModel() : m_position()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("ClientModel::ClientModel @" << Simulator::Now().GetSeconds());
  m_consumption = 0.0;
}

ClientModel::~ClientModel()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("ClientModel::~ClientModel @" << Simulator::Now().GetSeconds());
  m_position.clear();
}

void ClientModel::SetPosition(double x, double y)
{
  m_position.clear();
  m_position.push_back(x);
  m_position.push_back(y);
}

const std::vector<double>
ClientModel::GetPosition()
{
  return m_position;
}

void ClientModel::SetLogin(std::string str)
{
  m_login = str;
}

std::string
ClientModel::GetLogin()
{
  return m_login;
}

double
ClientModel::GetConsumption ()
{
  return m_consumption;
}

void ClientModel::SetConsumption (double c)
{
  m_consumption = c;
}

void ClientModel::SetIp (Ipv4Address ip) {
  m_addr = ip;
}

Ipv4Address ClientModel::GetIp () {
  return m_addr;
}

void ClientModel::SetUpdatePos (Time t)
{
  m_updatePos = t;
}

Time ClientModel::GetUpdatePos ()
{
  return m_updatePos;
}

void ClientModel::DoDispose () {
  NS_LOG_DEBUG ("ClientModel::Dispose login " << m_login << " REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
}

// void ClientModel::SetUpdateCons (Time t)
// {
//   m_updateCons = t;
// }
// Time ClientModel::GetUpdateCons ()
// {
//   return m_updateCons;
// }

} // namespace ns3
