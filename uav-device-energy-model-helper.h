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

#ifndef UAV_DEVICE_ENERGY_MODEL_HELPER_H
#define UAV_DEVICE_ENERGY_MODEL_HELPER_H

#include "ns3/node-container.h"
#include "uav-device-energy-model.h"
#include "ns3/core-module.h"

namespace ns3
{

/**
 * \ingroup energy
 * \brief Creates DeviceEnergyModel objects.
 *
 * This class helps to create and install DeviceEnergyModel onto NetDevice. A
 * DeviceEnergyModel is connected to a NetDevice (or PHY object) by callbacks.
 * Note that DeviceEnergyModel objects are *not* aggregated onto the node. They
 * can be accessed through the EnergySource object, which *is* aggregated onto
 * the node.
 *
 */
class UavDeviceEnergyModelHelper : public Object
{
public:
  static TypeId GetTypeId(void);
  UavDeviceEnergyModelHelper();
  virtual ~UavDeviceEnergyModelHelper();

  /**
   * \param node Pointer to the Node.
   * \param source The EnergySource the DeviceEnergyModel will be using.
   * \returns An DeviceEnergyModelContainer contains all the DeviceEnergyModels.
   *
   * Installs an DeviceEnergyModel with a specified EnergySource onto a
   * xNetDevice.
   */
  DeviceEnergyModelContainer Install(Ptr<Node> node,
                                        Ptr<EnergySource> source) const;

  /**
   * \param deviceContainer List of NetDevices to be install DeviceEnergyModel
   * objects.
   * \param sourceContainer List of EnergySource the DeviceEnergyModel will be
   * using.
   * \returns An DeviceEnergyModelContainer contains all the DeviceEnergyModels.
   *
   * Installs DeviceEnergyModels with specified EnergySources onto a list of
   * NetDevices.
   */
  DeviceEnergyModelContainer Install(NodeContainer nodeContainer,
                                        EnergySourceContainer sourceContainer) const;

  void Set(std::string name, const AttributeValue &v);

  void SetEnergyDepletionCallback (UavDeviceEnergyModel::EnergyCallback);
  void SetEnergyRechargedCallback (UavDeviceEnergyModel::EnergyCallback);

private:
  virtual Ptr<UavDeviceEnergyModel> DoInstall(Ptr<Node> node, Ptr<EnergySource> source) const;

  ObjectFactory m_energyModel;
  UavDeviceEnergyModel::EnergyCallback m_depletionCallback;
  UavDeviceEnergyModel::EnergyCallback m_rechargedCallback;
};

} // namespace ns3

#endif /* UAV_DEVICE_ENERGY_MODEL_HELPER_H */
