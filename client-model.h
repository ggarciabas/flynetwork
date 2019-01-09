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

#ifndef CLIENT_MODEL
#define CLIENT_MODEL

#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include <string>
#include <vector>

using namespace std;

namespace ns3
{

/**
 * ClientModel
 */
class ClientModel : public Object
{
public:
  static TypeId GetTypeId(void);
  ClientModel();
  virtual ~ClientModel();

  void SetLogin(std::string);
  std::string GetLogin();

  void SetPosition(double, double);
  const std::vector<double> GetPosition();

  double GetConsumption ();
  void SetConsumption (double c);

  void SetIp (Ipv4Address ip);
  Ipv4Address GetIp ();

  void SetUpdatePos (Time t);
  Time GetUpdatePos ();

  std::string ToString()
  {
    std::ostringstream os;
    os << "Login: " << m_login << "\n\tPos: (" << m_position.at(0) << "," << m_position.at(1) << ")" << "\n\tUpdatePos: " << m_updatePos.GetSeconds() << "\n\tConsumo: " << m_consumption << std::endl;
    return os.str();
  }

  // void SetUpdateCons (Time t);
  // Time GetUpdateCons ();

  double GetYPosition();
  double GetXPosition();

  void SetLocConnected (Ptr<LocationModel>);
  Ptr<LocationModel> GetLocConnected ();

  void SetConnected (bool);
  bool GetConnected ();

private:
  void DoDispose ();
  bool m_connected;
  Ptr<LocationModel> m_locConnected;
  std::string m_login;
  std::vector<double> m_position;
  double m_consumption; // fixed value
  Time m_updatePos;
  // Time m_updateCons;
  Ipv4Address m_addr;
};

} // namespace ns3

#endif /* CLIENT_MODEL */
