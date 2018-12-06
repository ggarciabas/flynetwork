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
      ;
  return tid;
}

UavModel::UavModel() : m_position()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("UavModel::UavModel @" << Simulator::Now().GetSeconds());
  m_confirmed = true; // para que nao de erro na primera execucao
  m_clientData = true; // para que nao de erro na primeira execucao
}

UavModel::~UavModel()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_DEBUG ("UavModel::~UavModel @" << Simulator::Now().GetSeconds());
}

void
UavModel::SetSendPositionEvent (EventId id)
{
  m_sendPosition = id;
}

void
UavModel::CancelSendPositionEvent()
{
  Simulator::Remove(m_sendPosition);
}

void
UavModel::SetSendDepletionEvent (EventId id)
{
  m_sendDepletion = id;
}

void
UavModel::CancelSendDepletionEvent()
{
  Simulator::Remove(m_sendDepletion);
}

void UavModel::SetAskCliDataEvent (EventId id)
{
  m_askCliData = id;
}

void
UavModel::SetSendSupplyEvent (EventId id)
{
  m_sendSupply = id;
}

void
UavModel::CancelSendSupplyEvent()
{
  Simulator::Remove(m_sendSupply);
}

void UavModel::CancelAskCliDataEvent()
{
  Simulator::Remove(m_askCliData);
}

void
UavModel::SetSocket(Ptr<Socket> s) {
  m_socket = s;
}

Ptr<Socket>
UavModel::GetSocket(){
  return m_socket;
}

double
UavModel::CalculateEnergyCost(double dist)
{
  return m_energyCost * dist; // in joule
}
double
UavModel::GetTotalBattery()
{
  return m_totalBattery;
}

void UavModel::SetTotalEnergy(double total)
{
  m_totalEnergy = total;
}

double
UavModel::GetTotalEnergy()
{
  return m_totalEnergy;
}

void UavModel::SetId(uint32_t id)
{
  m_id = id;
}

uint32_t
UavModel::GetId()
{
  return m_id;
}

void UavModel::SetNewPosition(std::vector<double> pos)
{
  SetNewPosition(pos.at(0), pos.at(1));
}

void UavModel::SetPosition(double x, double y)
{
  m_position.clear();
  m_position.push_back(x);
  m_position.push_back(y);
}

const std::vector<double>
UavModel::GetPosition()
{
  return m_position;
}

void UavModel::SetNewPosition(double x, double y)
{
  m_newPos.clear();
  m_newPos.push_back(x);
  m_newPos.push_back(y);
}

const std::vector<double>
UavModel::GetNewPosition()
{
  return m_newPos;
}

Ipv4Address
UavModel::GetAddressAdhoc()
{
  return m_addressAdhoc;
}

bool
UavModel::IsConfirmed ()
{
  return m_confirmed;
}

void
UavModel::NotConfirmed ()
{
  m_confirmed = false;
}

void
UavModel::ConfirmPosition () {
  if (int(m_newPos.at(0)) == int(m_position.at(0)) && int(m_newPos.at(1)) == int(m_position.at(1))) { // compara com a posicao que se deseja que ele chegue
    m_confirmed = true;
    NS_LOG_INFO ("UAVMODEL :: posicionamento do uav " << m_id << " confirmado!");
  }
}

void UavModel::DoDispose () {
  CancelAskCliDataEvent();
  CancelSendPositionEvent();
  NS_LOG_DEBUG("UavModel::DoDispose id " << m_id << " REF " << GetReferenceCount() << " SKT REF " << m_socket->GetReferenceCount() << "@" << Simulator::Now().GetSeconds());
  m_socket->ShutdownRecv();
  m_socket->ShutdownSend();
  m_socket->Close();
  m_socket = 0;
}

bool UavModel::ClientDataConfirmed ()
{
  return m_clientData;
}

void UavModel::SetClientDataConfirmed (bool b)
{
  m_clientData = b;
}

} // namespace ns3
