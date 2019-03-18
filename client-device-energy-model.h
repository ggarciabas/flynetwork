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

#ifndef CLIENT_DEVICE_ENERGY_MODEL_H
#define CLIENT_DEVICE_ENERGY_MODEL_H

#include "ns3/nstime.h"
#include "ns3/traced-value.h"
#include "ns3/core-module.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mobility-module.h"
#include "ns3/energy-module.h"
#include "uav-energy-source.h"

namespace ns3
{

/**
 *
 * A simple device energy  that helps to calculate the energy used of UAv based on clients
 *
 */
class ClientDeviceEnergyModel : public DeviceEnergyModel
{
public:
  typedef Callback<void> EnergyCallback;

  static TypeId GetTypeId(void);
  ClientDeviceEnergyModel();
  virtual ~ClientDeviceEnergyModel();

  /**
   * \param interval Energy update interval.
   *
   * This function sets the interval between each energy update.
   */
  void SetEnergyUpdateInterval(Time interval);

  /**
   * \returns The interval between each energy update.
   */
  Time GetEnergyUpdateInterval(void) const;

  /**
   * \returns Total energy consumption of the vehicle.
   *
   * Implements DeviceEnergyModel::GetTotalEnergyConsumption.
   */
  virtual double GetTotalEnergyConsumption (void) const;

  /**
   * \brief Sets pointer to EnergySouce installed on node.
   *
   * \param source Pointer to EnergySource installed on node.
   *
   */
  virtual void SetEnergySource(Ptr<EnergySource> source);

  /**
   * \param callback Callback function.
   *
   * Sets callback for energy depletion handling.
   */
  void SetEnergyDepletionCallback(EnergyCallback callback);
  void SetEnergyRechargedCallback(EnergyCallback callback);

  /**
   * \brief Handles energy depletion.
   *
   * Not implemented
   */
  virtual void HandleEnergyDepletion(void);

  virtual void HandleEnergyRecharged (void);

  /**
   * \brief Handles energy changed.
   *
   * Not implemented
   */
  virtual void HandleEnergyChanged (void);

  virtual void ChangeState (int) {}

  /**
   * \brief Sets pointer to node.
   *
   * \param node Pointer to node.
   *
   */
  virtual void SetNode(Ptr<Node> node);

  /**
   * \brief Gets pointer to node.
   *
   * \returns Pointer to node.
   *
   */
  virtual Ptr<Node> GetNode(void) const;

  void AddClient ();
  void RemoveClient();

private:

  void DoDispose (void);

  void DoInitialize();

  double UpdateConsumption ();

  /**
   * \returns Current draw of device, at current state.
   *
   * Implements DeviceEnergyModel::GetCurrentA.
   */
  virtual double DoGetCurrentA (void) const;

  void ClientConsumption ();

private:
  Time m_energyUpdateInterval;
  Time m_lastUpdateTime;
  TracedValue<double> m_totalEnergyConsumption;
  std::ofstream m_file;
  std::string m_pathData;
  double m_clientCost; // in Joules
  EventId m_cliEvent;
  Ptr<EnergySource> m_source;
  Ptr<Node> m_node;

  int m_clientCount;
  /**
   * Callback type for Energy Depletion function. Devices uses this callbak to notify
   * the node about the energy depletion.
   */
  EnergyCallback m_energyDepletionCallback;
  EnergyCallback m_energyRechargedCallback;
};

} // namespace ns3

#endif /* CLIENT_DEVICE_ENERGY_MODEL_H */
