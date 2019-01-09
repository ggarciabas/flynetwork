/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Giovanna Garcia Basilio <giovanna@alunos.utfpr.edu.br>
 */

#ifndef UAV_MOBILITY_H
#define UAV_MOBILITY_H

#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/constant-velocity-helper.h"

namespace ns3
{

class UavMobilityModel : public MobilityModel
{
public:
  /**
   * Register this type with the TypeId system.
   * \return the object TypeId
   */
  static TypeId GetTypeId(void);

  void SetSpeed(double speed);
  double GetSpeed() const;

  void SetFirstPosition(const Vector &position);

private:
  // void Rebound (Time timeLeft);
  void DoDispose();
  void DoStop();
  void DoInitializePrivate(void);
  virtual Vector DoGetPosition(void) const;
  virtual void DoSetPosition(const Vector &position);
  virtual Vector DoGetVelocity(void) const;
  void UpdatePosition(); //  utilizado para atualizar posicionamento dos UAVs e manter um valor mais realistico com a 'movimentacao'

  ConstantVelocityHelper m_helper;
  EventId m_event;
  EventId m_envPos;
  double m_speed;
  Vector m_goTo; //
  Time m_updatePosition;

  ns3::TracedCallback<Ptr<const MobilityModel> > m_courseChangeDevice;
};

} // namespace ns3

#endif /* UAV_MOBILITY_H */
