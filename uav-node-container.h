/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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

#ifndef UAV_NODE_CONTAINER_H
#define UAV_NODE_CONTAINER_H

#include "uav-model.h"
#include "ns3/node-container.h"
#include <vector>
#include <stdint.h>

namespace ns3
{

/**
 * \ingroup FlyNetwork
 */
class UavNodeContainer : public NodeContainer
{
  public:
    void Clear();
    Ptr<Node> RemoveAt (uint32_t pos);
    Ptr<Node> RemoveId (uint32_t id);
    uint32_t CheckId (uint32_t id); // verificar se o UAV está no container
};

}

#endif /* UAV_NODE_CONTAINER_H */
