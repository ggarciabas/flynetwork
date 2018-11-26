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

#ifndef LOCATION_MODEL
#define LOCATION_MODEL

#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include <vector>
#include <cmath>

using namespace std;

namespace ns3
{

/**
 * LocationModel
 */
class LocationModel : public Object
{
public:
  static TypeId GetTypeId(void);
  LocationModel();
  virtual ~LocationModel();

  void SetId(uint32_t);
  uint32_t GetId();

  void SetPosition(double, double);
  const std::vector<double> GetPosition();

  std::string toString() {
    std::ostringstream os;
    os << "Id: " << m_id << "\n\tPos: (" << m_position.at(0) << "," << m_position.at(1) << ")\n\tConsumption: " << m_totalConsumption << std::endl;
    return os.str();
  }

  bool IsUsed();
  void SetUsed();

  double GetTotalConsumption ();
  void SetTotalConsumption (double);

  void SetTotalCli (int t);
  int GetTotalCli ();

private:
  void DoDispose ();
  bool m_used;
  int m_totaCli;
  uint32_t m_id;
  std::vector<double> m_position;
  double m_totalConsumption;
};

} // namespace ns3

#endif /* LOCATION_MODEL */
