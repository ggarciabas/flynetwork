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

#include "uav-application-container.h"
#include "ns3/names.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavApplicationContainer");

UavApplicationContainer::UavApplicationContainer()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
}

UavApplicationContainer::UavApplicationContainer(Ptr<UavApplication> app)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << app);
  NS_ASSERT(app != NULL);
  m_apps.push_back(app);
}

UavApplicationContainer::UavApplicationContainer(std::string appName)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << appName);
  Ptr<UavApplication> app = Names::Find<UavApplication>(appName);
  NS_ASSERT(app != NULL);
  m_apps.push_back(app);
}

UavApplicationContainer::UavApplicationContainer(const UavApplicationContainer &a, const UavApplicationContainer &b)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << &a << &b);
  *this = a;
  Add(b);
}

UavApplicationContainer::Iterator
UavApplicationContainer::Begin(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_apps.begin();
}

UavApplicationContainer::Iterator
UavApplicationContainer::End(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_apps.end();
}

uint32_t
UavApplicationContainer::GetN(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_apps.size();
}

Ptr<UavApplication>
UavApplicationContainer::Get(uint32_t i) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << i);
  return m_apps[i];
}

void UavApplicationContainer::Add(UavApplicationContainer container)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << &container);
  for (Iterator i = container.Begin(); i != container.End(); i++)
  {
    m_apps.push_back(*i);
  }
}

void UavApplicationContainer::Add(Ptr<UavApplication> app)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << app);
  NS_ASSERT(app != NULL);
  m_apps.push_back(app);
}

void UavApplicationContainer::Add(std::string appName)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << appName);
  Ptr<UavApplication> app = Names::Find<UavApplication>(appName);
  NS_ASSERT(app != NULL);
  m_apps.push_back(app);
}

void UavApplicationContainer::Clear(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  for (Iterator i = m_apps.begin(); i != m_apps.end(); i++)
  {
    NS_LOG_DEBUG ("UavApplicationContainer::Clear id " << (*i)->GetId() << " REF " << (*i)->GetReferenceCount());
    (*i)->Dispose();
  }
  m_apps.clear();
}

} // namespace ns3
