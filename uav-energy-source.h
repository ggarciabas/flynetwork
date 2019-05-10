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

#ifndef UAV_ENERGY_SOURCE_H
#define UAV_ENERGY_SOURCE_H

#include "ns3/traced-value.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/node.h"
#include "ns3/core-module.h"
#include "ns3/energy-module.h"
#include <cmath>

#include "client-device-energy-model.h"
#include "uav-device-energy-model.h"

namespace ns3
{

  class ClientDeviceEnergyModel;
  class UavDeviceEnergyModel;

/**
 * UavEnergySource decreases/increases remaining energy stored in itself in
 * linearly.
 *
 */
class UavEnergySource : public EnergySource
{
public:
  static TypeId GetTypeId(void);
  UavEnergySource();
  virtual ~UavEnergySource();

  /**
   * \returns Initial energy (capacity) of the energy source.
   *
   * Set method is to be defined in child class only if necessary. For sources
   * with a fixed initial energy (energy capacity), set method is not needed.
   */
  virtual double GetInitialEnergy(void) const;

  /**
   * \returns Supply voltage at the energy source.
   *
   * Implements GetSupplyVoltage.
   */
  virtual double GetSupplyVoltage (void) const;

  /**
   * \returns Remaining energy at the energy source.
   */
  virtual double GetRemainingEnergy(void);

  /**
   * \return Energy fraction = remaining energy / initial energy [0, 1]
   *
   * This function returns the percentage of energy left in the energy source.
   */
  virtual double GetEnergyFraction(void);

  /**
   * This function goes through the DeviceEnergyModels to obtain total
   * energy cost draw at the energy source and updates remaining energy. Called by
   * DeviceEnergyModel to inform EnergySource of a state change.
   */
  virtual void UpdateEnergySource (void);
  virtual void UpdateEnergySourceMove (double);
  virtual void UpdateEnergySourceClient (double);
  virtual void UpdateEnergySourceHover (double energyToDecrease);

  /**
   * \param initialEnergyJ Initial energy, in Joules
   *
   * Sets initial energy stored in the energy source. Note that initial energy
   * is assumed to be set before simulation starts and is set only once per
   * simulation.
   */
  virtual void SetInitialEnergy(double initialEnergyJ);

  /**
   * \param supplyVoltageV Supply voltage at the energy source, in Volts.
   *
   * Sets supply voltage of the energy source.
   */
  void SetSupplyVoltage (double supplyVoltageV);

  /**
   * \param interval Energy update interval.
   *
   * This function sets the interval between each energy update.
   */
  void SetEnergyUpdateInterval (Time interval);
  void SetUpdateThresholdInterval (Time interval);

  /**
   * \returns The interval between each energy update.
   */
  Time GetEnergyUpdateInterval (void) const;
  Time GetUpdateThresholdInterval (void) const;

  void SetBasicEnergyLowBatteryThresholdUav (double thr);

  void Stop (); // stop, UAV fora da rede
  void Start (); // para iniciar o posicionamento

  void SetDeviceEnergyModel (Ptr<DeviceEnergyModel>);
  void SetCliDeviceEnergyModel (Ptr<DeviceEnergyModel>);

  double GetWifiAcum () { return m_wifiAcum; }
  double GetMoveAcum () { return m_movAcum; }
  double GetHoverAcum () { return m_hoverAcum; }
  double GetCliAcum () { return m_cliAcum; }

  double GetRealRemainingEnergy(); // para saber a bateria correta quando (problema com wifiEnergyModule!)

  double GetWifiEnergy () {
    return m_wifiEnergy;
  }
  double GetClientEnergy () {
    return m_clientEnergy;
    }
  double GetMoveEnergy () {
    return m_moveEnergy;
    }
  double GetHoverEnergy () {
    return m_hoverEnergy;
    }

  void TimeEnergy(Time);

private:
  /// Defined in ns3::Object
  void DoInitialize (void);

  /// Defined in ns3::Object
  void DoDispose (void);
  /**
   * Handles the remaining energy going to zero event. This function notifies
   * all the energy models aggregated to the node about the energy being
   * depleted. Each energy model is then responsible for its own handler.
   */
  virtual void HandleEnergyDrainedEvent(void);

  /**
   * Handles the remaining energy exceeding the high threshold after it went
   * below the low threshold. This function notifies all the energy models
   * aggregated to the node about the energy being recharged. Each energy model
   * is then responsible for its own handler.
   */
  void HandleEnergyRechargedEvent (void) {}

  /**
   * Calculates remaining energy. This function uses the total current from all
   * device models to calculate the amount of energy to decrease. The energy to
   * decrease is given by:
   *    energy to decrease = total current * supply voltage * time duration
   * This function subtracts the calculated energy to decrease from remaining
   * energy.
   */
  void CalculateRemainingEnergy (void);

  void UpdateThreshold ();  

private:

  double m_wifiTE; // timing energy calculation
  double m_clientTE;
  double m_moveTE;
  double m_hoverTE;

  EventId m_timeEnergy;

  double m_wifiEnergy;
  double m_clientEnergy;
  double m_moveEnergy;
  double m_hoverEnergy;

  double m_initialEnergyJ;                // initial energy, in Joules
  double m_supplyVoltageV;                // supply voltage, in Volts
  double m_lowBatteryThUav;                 // low battery threshold, as a fraction of the initial energy
  double m_lowBatteryThCli;
  double m_highBatteryTh;                // high battery threshold, as a fraction of the initial energy
  bool m_depleted;                       // set to true when the remaining energy goes below the low threshold,
                                         // set to false again when the remaining energy exceeds the high threshold
  TracedValue<double> m_remainingEnergyJ; // remaining energy, in mAs
  EventId m_energyUpdateEvent;           // energy update event
  EventId m_updateThr;
  Vector m_lastPosition;                 // last position of the node
  Time m_lastUpdateTime;                  // last update time
  Time m_updateThrTime;
  Time m_energyUpdateInterval;           // energy update interval
  std::ofstream m_file;

  bool m_onoff;

  double m_hoverAcum;
  double m_movAcum;
  double m_cliAcum;
  double m_wifiAcum;

  std::string m_pathData;

  Ptr<DeviceEnergyModel> m_uavDev;
  Ptr<UavDeviceEnergyModel> m_uavDevModel;
  Ptr<DeviceEnergyModel> m_cliDev;
  Ptr<ClientDeviceEnergyModel> m_cliDevModel;

};

} // namespace ns3

#endif /* UAV_ENERGY_SOURCE_H */

