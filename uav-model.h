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

#ifndef UAV_MODEL
#define UAV_MODEL

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <vector>

namespace ns3
{

/**
 * UavModel
 *
 */
class UavModel : public Object
{
public:
  static TypeId GetTypeId(void);
  UavModel();
  virtual ~UavModel();

  void SetId(uint32_t);
  uint32_t GetId();

  void SetPosition(double, double);
  const std::vector<double> GetPosition();

  void SetNewPosition(double, double);
  const std::vector<double> GetNewPosition();

  Ipv4Address GetAddressAdhoc();
  void SetDepletion (bool);
  bool IsDepletion ();

  void SetTotalEnergy(double);
  double GetTotalEnergy();
  double CalculateEnergyCost(double dist);
  double GetHoverCost ();
  double GetTotalBattery();
  void SetNewPosition(std::vector<double> pos);

  void SetSocket(Ptr<Socket>);
  Ptr<Socket> GetSocket();

  bool IsConfirmed ();
  void NotConfirmed ();
  void ConfirmPosition ();
  void GotoReceived (bool);
  bool IsGotoReceived ();

  bool ClientDataConfirmed ();
  void SetClientDataConfirmed (bool);

  void SetSendPositionEvent (EventId);
  void CancelSendPositionEvent();

  void SetAskCliDataEvent (EventId);
  void CancelAskCliDataEvent();

  std::string toString() {
    std::ostringstream os;
    os << "Id: " << m_id << "\n\tPos: (" << m_position.at(0) << "," << m_position.at(1) << ")";
    os << "\n\tTotalEnergy: " << m_totalEnergy << "\n\tTotalBattery: " << m_totalEnergy;
    os << "\n\tEnergyCost: " << m_energyCost << std::endl;
    return os.str();
  }

private:
  void DoDispose ();

  uint32_t m_id;
  bool m_depletion;
  bool m_gotoReceived;
  std::vector<double> m_position;
  std::vector<double> m_newPos;
  EventId m_sendPosition;
  EventId m_askCliData;
  Ipv4Address m_addressAdhoc;
  double m_totalEnergy; // total de bateria atual do UAV in Joules
  double m_energyCost;
  double m_hoverCost;
  double m_totalBattery; // total de bateria quando cheia in Joules
  Ptr<Socket> m_socket;
  bool m_confirmed;
  bool m_clientData;
};

} // namespace ns3

#endif /* UAV_MODEL */
