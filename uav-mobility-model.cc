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
#include "uav-mobility-model.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <cmath>

#include "global-defines.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavMobilityModel");

NS_OBJECT_ENSURE_REGISTERED(UavMobilityModel);

TypeId
UavMobilityModel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::UavMobilityModel")
                          .SetParent<MobilityModel>()
                          .SetGroupName("Flynetwork-Mobility")
                          .AddConstructor<UavMobilityModel>()
                          .AddAttribute("Speed", "The speed (m/s).",
                                        DoubleValue(global_speed),
                                        MakeDoubleAccessor(&UavMobilityModel::SetSpeed,
                                                           &UavMobilityModel::GetSpeed),
                                        MakeDoubleChecker<double>(1, 10000))
                          .AddAttribute("UpdatePosition", "Time to update position (s)",
                                        TimeValue (Seconds (0.001)),
                                        MakeTimeAccessor (&UavMobilityModel::m_updatePosition),
                                        MakeTimeChecker ())
                          .AddAttribute("FirstPosition", "The first position",
                                        VectorValue(Vector(0.0, 0.0, 0.0)),
                                        MakeVectorAccessor(&UavMobilityModel::SetFirstPosition),
                                        MakeVectorChecker())
                           .AddTraceSource ("CourseChangeDevice",
                                            "",
                                            MakeTraceSourceAccessor (&UavMobilityModel::m_courseChangeDevice),
                                            "ns3::MobilityModel::TracedCallback");
  return tid;
}

void UavMobilityModel::SetFirstPosition (const Vector &position)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds()  << position);
  NS_LOG_DEBUG("UavMobilityModel::SetFirstPosition @" << Simulator::Now().GetSeconds());
  m_helper.SetPosition(position);
  NS_ASSERT(m_helper.GetCurrentPosition().x == position.x && m_helper.GetCurrentPosition().y == position.y);
}

void UavMobilityModel::DoInitializePrivate(void)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_helper.Update(); // atualiza o constantvelociyt model
  Vector position = m_helper.GetCurrentPosition();

  double distance = std::sqrt(std::pow((position.x - m_goTo.x), 2) + std::pow((position.y - m_goTo.y), 2)); // hipotenusa

  double dis_speed_perc = m_speed/distance; // para saber a proporcao da velocidade para distancia

  double x_speed = (m_goTo.x - position.x) * dis_speed_perc;
  double y_speed = (m_goTo.y - position.y) * dis_speed_perc;

  Vector vec (x_speed, y_speed, 0.0); // m/s

  m_helper.SetVelocity(vec);
  m_helper.Unpause(); // allow Update method to be called

  Time delayLeft = Seconds(distance / m_speed);

  NS_LOG_DEBUG("UavMobilityModel::DoInitializePrivate tempo[" << delayLeft.GetSeconds() << "] distancia[" << distance << "] @" << Simulator::Now().GetSeconds());

  m_event = Simulator::Schedule(delayLeft, &UavMobilityModel::DoStop, this);
  m_envPos = Simulator::Schedule(m_updatePosition, &UavMobilityModel::UpdatePosition, this);
}

void UavMobilityModel::DoStop()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_helper.Update();
  m_helper.Pause(); // set the vector to 0.0 and block Update method
  Simulator::Remove(m_event);
  Simulator::Remove(m_envPos);

  Vector position = m_helper.GetCurrentPosition();
  NS_LOG_DEBUG ("UavMobilityModel :: [[ FINAL ]] " << Simulator::Now().GetSeconds() << " (" << position.x << "," << position.y << ") deveria estar em (" << m_goTo.x << "," << m_goTo.y << ")");
  
  NotifyCourseChange();
  m_courseChangeDevice(this);
}

void UavMobilityModel::UpdatePosition()
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_helper.Update(); // https://www.nsnam.org/doxygen/constant-velocity-helper_8cc_source.html#l00080
  m_courseChangeDevice(this); // notificacoes intermediarias - somente para o dispositivo para consumo energético
  m_envPos = Simulator::Schedule(m_updatePosition, &UavMobilityModel::UpdatePosition, this);
}

Vector
UavMobilityModel::DoGetPosition(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  m_helper.Update();
  return m_helper.GetCurrentPosition();
}

void UavMobilityModel::DoSetPosition(const Vector &goto_)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() << goto_);
  Vector position = m_helper.GetCurrentPosition();
  NS_LOG_DEBUG ("UavMobilityModel::DoSetPosition [" << goto_.x << "," << goto_.y << "] <--- [" << position.x << "," << position.y << "] @" << Simulator::Now().GetSeconds());
  if (CalculateDistance(goto_, position) >= 1e-3) { // caso onde se tenha que ir seja distante, entao avancar!
    m_goTo = goto_; // posicao destino
    Simulator::Remove(m_event);
    Simulator::Remove(m_envPos);
    m_event = Simulator::ScheduleNow(&UavMobilityModel::DoInitializePrivate, this);
  }
}

Vector
UavMobilityModel::DoGetVelocity(void) const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_helper.GetVelocity();
}

void UavMobilityModel::SetSpeed(double speed)
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() <<speed);
  m_speed = speed;
}

double
UavMobilityModel::GetSpeed() const
{
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  return m_speed;
}

void UavMobilityModel::DoDispose() {
  NS_LOG_FUNCTION(this << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("UavMobilityModel::DoDispose");

  m_helper.Pause();
  Simulator::Remove(m_event);
  Simulator::Remove(m_envPos);
}

} // namespace ns3
