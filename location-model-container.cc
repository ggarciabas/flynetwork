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

#include "location-model-container.h"
#include "location-model.h"
#include "ns3/names.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LocationModelContainer");

LocationModelContainer::LocationModelContainer()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

LocationModelContainer::LocationModelContainer(Ptr<LocationModel> model)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << model);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

LocationModelContainer::LocationModelContainer(std::string modelName)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << modelName);
  Ptr<LocationModel> model = Names::Find<LocationModel>(modelName);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

LocationModelContainer::LocationModelContainer(const LocationModelContainer &a, const LocationModelContainer &b)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << &a << &b);
  *this = a;
  Add(b);
}

LocationModelContainer::Iterator
LocationModelContainer::Begin(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_models.begin();
}

LocationModelContainer::Iterator
LocationModelContainer::End(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_models.end();
}

uint32_t
LocationModelContainer::GetN(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_models.size();
}

const Ptr<LocationModel>
LocationModelContainer::Get(uint32_t i) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << i << m_models.size());
  return m_models[i];
}

void LocationModelContainer::Add(LocationModelContainer container)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << &container);
  for (Iterator i = container.Begin(); i != container.End(); i++)
  {
    m_models.push_back(*i);
  }
}

void LocationModelContainer::Add(Ptr<LocationModel> model)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << model);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

void LocationModelContainer::Add(std::string modelName)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << modelName);
  Ptr<LocationModel> model = Names::Find<LocationModel>(modelName);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

void LocationModelContainer::Clear(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("LocationModelContainer::Clear");
  for (Iterator i = m_models.begin(); i != m_models.end(); i++)
  {
    NS_LOG_DEBUG ("LocationModelContainer::Clear id " << (*i)->GetId() << " REF " << (*i)->GetReferenceCount());
    (*i)->Dispose();
  }
  m_models.clear();
}

void LocationModelContainer::Remove (uint32_t id) {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  for (Iterator i = m_models.begin(); i != m_models.end(); i++)
  {
    if ((*i)->GetId() == id) {
      (*i)->Dispose();
      break;
    }
  }
}

} // namespace ns3
