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

#ifndef LOCATION_MODEL_CONTAINER_H
#define LOCATION_MODEL_CONTAINER_H

// #include "location-model.h"
#include "ns3/core-module.h"
#include <vector>
#include <stdint.h>

namespace ns3
{

  class LocationModel;

/**
 * \ingroup FlyNetwork
 */
class LocationModelContainer
{
public:
  typedef std::vector<Ptr<LocationModel>>::const_iterator Iterator;

public:
  /**
   * Creates an empty LocationModelContainer.
   */
  LocationModelContainer();

  LocationModelContainer(Ptr<LocationModel> model);

  LocationModelContainer(std::string modelName);

  /**
   * \param a A LocationModelContainer.
   * \param b Another LocationModelContainer.
   *
   * Creates a LocationModelContainer by concatenating LocationModelContainer b
   * to LocationModelContainer a.
   *
   * \note Can be used to concatenate 2 Ptr<LocationModel> directly. C++
   * will be calling LocationModelContainer constructor with Ptr<LocationModel>
   * first.
   */
  LocationModelContainer(const LocationModelContainer &a,
                         const LocationModelContainer &b);

  Iterator Begin(void) const;

  Iterator End(void) const;

  /**
   * \brief Get the number of Ptr<LocationModel> stored in this container.
   *
   * \returns The number of Ptr<LocationModel> stored in this container.
   */
  uint32_t GetN(void) const;

  /**
   * \brief Get the i-th Ptr<LocationModel> stored in this container.
   *
   * \param i Index of the requested Ptr<LocationModel>.
   * \returns The requested Ptr<LocationModel>.
   */
  const Ptr<LocationModel> Get(uint32_t i) const;

  /**
   * \param container Another LocationModelContainer.
   *
   * Appends the contents of another LocationModelContainer to the end of
   * this LocationModelContainer.
   */
  void Add(LocationModelContainer container);

  /**
   * \brief Append a single Ptr<LocationModel> to the end of this container.
   *
   */
  void Add(Ptr<LocationModel> model);

  /**
   * \brief Append a single Ptr<LocationModel> referred to by its object
   * name to the end of this container.
   *
   */
  void Add(std::string modelName);

  /**
   * \brief Removes all elements in the container.
   */
  void Clear(void);

  void Remove (uint32_t id);

private:
  std::vector<Ptr<LocationModel>> m_models;
};

} // namespace ns3

#endif /* LOCATION_MODEL_CONTAINER_H */
