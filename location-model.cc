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
  m_changePosition = true;
  m_totaCli = 0;
  m_totalConsumption = 0.0;
  m_xAcumCli = m_yAcumCli = m_plj = 0.0;
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

  if (m_changePosition) { // verificar issue: https://github.com/ggarciabas/flynetwork/issues/10
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
    m_historico.push_back(m_position); // OBs: push_back faz cópia ou não?! Se nao fizer vai falar a estratégia!!
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
  m_totalConsumption = 0.0;
  m_totaCli = 0;
}

void LocationModel::NewClient (double dataRate, double cons) {
  m_wij += dataRate;
  m_totalConsumption += cons;
  m_totaCli++;
}

void LocationModel::RemoveClient (double dataRate, double cons) {
  m_wij -= dataRate;
  m_totalConsumption -= cons;
  m_totaCli--;
}

double LocationModel::GetWij () {
  return m_wij;
}

void LocationModel::SetTempPljci (double pljci) {
  m_tempPljci = pljci;
  // std::cout << " " << pljci;
}

double LocationModel::AddPljCiPuro (Ptr<ClientModel> ci, double Zci, double r_max) {
  // calculando parte do novo posicionamento da localização
  m_xAcumCli += ci->GetPci()*(m_tempPljci/Zci)*ci->GetXPosition();
  m_yAcumCli += ci->GetPci()*(m_tempPljci/Zci)*ci->GetYPosition();
  m_plj += ci->GetPci()*(m_tempPljci/Zci);
  return (m_tempPljci/Zci);
}

void LocationModel::AddPljCi (Ptr<ClientModel> ci, double Zci, double r_max) {
  m_pljci[ci] = m_tempPljci/Zci;
  // calculando parte do novo posicionamento da localização
  m_xAcumCli += ci->GetPci()*(m_tempPljci/Zci)*ci->GetXPosition(r_max);
  m_yAcumCli += ci->GetPci()*(m_tempPljci/Zci)*ci->GetYPosition(r_max);
  m_plj += ci->GetPci()*(m_tempPljci/Zci);
}

bool LocationModel::UpdatePunishNeighboor (double sinrUavMin) {
  if (m_sinrFather >= sinrUavMin) {
    m_punshNeigh *=  0.9;
    m_punshNeigh = (m_punshNeigh>0.01)?m_punshNeigh:0.01;
    m_connected = false;
  } else {
    m_punshNeigh *= 1.1;
    m_punshNeigh = (m_punshNeigh > 2) ? 2 : m_punshNeigh;
    m_connected = true;
  }

  return m_connected;
}

void LocationModel::SetFather (Ptr<LocationModel> l, double dist, double r_max, double prRef, double pi, double comp_onda, double ptUav, double gain, double fsInterf, double N, double sinrUavMin) {

  double pl = 10*3.32*std::log10(dist*r_max)+4.48; // dB - Beta e sigma para ambientes outdoor - LogDistance (ver dissertacao)
  double pr = std::pow(10,((prRef - pl)/*dBm*/-30)/10.0); // W
  double it = fsInterf*pr; // W -- convertido pra W
  double sinr = (10*std::log10((pr / (it+N))/*W*/))/*dB*/+30; // dBm

  if (sinr >= sinrUavMin) {
    std::cout << "---------> UAV Distancia: " << dist*r_max << std::endl;
    m_connected = true;
  } else {
    m_connected = false;
  }

  // FUTURO: pensar na metrica para futuro
  // m_father->UavConsumption(-dataRate); // removendo o consumo do pai anterior
  // l->UavConsumption(dataRate); // adicionando consumo no pai atual

  // atualizando parte do novo posicionamento da localizacao
  m_xAcum += l->GetXPosition(r_max);
  m_yAcum += l->GetYPosition(r_max);
  m_sinrFather = sinr;
  m_father = l;
}

Ptr<LocationModel> LocationModel::GetFather () {
  return m_father;
}

void LocationModel::AddChild (Ptr<LocationModel> l, double r_max) {
  m_childList.Add(l);
  m_xAcum += l->GetXPosition(r_max); // PENSAR: m_punishNeigh -> é interessante somente para manter o UAV próximo ao pai, para garantir conexão, não sei se vale a pena forçar com a mesma intensidade no sentido dos clientes.
  m_yAcum += l->GetYPosition(r_max); // ESTÁ NA EQUAÇAO, NAO PODE MUDAR -- : Modificado para 50%! Considerando como peso os filhos somente no valor de 50%!
}

void LocationModel::ClearChildList () {
  m_childList.Clear();
}

LocationModelContainer LocationModel::GetChildList () {
  return m_childList;
}

void LocationModel::LimparAcumuladoPosicionamentoClientes () {
  m_xAcumCli = m_yAcumCli = 0.0;
  m_plj = 0.0;
}

void LocationModel::LimparAcumuladoPosicionamento () {
  m_xAcum = m_yAcum = 0.0;
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

double LocationModel::GetXAcum() {
  return m_xAcum;
}

double LocationModel::GetYAcum() {
  return m_yAcum;
}

double LocationModel::GetXAcumCli() {
  return m_xAcumCli;
}

double LocationModel::GetYAcumCli() {
  return m_yAcumCli;
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

bool LocationModel::ValidarCapacidade (double maxDrUav, double taxa_capacidade) {
  if (m_wij > maxDrUav) {
    // atualizar taxa de punicao
    m_punshCapacity *= taxa_capacidade;
    return false;
  } else {
    m_punshCapacity *= 0.6; // NOVO: reduz 60%
    m_punshCapacity = (m_punshCapacity<0.01)?0.01:m_punshCapacity; // define um minimo!
  }
  return true;
}

void LocationModel::LimparMapaPljci () {
  m_pljci.clear();
}

}
