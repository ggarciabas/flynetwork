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

#ifndef UAV_APP_CONTAINER_H
#define UAV_APP_CONTAINER_H

#include "uav-application.h"
#include "ns3/names.h"
#include "ns3/log.h"

namespace ns3
{

/**
 * \ingroup FlyNetwork
 */
class UavApplicationContainer
{
public:
  typedef std::vector<Ptr<UavApplication>>::const_iterator Iterator;

public:
  /**
   * Creates an empty UavApplicationContainer.
   */
  UavApplicationContainer();

  UavApplicationContainer(Ptr<UavApplication> app);

  UavApplicationContainer(std::string appName);

  /**
   * \param a A UavApplicationContainer.
   * \param b Another UavApplicationContainer.
   *
   * Creates a UavApplicationContainer by concatenating UavApplicationContainer b
   * to UavApplicationContainer a.
   *
   * \note Can be used to concatenate 2 Ptr<UavApplication> directly. C++
   * will be calling UavApplicationContainer constructor with Ptr<UavApplication>
   * first.
   */
  UavApplicationContainer(const UavApplicationContainer &a,
                          const UavApplicationContainer &b);

  Iterator Begin(void) const;

  Iterator End(void) const;

  /**
   * \brief Get the number of Ptr<UavApplication> stored in this container.
   *
   * \returns The number of Ptr<UavApplication> stored in this container.
   */
  uint32_t GetN(void) const;

  /**
   * \brief Get the i-th Ptr<UavApplication> stored in this container.
   *
   * \param i Index of the requested Ptr<UavApplication>.
   * \returns The requested Ptr<UavApplication>.
   */
  Ptr<UavApplication> Get(uint32_t i) const;

  /**
   * \param container Another UavApplicationContainer.
   *
   * Appends the contents of another UavApplicationContainer to the end of
   * this UavApplicationContainer.
   */
  void Add(UavApplicationContainer container);

  /**
   * \brief Append a single Ptr<UavApplication> to the end of this container.
   *
   */
  void Add(Ptr<UavApplication> app);

  /**
   * \brief Append a single Ptr<UavApplication> referred to by its object
   * name to the end of this container.
   *
   */
  void Add(std::string appName);

  /**
   * \brief Removes all elements in the container.
   */
  void Clear(void);

private:
  std::vector<Ptr<UavApplication>> m_apps;
};

} // namespace ns3

#endif /* UAV_APP_CONTAINER_H */
