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

#include "uav-model-container.h"
#include "ns3/names.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavModelContainer");

UavModelContainer::UavModelContainer()
{
  NS_LOG_FUNCTION(this);
}

UavModelContainer::UavModelContainer(Ptr<UavModel> model)
{
  NS_LOG_FUNCTION(this << model);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

UavModelContainer::UavModelContainer(std::string modelName)
{
  NS_LOG_FUNCTION(this << modelName);
  Ptr<UavModel> model = Names::Find<UavModel>(modelName);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

UavModelContainer::UavModelContainer(const UavModelContainer &a, const UavModelContainer &b)
{
  NS_LOG_FUNCTION(this << &a << &b);
  *this = a;
  Add(b);
}

Ptr<UavModel>
UavModelContainer::GetLast()
{
  return m_models.back();
}

Ptr<UavModel>
UavModelContainer::FindUavModel(uint32_t id)
{
  NS_LOG_FUNCTION(this);
  for (Iterator i = m_models.begin(); i != m_models.end(); i++)
  {
    if ((*i)->GetId() == id)
    {
      return (*i);
    }
  }
  return NULL;
}

UavModelContainer::Iterator
UavModelContainer::Begin(void) const
{
  NS_LOG_FUNCTION(this);
  return m_models.begin();
}

UavModelContainer::Iterator
UavModelContainer::End(void) const
{
  NS_LOG_FUNCTION(this);
  return m_models.end();
}

uint32_t
UavModelContainer::GetN(void) const
{
  NS_LOG_FUNCTION(this);
  return m_models.size();
}

Ptr<UavModel>
UavModelContainer::Get(uint32_t i) const
{
  NS_LOG_FUNCTION(this << i);
  return m_models[i];
}

void UavModelContainer::Add(UavModelContainer container)
{
  NS_LOG_FUNCTION(this << &container);
  for (Iterator i = container.Begin(); i != container.End(); i++)
  {
    m_models.push_back(*i);
  }
}

void UavModelContainer::Add(Ptr<UavModel> model)
{
  NS_LOG_FUNCTION(this << model);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

void UavModelContainer::Add(std::string modelName)
{
  NS_LOG_FUNCTION(this << modelName);
  Ptr<UavModel> model = Names::Find<UavModel>(modelName);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

void UavModelContainer::Add(Ptr<UavModel> model, uint32_t pos)
{
  RemoveAt(pos);
  Add(model);
}

void UavModelContainer::Clear(void)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("UavModelContainer::Clear");
  for (Iterator i = m_models.begin(); i != m_models.end(); i++)
  {
    NS_LOG_DEBUG ("UavModelContainer::Clear id " << (*i)->GetId() << " REF " << (*i)->GetReferenceCount());
    (*i)->Dispose();
  }
  m_models.clear();
}

Ptr<UavModel> UavModelContainer::RemoveAt (uint32_t pos) {
  Ptr<UavModel> n = m_models[pos];
  m_models.erase(m_models.begin()+pos);
  return n;
}

void UavModelContainer::RemoveUav (Ptr<UavModel> uav) {
  int c = 0;
  for (Iterator i = m_models.begin(); i != m_models.end(); i++, c++)
  {
    if ((*i)->GetId() == uav->GetId()) {
      break;
    }
  }
  RemoveAt(c);
}

} // namespace ns3
