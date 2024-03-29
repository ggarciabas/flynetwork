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
#include "client-model-container.h"

# include <math.h>

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
  NS_LOG_INFO ("LocationModel::LocationModel @" << Simulator::Now().GetSeconds());
  m_used = false;
  m_totaCli = 0;
  m_totalConsumption = 0.0;
  m_changePosition = true;
  m_totaCli = 0;
  m_totalConsumption = 0.0;
  // m_xAcumCli = m_yAcumCli = m_plj = 0.0;
}

LocationModel::~LocationModel()
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  NS_LOG_INFO ("LocationModel::~LocationModel @" << Simulator::Now().GetSeconds());
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

  if (m_changePosition) { // verificar issue: https://github.com/ggarciabas/wifi/issues/10
    // procurar no historico se já passou por esta localizacao
    for (std::vector<std::vector<double> >::iterator i = m_historico.begin(); i != m_historico.end(); ++i) {
      if (x*r_max == (*i).at(0) && y*r_max == (*i).at(1)) {
        // já conhece a posicao, não altera mais! Selecionou ela novamente.
        // std::cout << "#### ---> Posição já avaliada! Nao permite mais trocar de posicao esta localizacao!\n";
        m_changePosition = false;
      }
    }
    m_position.clear();
    m_position.push_back(x*r_max);
    m_position.push_back(y*r_max);
    m_historico.push_back(m_position); // OBs: push_back faz cópia ou não?! Se nao fizer vai falhar a estratégia!!
  }
}

void LocationModel::SetPositionPuro(double x, double y, double r_max)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() <<x<<y);

  m_position.clear();
  m_position.push_back(x);
  m_position.push_back(y);

}

void LocationModel::LimparHistorico () {
  for (std::vector<std::vector<double> >::iterator i = m_historico.begin(); i != m_historico.end(); ++i) {
    (*i).clear();
  }
  m_historico.clear();
  m_changePosition = true; // permite alterar, senão NÉ!
}

std::vector<double>
LocationModel::GetPosition(double r_max)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  std::vector<double> p;
  p.push_back(m_position.at(0)/r_max);
  p.push_back(m_position.at(1)/r_max);
  return p;
}


std::vector<double>
LocationModel::GetPositionA(double r_max)
{
  NS_LOG_FUNCTION(this->m_id << Simulator::Now().GetSeconds() );
  std::vector<double> p;
  p.push_back(m_positionA.at(0)/r_max);
  p.push_back(m_positionA.at(1)/r_max);
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
  NS_LOG_INFO ("LocationModel::Dispose id "<< m_id << " REF " << GetReferenceCount() << " @" << Simulator::Now().GetSeconds());
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
  if (std::abs(m_position.at(0) - m_positionA.at(0)) < 0.1 && std::abs(m_position.at(1) - m_positionA.at(1)) < 0.1) {// metros
    return false;
  }
  return true;
}

void LocationModel::IniciarMovimentoB () {
  m_positionB.clear();
  m_positionB.push_back(m_position.at(0));
  m_positionB.push_back(m_position.at(1));
}

bool LocationModel::MovimentoB () {
  if (std::abs(m_position.at(0) - m_positionB.at(0)) < 0.1 && std::abs(m_position.at(1) - m_positionB.at(1)) < 0.1) { // metros
    return false;
  }
  return true;
}

void LocationModel::SetPunishNeighboor (double pn) {
  m_punshNeigh = pn;
}

double LocationModel::GetPunishNeighboor () {
  return m_punshNeigh;
}

void LocationModel::InitializeWij (double v) {
  m_wij = v;
  m_totalConsumption = 0.0;
  m_totaCli = 0;
}

double LocationModel::GetMaxDistClient () {
  return m_maxDistCli;
}

void LocationModel::UpdateDistCli (double d) {
  m_maxDistCli = (d>m_maxDistCli) ? d : m_maxDistCli;
}

void LocationModel::NewClient (double dataRate, double cons, double dist, Ptr<ClientModel> cli) {
  m_wij += dataRate;
  m_totalConsumption += cons;
  m_totaCli++;
  m_maxDistCli = (dist>m_maxDistCli) ? dist : m_maxDistCli;
  m_cli.push_back(cli->GetLogin());
}

void LocationModel::RemoveClient (double dataRate, double cons, Ptr<ClientModel> cli) {
  m_wij -= dataRate;
  m_totalConsumption -= cons;
  m_totaCli--;
}

double LocationModel::GetWij () {
  return m_wij;
}

void LocationModel::SetTempPljci (double pljci) {
  m_tempPljci = pljci;
}

double LocationModel::AddPljCiPuro (Ptr<ClientModel> ci, double Zci, double r_max) {
  return (m_tempPljci/Zci);
}

void LocationModel::AddPljCi (Ptr<ClientModel> ci, double Zci, double r_max) {
  m_pljci[ci] = m_tempPljci/Zci;
}

void LocationModel::UpdatePosition (double mx, double my) { // normalizados
  double x_f = 0.0;
  double y_f = 0.0;
  double plj_f = 0.0;
  double x = 0.0;
  double y = 0.0;
  double plj = 0.0;
  NS_LOG_DEBUG("LOGIN\tX\tY\tPLJCI\tPCI");
  for (std::map<Ptr<ClientModel>, double>::iterator ci = m_pljci.begin(); ci != m_pljci.end(); ++ci) {
    NS_LOG_DEBUG((ci->first)->GetLogin() << "\t" << (ci->first)->GetXPosition() << "\t" << (ci->first)->GetYPosition() << "\t" << ci->second << "\t" << (ci->first)->GetPci());
    x += (ci->first)->GetXPosition() * (ci->first)->GetPci() * ci->second;
    y += (ci->first)->GetYPosition() * (ci->first)->GetPci() * ci->second;
    plj += (ci->first)->GetPci() * ci->second;
  }
  NS_LOG_DEBUG ("Client x: " << x << "\ty: " << y << "\tplj: " << plj << "\tx/: " << x/plj << "\ty/: " << y/plj);
  // if (plj > 1.0) {
  //   NS_FATAL_ERROR (" Client -- fuu plj maior que 1!");
  // }
  if (std::isnan(plj)) {
    NS_FATAL_ERROR ("With client -- plj got nan value");
  }
  x_f += x; y_f += y; plj_f += plj; // acumulando

  if (m_childList.GetN() > 0) {
    x = y = plj = 0.0; // limpando  
    NS_LOG_DEBUG ("ID\tX\tY");
    for (LocationModelContainer::Iterator clj = m_childList.Begin(); clj != m_childList.End(); ++clj) {
      NS_LOG_DEBUG ((*clj)->GetId() << "\t" << (*clj)->GetXPosition() << "\t" << (*clj)->GetYPosition());
      x += (*clj)->GetXPosition() * (*clj)->GetPunishNeighboor(); // URGENTE: lembrar de atualizar os documentos! usando Wm e nao Wn ! 
      y += (*clj)->GetYPosition() * (*clj)->GetPunishNeighboor();
      plj += (*clj)->GetPunishNeighboor();
    }
    NS_LOG_DEBUG ("Child x: " << x << "\ty: " << y << "\tplj: " << plj << "\tx/: " << x/plj << "\ty/: " << y/plj);
    x_f += x; y_f += y; plj_f += plj; // acumulando
  }

  x = y = plj = 0.0; // limpando
  x += m_father->GetXPosition() * m_punshNeigh;
  y += m_father->GetYPosition() * m_punshNeigh;
  plj += m_punshNeigh;    
  NS_LOG_DEBUG ("Father x: " << x << "\ty: " << y << "\tplj: " << plj << "\tx/: " << x/plj << "\ty/: " << y/plj);
  x_f += x; y_f += y; plj_f += plj; // acumulando


  NS_LOG_DEBUG ("Final x: " << x_f << "\ty: " << y_f << "\tplj: " << plj_f << "\tx/: " << x_f/plj_f << "\ty/: " << y_f/plj_f);
  x_f /= plj_f;
  y_f /= plj_f;

  m_position.clear();
  m_position.push_back(x_f);
  m_position.push_back(y_f);  

  if (x_f > mx || y_f > my || x_f < 0 || y_f < 0) {
    NS_FATAL_ERROR("Log fora do cenario");
  } 
}

bool LocationModel::UpdatePunishNeighboor (double uav_cob_norm) {
  if (m_distFather <= uav_cob_norm) {
    m_punshNeigh *= std::exp (-1+(m_distFather/uav_cob_norm)); // m_punshNeigh * 0.9; // 
    m_punshNeigh = (m_punshNeigh>0.01)?m_punshNeigh:0.01;
  } else {
    m_punshNeigh *= 1.2;
    m_punshNeigh = (m_punshNeigh > 2) ? 2 : m_punshNeigh;
  }
  return m_connected;
}

void LocationModel::SetFather (Ptr<LocationModel> l, double dist, double r_max, double uav_cob) {

  if (dist*r_max <= uav_cob) {
    m_connected = true;
    m_distFather = dist;
  } else {
    m_connected = false;
    m_distFather = 0.0; // para nao atrapalhar no grafico do cenario
  }

  // FUTURO: pensar na metrica para futuro
  // m_father->UavConsumption(-dataRate); // removendo o consumo do pai anterior
  // l->UavConsumption(dataRate); // adicionando consumo no pai atual

  m_distFather = dist;
  m_father = l;
}

Ptr<LocationModel> LocationModel::GetFather () {
  return m_father;
}

void LocationModel::AddChild (Ptr<LocationModel> l, double r_max) {
  m_childList.Add(l);
}

void LocationModel::ClearChildList () {
  m_childList.Clear();
}

LocationModelContainer LocationModel::GetChildList () {
  return m_childList;
}

double LocationModel::GetXPosition (double r_max) {
  return m_position.at(0)/r_max;
}

double LocationModel::GetYPosition (double r_max) {
  return m_position.at(1)/r_max;
}

double LocationModel::GetXPosition () {
  return m_position.at(0);
}

double LocationModel::GetYPosition () {
  return m_position.at(1);
}

double LocationModel::GetXPositionA () {
  return m_positionA.at(0);
}

double LocationModel::GetYPositionA () {
  return m_positionA.at(1);
}

double LocationModel::GetXFirstPosition()
{
  return m_firstP.at(0);
}

double LocationModel::GetYFirstPosition()
{
  return m_firstP.at(1);
}

void LocationModel::CopyFirstPosition() {
  m_firstP.clear();
  m_firstP.push_back(m_position.at(0));
  m_firstP.push_back(m_position.at(1));
}

double LocationModel::GetChildListSize() {
  return (double)m_childList.GetN();
}

bool LocationModel::IsConnected () {
  return m_connected;
}

bool LocationModel::ValidarCapacidade (double maxDr) {
  if (m_wij <= maxDr) {
    return true;
  }
  return false;
}

void LocationModel::LimparMapaPljci () {
  m_pljci.clear();
}

}
