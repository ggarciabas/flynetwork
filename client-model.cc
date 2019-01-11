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
                                        MakeStringChecker())
                          .AddAttribute("Consumption",
                                        "Consumption in joules per second.",
                                        DoubleValue(0.34), // J/s
                                        MakeDoubleAccessor(&ClientModel::m_consumption),
                                        MakeDoubleChecker<double>());
  return tid;
}

ClientModel::ClientModel() : m_position()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("ClientModel::ClientModel @" << Simulator::Now().GetSeconds());
  m_consumption = 0.0;
  m_locConnected = 0;
  m_pci = 1.0;
}

ClientModel::~ClientModel()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("ClientModel::~ClientModel @" << Simulator::Now().GetSeconds());
  m_position.clear();
  m_locConnected = 0;
}

void ClientModel::SetPosition(double x, double y)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() <<x<<y);
  m_position.clear();
  m_position.push_back(x);
  m_position.push_back(y);
}

const std::vector<double>
ClientModel::GetPosition()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  return m_position;
}

const std::vector<double>
ClientModel::GetPosition(double r_max)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  std::vector<double> p;
  p.push_back(m_position.at(0)/r_max);
  p.push_back(m_position.at(1)/r_max);
  return p;
}

void ClientModel::SetLogin(std::string str)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() <<str);
  m_login = str;
}

std::string
ClientModel::GetLogin()
{
  // NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  return m_login;
}

double
ClientModel::GetConsumption ()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  return m_consumption;
}

void ClientModel::SetConsumption (double c)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() <<c);
  m_consumption = c;
}

void ClientModel::SetIp (Ipv4Address ip) {
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() <<ip);
  m_addr = ip;
}

Ipv4Address ClientModel::GetIp () {
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  return m_addr;
}

void ClientModel::SetUpdatePos (Time t)
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() <<t);
  m_updatePos = t;
}

Time ClientModel::GetUpdatePos ()
{
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
  return m_updatePos;
}

void ClientModel::DoDispose () {
  NS_LOG_FUNCTION(this->m_login << Simulator::Now().GetSeconds() );
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

double ClientModel::GetXPosition (double r_max) {
  return m_position.at(0)/r_max;
}

double ClientModel::GetYPosition (double r_max) {
  return m_position.at(1)/r_max;
}

double ClientModel::GetXPosition () {
  return m_position.at(0);
}

double ClientModel::GetYPosition () {
  return m_position.at(1);
}


void ClientModel::SetLocConnected (Ptr<LocationModel> l) {
  m_locConnected = l;
}

Ptr<LocationModel> ClientModel::GetLocConnected () {
  return m_locConnected;
}

void ClientModel::SetConnected (bool c) {
    m_connected = c;
}

bool ClientModel::IsConnected () {
  return m_connected;
}

double ClientModel::GetPci () {
  return m_pci;
}

void ClientModel::SetPci (double p) {
  m_pci = p;
}

} // namespace ns3
