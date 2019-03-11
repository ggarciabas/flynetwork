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

#include "uav-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UavModel");

NS_OBJECT_ENSURE_REGISTERED(UavModel);

TypeId
UavModel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::UavModel")
                          .SetParent<Object>()
                          .SetGroupName("Flynetwork-Model")
                          .AddConstructor<UavModel>()
                          .AddAttribute("Id",
                                        "Uav model id",
                                        UintegerValue(-1),
                                        MakeUintegerAccessor(&UavModel::m_id),
                                        MakeUintegerChecker<uint32_t>())
                          .AddAttribute("AddressAdhoc", "The Ipv4 address of the node",
                                        Ipv4AddressValue(),
                                        MakeIpv4AddressAccessor(&UavModel::m_addressAdhoc),
                                        MakeIpv4AddressChecker())
                          .AddAttribute("TotalBattery", "Total de bateria do UAV",
                                        DoubleValue(0),
                                        MakeDoubleAccessor(&UavModel::m_totalBattery),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("TotalEnergy", "Total de energia do UAV",
                                        DoubleValue(156960), // in Joules
                                        MakeDoubleAccessor(&UavModel::m_totalEnergy),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("Socket", "Point to the socket",
                                        PointerValue(),
                                        MakePointerAccessor(&UavModel::m_socket),
                                        MakePointerChecker<Socket>())
                          .AddAttribute("EnergyCost",
                                        "Initial energy cost per meter for basic energy source.",
                                        DoubleValue(0), // joule/meter
                                        MakeDoubleAccessor(&UavModel::m_energyCost),
                                        MakeDoubleChecker<double>())
                          .AddAttribute("HoverCost",
                                        "Initial hover cost per second for basic energy source.",
                                        DoubleValue(0), // joule/s
                                        MakeDoubleAccessor(&UavModel::m_hoverCost),
                                        MakeDoubleChecker<double>())
      ;
  return tid;
}

UavModel::UavModel() : m_position()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  // NS_LOG_DEBUG ("UavModel::UavModel @" << Simulator::Now().GetSeconds());
  m_confirmed = true; // para que nao de erro na primera execucao
  m_clientData = true; // para que nao de erro na primeira execucao
  m_depletion = false;
}

UavModel::~UavModel()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  // NS_LOG_DEBUG ("UavModel::~UavModel @" << Simulator::Now().GetSeconds());
}

void
UavModel::SetSendPositionEvent (EventId id)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds());
  m_sendPosition = id;
}

void
UavModel::CancelSendPositionEvent()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_sendPosition);
}

void UavModel::SetDepletion (bool t) {
  m_depletion = t;
}

bool UavModel::IsDepletion () {
  return m_depletion;
}

void UavModel::SetAskCliDataEvent (EventId id)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds());
  m_askCliData = id;
}

void UavModel::CancelAskCliDataEvent()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  Simulator::Remove(m_askCliData);
}

void
UavModel::SetSocket(Ptr<Socket> s) {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<s);
  m_socket = s;
}

Ptr<Socket>
UavModel::GetSocket(){
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_socket;
}

double
UavModel::CalculateEnergyCost(double dist)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<dist);
  return m_energyCost * dist; // in joule
}

double
UavModel::GetTotalBattery()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_totalBattery;
}

double 
UavModel::GetHoverCost(){
  return m_hoverCost;
}

void UavModel::SetTotalEnergy(double total)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<total);
  m_totalEnergy = total;
}

double
UavModel::GetTotalEnergy()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_totalEnergy;
}

void UavModel::SetId(uint32_t id)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<id);
  m_id = id;
}

uint32_t
UavModel::GetId()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_id;
}

void UavModel::SetNewPosition(std::vector<double> pos)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<pos);
  SetNewPosition(pos.at(0), pos.at(1));
}

void UavModel::SetPosition(double x, double y)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<x<<y);
  m_position.clear();
  m_position.push_back(x);
  m_position.push_back(y);
}

const std::vector<double>
UavModel::GetPosition()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_position;
}

void UavModel::SetNewPosition(double x, double y)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<x<<y);
  m_newPos.clear();
  m_newPos.push_back(x);
  m_newPos.push_back(y);
}

const std::vector<double>
UavModel::GetNewPosition()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_newPos;
}

Ipv4Address
UavModel::GetAddressAdhoc()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_addressAdhoc;
}

bool
UavModel::IsConfirmed ()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() << m_confirmed);
  return m_confirmed;
}

void
UavModel::NotConfirmed ()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() << m_confirmed);
  m_confirmed = false;
}

void
UavModel::ConfirmPosition () {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("UAV [ " << m_id << "] (" << m_position.at(0) << "," << m_position.at(1) << ") === > (" << m_newPos.at(0) << "," << m_newPos.at(1) << ") @" << Simulator::Now().GetSeconds());
  if (int(m_newPos.at(0)) == int(m_position.at(0)) && int(m_newPos.at(1)) == int(m_position.at(1))) { // compara com a posicao que se deseja que ele chegue 
    m_confirmed = true; 
  } else {
    m_confirmed = false;
  }
}

void UavModel::DoDispose () {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  CancelAskCliDataEvent();
  CancelSendPositionEvent();
  // NS_LOG_DEBUG("UavModel::DoDispose id " << m_id << " REF " << GetReferenceCount() << " SKT REF " << m_socket->GetReferenceCount() << "@" << Simulator::Now().GetSeconds());
  m_socket->ShutdownRecv();
  m_socket->ShutdownSend();
  m_socket->Close();
  m_socket = 0;
}

bool UavModel::ClientDataConfirmed ()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() << m_clientData);
  return m_clientData;
}

void UavModel::SetClientDataConfirmed (bool b)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() << b);
  m_clientData = b;
}

} // namespace ns3
