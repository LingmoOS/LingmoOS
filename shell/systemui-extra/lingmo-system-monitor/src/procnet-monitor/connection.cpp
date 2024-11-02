/*
 * connection.cpp
 *
 * Copyright (c) 2004-2006,2008 Arnout Engelen
 *               2021 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 *USA.
 *
 */

#include <iostream>
#include <cassert>
#ifdef __APPLE__
#include <sys/malloc.h>
#elif __FreeBSD__
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include "nethogs.h"
#include "connection.h"
#include "process.h"

ConnList *connections = NULL;

void PackList::add(Packet *p) {
  if (content == NULL) {
    content = new PackListNode(new Packet(*p));
    return;
  }

  if (content->val->time.tv_sec == p->time.tv_sec) {
    content->val->len += p->len;
    return;
  }

  /* store copy of packet, so that original may be freed */
  content = new PackListNode(new Packet(*p), content);
}

/* sums up the total bytes used and removes 'old' packets */
u_int32_t PackList::sumanddel(timeval t) {
  u_int32_t retval = 0;
  PackListNode *current = content;
  PackListNode *previous = NULL;

  while (current != NULL) {
    // qDebug() << "Comparing " << current->val->time.tv_sec << " <= " <<
    // t.tv_sec - PERIOD << endl;
    if (current->val->time.tv_sec <= t.tv_sec - PERIOD) {
      if (current == content)
        content = NULL;
      else if (previous != NULL)
        previous->next = NULL;
      delete current;
      return retval;
    }
    retval += current->val->len;
    previous = current;
    current = current->next;
  }
  return retval;
}

/* packet may be deleted by caller */
Connection::Connection(Packet *packet) {
  assert(packet != NULL);
  connections = new ConnList(this, connections);
  sent_packets = new PackList();
  recv_packets = new PackList();
  sumSent = 0;
  sumRecv = 0;
  if (0) {
    qDebug() << "New connection, with package len " << packet->len;
  }
  if (packet->Outgoing()) {
    sumSent += packet->len;
    sent_packets->add(packet);
    refpacket = new Packet(*packet);
  } else {
    sumRecv += packet->len;
    recv_packets->add(packet);
    refpacket = packet->newInverted();
  }
  lastpacket = packet->time.tv_sec;
  if (0)
    qDebug() << "New reference packet created at " << refpacket;
}

Connection::~Connection() {
  if (0)
    qDebug() << "Deleting connection";
  /* refpacket is not a pointer to one of the packets in the lists
   * so deleted */
  delete (refpacket);
  if (sent_packets != NULL)
    delete sent_packets;
  if (recv_packets != NULL)
    delete recv_packets;

  ConnList *curr_conn = connections;
  ConnList *prev_conn = NULL;
  while (curr_conn != NULL) {
    if (curr_conn->getVal() == this) {
      ConnList *todelete = curr_conn;
      curr_conn = curr_conn->getNext();
      if (prev_conn == NULL) {
        connections = curr_conn;
      } else {
        prev_conn->setNext(curr_conn);
      }
      delete (todelete);
    } else {
      prev_conn = curr_conn;
      curr_conn = curr_conn->getNext();
    }
  }
}

/* the packet will be freed by the calling code */
void Connection::add(Packet *packet) {
  lastpacket = packet->time.tv_sec;
  if (packet->Outgoing()) {
    if (DEBUG) {
      qDebug() << "Outgoing: " << packet->len;
    }
    sumSent += packet->len;
    sent_packets->add(packet);
  } else {
    if (DEBUG) {
      qDebug() << "Incoming: " << packet->len;
    }
    sumRecv += packet->len;
    if (DEBUG) {
      qDebug() << "sumRecv now: " << sumRecv;
    }
    recv_packets->add(packet);
  }
}

Connection *findConnectionWithMatchingSource(Packet *packet) {
  assert(packet->Outgoing());

  ConnList *current = connections;
  while (current != NULL) {
    /* the reference packet is always outgoing */
    if (packet->matchSource(current->getVal()->refpacket)) {
      return current->getVal();
    }

    current = current->getNext();
  }
  return NULL;
}

Connection *findConnectionWithMatchingRefpacketOrSource(Packet *packet) {
  ConnList *current = connections;
  while (current != NULL) {
    /* the reference packet is always *outgoing* */
    if (packet->match(current->getVal()->refpacket)) {
      return current->getVal();
    }

    current = current->getNext();
  }
  return findConnectionWithMatchingSource(packet);
}

/*
 * finds connection to which this packet belongs.
 * a packet belongs to a connection if it matches
 * to its reference packet
 */
Connection *findConnection(Packet *packet) {
  if (packet->Outgoing())
    return findConnectionWithMatchingRefpacketOrSource(packet);
  else {
    Packet *invertedPacket = packet->newInverted();
    Connection *result =
        findConnectionWithMatchingRefpacketOrSource(invertedPacket);

    delete invertedPacket;
    return result;
  }
}

/*
 * Connection::sumanddel
 *
 * sums up the total bytes used
 * and removes 'old' packets.
 *
 * Returns sum of sent packages (by address)
 *	   sum of received packages (by address)
 */
void Connection::sumanddel(timeval t, u_int32_t *recv, u_int32_t *sent) {
  (*sent) = (*recv) = 0;

  *sent = sent_packets->sumanddel(t);
  *recv = recv_packets->sumanddel(t);
}
