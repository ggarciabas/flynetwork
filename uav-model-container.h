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

#ifndef UAV_MODEL_CONTAINER_H
#define UAV_MODEL_CONTAINER_H

#include "uav-model.h"
#include <vector>
#include <stdint.h>

namespace ns3
{

/**
 * \ingroup FlyNetwork
 */
class UavModelContainer
{
public:
  typedef std::vector<Ptr<UavModel>>::const_iterator Iterator;

public:
  /**
   * Creates an empty UavModelContainer.
   */
  UavModelContainer();

  UavModelContainer(Ptr<UavModel> model);

  UavModelContainer(std::string modelName);

  /**
   * \param a A UavModelContainer.
   * \param b Another UavModelContainer.
   *
   * Creates a UavModelContainer by concatenating UavModelContainer b
   * to UavModelContainer a.
   *
   * \note Can be used to concatenate 2 Ptr<UavModel> directly. C++
   * will be calling UavModelContainer constructor with Ptr<UavModel>
   * first.
   */
  UavModelContainer(const UavModelContainer &a,
                    const UavModelContainer &b);

  Iterator Begin(void) const;

  Iterator End(void) const;

  Ptr<UavModel> FindUavModel(uint32_t id);

  Ptr<UavModel> RemoveAt (uint32_t pos);

  Ptr<UavModel> GetLast();

  /**
   * \brief Get the number of Ptr<UavModel> stored in this container.
   *
   * \returns The number of Ptr<UavModel> stored in this container.
   */
  uint32_t GetN(void) const;

  /**
   * \brief Get the i-th Ptr<UavModel> stored in this container.
   *
   * \param i Index of the requested Ptr<UavModel>.
   * \returns The requested Ptr<UavModel>.
   */
  Ptr<UavModel> Get(uint32_t i) const;

  /**
   * \param container Another UavModelContainer.
   *
   * Appends the contents of another UavModelContainer to the end of
   * this UavModelContainer.
   */
  void Add(UavModelContainer container);

  /**
   * \brief Append a single Ptr<UavModel> to the end of this container.
   *
   */
  void Add(Ptr<UavModel> model);

  void Add(Ptr<UavModel> model, uint32_t pos);

  /**
   * \brief Append a single Ptr<UavModel> referred to by its object
   * name to the end of this container.
   *
   */
  void Add(std::string modelName);

  /**
   * \brief Removes all elements in the container.
   */
  void Clear(void);

  void RemoveUav (Ptr<UavModel> uav);

private:
  std::vector<Ptr<UavModel>> m_models;
};

} // namespace ns3

#endif /* UAV_MODEL_CONTAINER_H */
