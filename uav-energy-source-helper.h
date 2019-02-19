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

#ifndef UAV_ENERGY_SOURCE_HELPER_H
#define UAV_ENERGY_SOURCE_HELPER_H

#include "ns3/energy-module.h"
#include "ns3/node.h"
#include "ns3/core-module.h"

namespace ns3
{

class UavEnergySourceHelper : public EnergySourceHelper
{
public:
  UavEnergySourceHelper();
  ~UavEnergySourceHelper();

  /**
   * \param name Name of attribute to set.
   * \param v Value of the attribute.
   *
   * Sets one of the attributes of underlying EnergySource.
   */
  void Set(std::string name, const AttributeValue &v);


private:
  virtual Ptr<EnergySource> DoInstall (Ptr<Node> node) const;

private:
  ObjectFactory m_uavEnergySource;
};

} // namespace ns3

#endif /* UAV_ENERGY_SOURCE_HELPER_H */
