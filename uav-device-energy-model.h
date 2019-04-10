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

#ifndef UAV_DEVICE_ENERGY_MODEL_H
#define UAV_DEVICE_ENERGY_MODEL_H

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
 * A simple device energy model where energyCost drain can be set by the user.
 *
 * It is supposed to be used as a testing model for energy sources.
 *
 */
class UavDeviceEnergyModel : public DeviceEnergyModel
{
public:
  typedef Callback<void> EnergyCallback;

  static TypeId GetTypeId(void);
  UavDeviceEnergyModel();
  virtual ~UavDeviceEnergyModel();

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
   * \brief Sets pointer to node.
   *
   * \param node Pointer to node.
   *
   */
  virtual void SetNode(Ptr<Node> node);

  virtual double GetEnergyCost ();

  double GetHoverCost ();
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

  const Ptr<EnergySource> GetEnergySource();

  /**
   * \param newState New state the device is in.
   *
   * Not implemented
   */
  virtual void ChangeState(int newState)
  {
  }

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

  virtual void HandleEnergyOff (void);

  virtual void HandleEnergyOn (void);

  double CalculateThreshold ();

  /**
   * \brief Handles energy changed.
   *
   * Not implemented
   */
  virtual void HandleEnergyChanged (void);
  void CourseChange (Ptr<const MobilityModel> mob);

  void HoverConsumption(void);
  void StartHover ();
  void StopHover ();

  void SetFlying(bool);
  bool IsFlying() {
    return m_flying;
  }

  double GetTimeToCentral () {
    return m_timeToCentral;
  }

private:

  void DoDispose (void);

  /**
   * \returns Current draw of device, at energyCost state.
   *
   * Implements DeviceEnergyModel::GetEnergyCost.
   */
  virtual double DoGetEnergyCost(void) const;

  /**
   * \returns Current draw of device, at current state.
   *
   * Implements DeviceEnergyModel::GetCurrentA.
   */
  virtual double DoGetCurrentA (void) const;

  void UpdateMoving(void);
  void HoveConsumption(void);

private:
  Time m_energyUpdateInterval;
  Vector m_lastPosition;
  double m_energyCost; // energy cost to a unit of distance J/m
  Ptr<EnergySource> m_source;
  EventId m_hoverEvent;
  Time m_lastTime;
  std::ofstream m_file;
  double m_avgVel;
  double m_resistTime;
  double m_hoverCost;
  double m_xCentral, m_yCentral;
  TracedValue<double> m_totalEnergyConsumption;
  std::string m_pathData;
  bool m_flying;
  double m_timeToCentral;
  /**
   * Callback type for Energy Depletion function. Devices uses this callbak to notify
   * the node about the energy depletion.
   */
  EnergyCallback m_energyDepletionCallback;
  EnergyCallback m_energyRechargedCallback;
};

} // namespace ns3

#endif /* UAV_DEVICE_ENERGY_MODEL_H */
