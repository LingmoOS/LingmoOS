/*
 * inode2prog.cpp
 *
 * Copyright (c) 2005,2006,2008,2009 Arnout Engelen
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

#include <sys/types.h>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <ctype.h>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <map>
#include <sys/stat.h>
#include <fcntl.h>
#include <climits>
#include <sys/times.h>
#include <set>
#include <algorithm>
#include <QDir>

#include "inode2prog.h"

extern bool bughuntmode;

// Not sure, but assuming there's no more PID's than go into 64 unsigned bits..
const int MAX_PID_LENGTH = 20;

// Max length of filenames in /proc/<pid>/fd/*. These are numeric, so 10 digits
// seems like a safe assumption.
const int MAX_FDLINK = 10;

/* maps from inode to program-struct */
std::map<unsigned long, prg_node *> inodeproc;

bool is_number(const char *string) {
  while (*string) {
    if (!isdigit(*string))
      return false;
    string++;
  }
  return true;
}

unsigned long str2ulong(const char *ptr) {
  unsigned long retval = 0;

  while ((*ptr >= '0') && (*ptr <= '9')) {
    retval *= 10;
    retval += *ptr - '0';
    ptr++;
  }
  return retval;
}

int str2int(const char *ptr) {
  int retval = 0;

  while ((*ptr >= '0') && (*ptr <= '9')) {
    retval *= 10;
    retval += *ptr - '0';
    ptr++;
  }
  return retval;
}

static std::string read_file(int fd) {
  char buf[255];
  std::string content;

  for (int length; (length = read(fd, buf, sizeof(buf))) > 0;) {
    if (length < 0) {
      qCritical()<<"Error reading file:"<<std::strerror(errno);
      return "";
    }
    content.append(buf, length);
  }

  return content;
}

static std::string read_file(const char *filepath) {
  int fd = open(filepath, O_RDONLY);

  if (fd < 0) {
    qCritical()<<QString("Error opening %1: %2\n").arg(filepath).arg(std::strerror(errno));
    return "";
  }

  std::string contents = read_file(fd);

  if (close(fd)) {
    qCritical()<<QString("Error closing %1: %2\n").arg(filepath).arg(std::strerror(errno));
    return "";
  }

  return contents;
}

std::string getprogname(pid_t pid) {
  const int maxfilenamelen = 14 + MAX_PID_LENGTH + 1;
  char filename[maxfilenamelen];

  std::snprintf(filename, maxfilenamelen, "/proc/%d/cmdline", pid);
  return read_file(filename);
}

void setnode(unsigned long inode, pid_t pid) {
  prg_node *current_value = inodeproc[inode];

  if (current_value == NULL || current_value->pid != pid) {
    prg_node *newnode = new prg_node;
    newnode->inode = inode;
    newnode->pid = pid;
    newnode->name = getprogname(pid);

    inodeproc[inode] = newnode;
    delete current_value;
  }
}

void get_info_by_linkname(const char *pid, const char *linkname) {
  if (strncmp(linkname, "socket:[", 8) == 0) {
    setnode(str2ulong(linkname + 8), str2int(pid));
  }
}

/* updates the `inodeproc' inode-to-prg_node
 * for all inodes belonging to this PID
 * (/proc/pid/fd/42)
 * */
void get_info_for_pid(const char *pid) {
  char dirname[10 + MAX_PID_LENGTH];

  size_t dirlen = 10 + strlen(pid);
  snprintf(dirname, sizeof(dirname), "/proc/%s/fd", pid);

  QString m_name(dirname);
  QDir qdir(m_name);

  if (!qdir.exists()) {
      return;
  }

  QFileInfoList fileInfoList = qdir.entryInfoList(QDir::NoFilter);
  foreach (auto fileInfo, fileInfoList) {
    if(fileInfo.isSymLink()) {
      QString linkName = fileInfo.readLink();
      if (fileInfo.readLink().contains(m_name)) {
        QStringList nameList =   fileInfo.readLink().split("/");
        linkName =  nameList.at(nameList.size()-1);
      }
      if (!linkName.isEmpty()) {
        get_info_by_linkname(pid, linkName.toUtf8().data());
      }
    }
  }
}

/* updates the `inodeproc' inode-to-prg_node mapping
 * for all processes in /proc */
void reread_mapping() {
  DIR *proc = opendir("/proc");

  if (proc == 0) {
    qCritical() << "Error reading /proc, needed to get inode-to-pid-maping\n";
    return ;
  }

  dirent *entry;

  while ((entry = readdir(proc))) {
    if (entry->d_type != DT_DIR)
      continue;

    if (!is_number(entry->d_name))
      continue;

    get_info_for_pid(entry->d_name);
  }
  closedir(proc);
}

struct prg_node *findPID(unsigned long inode) {
  /* we first look in inodeproc */
  struct prg_node *node = inodeproc[inode];

  if (node != NULL) {
    if (bughuntmode) {
      qDebug() << ":) Found pid in inodeproc table";
    }
    return node;
  }

#ifndef __APPLE__
  reread_mapping();
#endif

  struct prg_node *retval = inodeproc[inode];
  if (bughuntmode) {
    if (retval == NULL) {
      qDebug() << ":( No pid after inodeproc refresh";
    } else {
      qDebug() << ":) Found pid after inodeproc refresh";
    }
  }
  return retval;
}

void prg_cache_clear(){};

/*void main () {
        qDebug() << "Fooo\n";
        reread_mapping();
        qDebug() << "Haihai\n";
}*/

static quad_t get_ms() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<quad_t>(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

static void get_pids(std::set<pid_t> *pids) {
  DIR *proc = opendir("/proc");
  if (proc == 0) {
    qDebug() << "Error reading /proc, needed to get pid set";
    return;
  }
  dirent *entry;

  while ((entry = readdir(proc))) {
    if (entry->d_type != DT_DIR)
      continue;

    if (!is_number(entry->d_name))
      continue;

    pids->insert(str2int(entry->d_name));
  }
  closedir(proc);
}

void garbage_collect_inodeproc() {
  static quad_t last_ms = 0;
  quad_t start_ms = 0;
  if (bughuntmode) {
    start_ms = get_ms();
    if (last_ms) {
      qDebug() << "PERF: GC interval: " << start_ms - last_ms << "[ms]";
    }
  }

  std::set<pid_t> pids;
  get_pids(&pids);
  if (pids.size() == 0) {
    return;
  }

  for (std::map<unsigned long, prg_node *>::iterator it = inodeproc.begin();
       it != inodeproc.end();) {
    if (!it->second || pids.find(it->second->pid) != pids.end()) {
      ++it;
      continue;
    }

    if (bughuntmode) {
      qDebug() << "GC prg_node (inode=" << it->first
                << ", pid=" << it->second->pid
                << ", cmdline=" << it->second->name.c_str() << ")";
    }
    delete it->second;
    inodeproc.erase(it++);
  }

  if (bughuntmode) {
    last_ms = get_ms();
    qDebug() << "PERF: GC proctime: " << last_ms - start_ms << "[ms]";
  }
}
