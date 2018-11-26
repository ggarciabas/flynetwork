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
#include "location-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LocationModel");

NS_OBJECT_ENSURE_REGISTERED(LocationModel);

TypeId
LocationModel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::LocationModel")
                          .SetParent<Object>()
                          .SetGroupName("Flynetwork-Model")
                          .AddConstructor<LocationModel>()
                          .AddAttribute("Id",
                                        "Uav model id",
                                        UintegerValue(-1),
                                        MakeUintegerAccessor(&LocationModel::m_id),
                                        MakeUintegerChecker<uint32_t>());
  return tid;
}

LocationModel::LocationModel()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("LocationModel::LocationModel @" << Simulator::Now().GetSeconds());
  m_used = false;
  m_totaCli = 0;
}

LocationModel::~LocationModel()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("LocationModel::~LocationModel @" << Simulator::Now().GetSeconds());
  m_position.clear();
}

void LocationModel::SetId(uint32_t id)
{
  m_id = id;
}

uint32_t
LocationModel::GetId()
{
  return m_id;
}

void LocationModel::SetPosition(double x, double y)
{
  m_position.clear();
  m_position.push_back(x);
  m_position.push_back(y);
}

const std::vector<double>
LocationModel::GetPosition()
{
  return m_position;
}

bool LocationModel::IsUsed() {
  return m_used;
}

void LocationModel::SetUsed () {
  m_used = true;
}

double LocationModel::GetTotalConsumption ()
{
  return m_totalConsumption;
}

void LocationModel::SetTotalConsumption (double v)
{
  m_totalConsumption = v;
}

void LocationModel::DoDispose () {
  NS_LOG_DEBUG ("LocationModel::Dispose id "<< m_id << " REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
}

void LocationModel::SetTotalCli (int t) {
  m_totaCli = t;
}

int LocationModel::GetTotalCli () {
  return m_totaCli;
}

} // namespace ns3
