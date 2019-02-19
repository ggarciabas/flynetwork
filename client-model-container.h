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

#ifndef CLIENT_MODEL_CONTAINER_H
#define CLIENT_MODEL_CONTAINER_H

#include "client-model.h"
#include <vector>
#include <stdint.h>

namespace ns3
{

/**
 * \ingroup FlyNetwork
 */
class ClientModelContainer
{
public:
  typedef std::vector<Ptr<ClientModel>>::const_iterator Iterator;

public:
  /**
   * Creates an empty ClientModelContainer.
   */
  ClientModelContainer();

  ClientModelContainer(Ptr<ClientModel> model);

  ClientModelContainer(std::string modelName);

  /**
   * \param a A ClientModelContainer.
   * \param b Another ClientModelContainer.
   *
   * Creates a ClientModelContainer by concatenating ClientModelContainer b
   * to ClientModelContainer a.
   *
   * \note Can be used to concatenate 2 Ptr<ClientModel> directly. C++
   * will be calling ClientModelContainer constructor with Ptr<ClientModel>
   * first.
   */
  ClientModelContainer(const ClientModelContainer &a,
                       const ClientModelContainer &b);

  Iterator Begin(void) const;

  Iterator End(void) const;

  Ptr<ClientModel> FindClientModel(std::string login);
  Ptr<ClientModel> FindClientModel(Ipv4Address ip);

  /**
   * \brief Get the number of Ptr<ClientModel> stored in this container.
   *
   * \returns The number of Ptr<ClientModel> stored in this container.
   */
  uint32_t GetN(void) const;

  /**
   * \brief Get the i-th Ptr<ClientModel> stored in this container.
   *
   * \param i Index of the requested Ptr<ClientModel>.
   * \returns The requested Ptr<ClientModel>.
   */
  Ptr<ClientModel> Get(uint32_t i) const;

  /**
   * \param container Another ClientModelContainer.
   *
   * Appends the contents of another ClientModelContainer to the end of
   * this ClientModelContainer.
   */
  void Add(ClientModelContainer container);

  /**
   * \brief Append a single Ptr<ClientModel> to the end of this container.
   *
   */
  void Add(Ptr<ClientModel> model);

  /**
   * \brief Append a single Ptr<ClientModel> referred to by its object
   * name to the end of this container.
   *
   */
  void Add(std::string modelName);

  /**
   * \brief Removes all elements in the container.
   */
  void Clear(void);

  bool IsEmpty();

  void RemoveLogin (std::string id);
  void RemoveAt (uint32_t pos);

private:
  std::vector<Ptr<ClientModel>> m_models;
};

} // namespace ns3

#endif /* CLIENT_MODEL_CONTAINER_H */
