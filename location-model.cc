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
#include "location-model.h"
#include "client-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LocationModel");

NS_OBJECT_ENSURE_REGISTERED(LocationModel);

TypeId
LocationModel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::LocationModel")
                          .SetParent<Object>()
                          .SetGroupName("Flynetwork-Model")
                          .AddConstructor<LocationModel>()
                          .AddAttribute("Id",
                                        "Uav model id",
                                        UintegerValue(-1),
                                        MakeUintegerAccessor(&LocationModel::m_id),
                                        MakeUintegerChecker<uint32_t>());
  return tid;
}

LocationModel::LocationModel()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("LocationModel::LocationModel @" << Simulator::Now().GetSeconds());
  m_used = false;
  m_totaCli = 0;
  m_totalConsumption = 0.0;
}

LocationModel::~LocationModel()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("LocationModel::~LocationModel @" << Simulator::Now().GetSeconds());
  m_position.clear();
  m_childList.Clear();
  m_pljci.clear();
}

void LocationModel::SetId(uint32_t id)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<id);
  m_id = id;
}

uint32_t
LocationModel::GetId()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_id;
}

void LocationModel::SetPosition(double x, double y)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<x<<y);
  m_position.clear();
  m_position.push_back(x);
  m_position.push_back(y);
}

const std::vector<double>
LocationModel::GetPosition()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_position;
}

void LocationModel::SetPosition(double x, double y, double r_max)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<x<<y);
  m_position.clear();
  m_position.push_back(x*r_max);
  m_position.push_back(y*r_max);
}

const std::vector<double>
LocationModel::GetPosition(double r_max)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  std::vector<double> p;
  p.push_back(m_position.at(0)/r_max);
  p.push_back(m_position.at(1)/r_max);
  return p;
}

bool LocationModel::IsUsed() {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_used;
}

void LocationModel::SetUsed () {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  m_used = true;
}

double LocationModel::GetTotalConsumption ()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_totalConsumption;
}

void LocationModel::SetTotalConsumption (double v)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<v);
  m_totalConsumption = v;
}

void LocationModel::DoDispose () {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_DEBUG ("LocationModel::Dispose id "<< m_id << " REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
}

void LocationModel::SetTotalCli (int t) {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<t);
  m_totaCli = t;
}

int LocationModel::GetTotalCli () {
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  return m_totaCli;
}

void LocationModel::IniciarMovimentoA () {
  m_positionA.clear();
  m_positionA.push_back(m_position.at(0));
  m_positionA.push_back(m_position.at(1));
}

bool LocationModel::MovimentoA () {
  if (m_position.at(0) == m_positionA.at(0) && m_position.at(1) == m_positionA.at(1)) {
    return true;
  }
  return false;
}

void LocationModel::IniciarMovimentoB () {
  m_positionB.clear();
  m_positionB.push_back(m_position.at(0));
  m_positionB.push_back(m_position.at(1));
}

bool LocationModel::MovimentoB () {
  if (m_position.at(0) == m_positionB.at(0) && m_position.at(1) == m_positionB.at(1)) {
    return true;
  }
  return false;
}

void LocationModel::SetPunishCapacity (double pn) {
  m_punshCapacity = pn;
}

double LocationModel::GetPunishCapacity () {
  return m_punshCapacity;
}

void LocationModel::SetPunishNeighboor (double pn) {
  m_punshNeigh = pn;
}

double LocationModel::GetPunishNeighboor () {
  return m_punshNeigh;
}

void LocationModel::InitializeWij (double v) {
  m_wij = v;
}

void LocationModel::NewClient (double Wi) {
  m_wij += Wi;
}

void LocationModel::RemoveClient (double Wi) {
  m_wij -= Wi;
}

double LocationModel::GetWij () {
  return m_wij;
}

void LocationModel::SetTempPljci (double pljci) {
  m_tempPljci = pljci;
}

void LocationModel::AddPljCi (Ptr<ClientModel> ci, double Zci, double r_max) {
  m_pljci[ci] = m_tempPljci/Zci;
  // calculando parte do novo posicionamento da localização
  m_xAcum += ci->GetPci()*m_tempPljci*ci->GetXPosition(r_max);
  m_yAcum += ci->GetPci()*m_tempPljci*ci->GetYPosition(r_max);
  m_plj += ci->GetPci()*m_tempPljci;
}

bool LocationModel::SetFather (Ptr<LocationModel> l, double dist, double uav_cob, double r_max) {
  m_father = l;
  // atualizando parte do novo posicionamento da localizacao
  m_xAcum = l->GetXPosition(r_max) * m_punshNeigh;
  m_yAcum = l->GetYPosition(r_max) * m_punshNeigh;

  // if (dist > uav_cob) { não faz sentido validar conexão, pois este calculo irá aliviar ou punir dependendo da distancia para com o pai.
  m_punshNeigh = m_punshNeigh * std::exp (-1+(dist/uav_cob));
  // }

  return dist>=uav_cob;
}

Ptr<LocationModel> LocationModel::GetFather () {
  return m_father;
}

void LocationModel::AddChild (Ptr<LocationModel> l, double r_max) {
  m_childList.Add(l);
  m_xAcum += l->GetXPosition(r_max) * 0.5; // PENSAR: m_punishNeigh -> é interessante somente para manter o UAV próximo ao pai, para garantir conexão, não sei se vale a pena forçar com a mesma intensidade no sentido dos clientes.
  m_yAcum += l->GetYPosition(r_max) * 0.5; // ALTERADO: Modificado para 50%! Considerando como peso os filhos somente no valor de 50%!
}

void LocationModel::ClearChildList () {
  m_childList.Clear();
}

LocationModelContainer LocationModel::GetChildList () {
  return m_childList;
}

void LocationModel::LimparAcumuladoPosicionamento () {
  m_xAcum = m_yAcum = 0.0;
  m_plj = 0.0;
}

double LocationModel::GetXPosition (double r_max) {
  return m_position.at(0)/r_max;
}

double LocationModel::GetYPosition (double r_max) {
  return m_position.at(1)/r_max; 
}

double LocationModel::GetXAcum() {
  return m_xAcum;
}

double LocationModel::GetYAcum() {
  return m_yAcum;
}

double LocationModel::GetPlj() {
  return m_plj;
}

double LocationModel::GetChildListSize() {
  return (double)m_childList.GetN();
}

bool LocationModel::IsConnected () {
  return m_connected;
}

bool LocationModel::ValidarCapacidade (double wj, double taxa_capacidade) {
  if (m_wij > wj) {
    // atualizar taxa de punicao
    m_punshCapacity *= taxa_capacidade;
    return false;
  } else {
    m_punshCapacity *= 0.6; // NOVO: reduz 60%
  }
  return true;
}

} // namespace ns3
