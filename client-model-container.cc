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

#include "client-model-container.h"
#include "ns3/names.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ClientModelContainer");

ClientModelContainer::ClientModelContainer()
{
  NS_LOG_FUNCTION(this);
}

ClientModelContainer::ClientModelContainer(Ptr<ClientModel> model)
{
  NS_LOG_FUNCTION(this << model);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

ClientModelContainer::ClientModelContainer(std::string modelName)
{
  NS_LOG_FUNCTION(this << modelName);
  Ptr<ClientModel> model = Names::Find<ClientModel>(modelName);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

ClientModelContainer::ClientModelContainer(const ClientModelContainer &a, const ClientModelContainer &b)
{
  NS_LOG_FUNCTION(this << &a << &b);
  *this = a;
  Add(b);
}

ClientModelContainer::Iterator
ClientModelContainer::Begin(void) const
{
  NS_LOG_FUNCTION(this);
  return m_models.begin();
}

ClientModelContainer::Iterator
ClientModelContainer::End(void) const
{
  NS_LOG_FUNCTION(this);
  return m_models.end();
}

uint32_t
ClientModelContainer::GetN(void) const
{
  NS_LOG_FUNCTION(this);
  return m_models.size();
}

Ptr<ClientModel>
ClientModelContainer::Get(uint32_t i) const
{
  NS_LOG_FUNCTION(this << i);
  return m_models[i];
}

void ClientModelContainer::Add(ClientModelContainer container)
{
  NS_LOG_FUNCTION(this << &container);
  for (Iterator i = container.Begin(); i != container.End(); i++)
  {
    m_models.push_back(*i);
  }
}

void ClientModelContainer::Add(Ptr<ClientModel> model)
{
  NS_LOG_FUNCTION(this << model);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

void ClientModelContainer::Add(std::string modelName)
{
  NS_LOG_FUNCTION(this << modelName);
  Ptr<ClientModel> model = Names::Find<ClientModel>(modelName);
  NS_ASSERT(model != NULL);
  m_models.push_back(model);
}

Ptr<ClientModel>
ClientModelContainer::FindClientModel(std::string login)
{
  NS_LOG_FUNCTION(this);
  for (Iterator i = m_models.begin(); i != m_models.end(); i++)
  {
    if ((*i)->GetLogin().compare(login) == 0)
    {
      NS_LOG_INFO ("ClientModelContainer::FindClientModel @" <<Simulator::Now().GetSeconds() << " encontrado cliente com ID " << login);
      return (*i);
    }
  }
  NS_LOG_INFO ("ClientModelContainer::FindClientModel @" <<Simulator::Now().GetSeconds() << " NAO id " << login);
  return NULL;
}

Ptr<ClientModel>
ClientModelContainer::FindClientModel(Ipv4Address ip)
{
  NS_LOG_FUNCTION(this);
  for (Iterator i = m_models.begin(); i != m_models.end(); i++)
  {
    if ((*i)->GetIp() == ip)
    {
      NS_LOG_INFO ("ClientModelContainer::FindClientModel @" <<Simulator::Now().GetSeconds() << " encontrado cliente com IP " << ip);
      return (*i);
    }
  }
  NS_LOG_INFO ("ClientModelContainer::FindClientModel @" <<Simulator::Now().GetSeconds() << " NAO ip " << ip);
  return NULL;
}

void ClientModelContainer::Clear()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("ClientModelContainer::Clear");
  for (Iterator i = m_models.begin(); i != m_models.end(); i++)
  {
    NS_LOG_DEBUG ("ClientModelContainer::Clear login " << (*i)->GetLogin() << " REF " << (*i)->GetReferenceCount());
    (*i)->Dispose();
  }
  m_models.clear();
}

bool ClientModelContainer::IsEmpty()
{
  return m_models.size() == 0;
}

void ClientModelContainer::RemoveAt (uint32_t pos) {
  m_models.erase(m_models.begin()+pos);
}

void ClientModelContainer::RemoveLogin (std::string id) {
  int c=0;
  Iterator i;
  for (i = m_models.begin(); i != m_models.end(); ++i, ++c) {
    if (std::strcmp((*i)->GetLogin().c_str(), id.c_str()) == 0) {
      break;
    }
  }
  if (i != m_models.end()) RemoveAt(uint32_t(c)); // nao remover se nao existe!
}

} // namespace ns3
