// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUPSSNMP_H
#define CUPSSNMP_H

#include <stdio.h>
#include <string>
#include <vector>
#include <cups/http.h>

using namespace std;

typedef struct	supplysData			/**** Printer supply data ****/
{
  char	name[1024];         /* Name of supply */
  char  color[8];			/* Color: "#RRGGBB" or "none" */
  char  colorName[20];      /*Name of color*/
  int	colorant;			/* Colorant index */
  int   sclass;				/* Supply class */
  int   type;				/* Supply type */
  int   max_capacity;		/* Maximum capacity */
  int   level;				/* Current level value */

  supplysData()
  {
      name[0] = 0;
      strcpy(color, "None");
      strcpy(colorName, "Waste");
      colorant = -1;
      sclass = -1;
      type = -1;
      max_capacity = -1;
      level = -1;
  }

  supplysData(const supplysData& other)
  {
      if(this != &other)
      {
          memset(name, 0, sizeof(name));
          memset(color, 0, sizeof(color));
          memset(colorName, 0, sizeof(colorName));
          strcpy(name, other.name);
          strcpy(color, other.color);
          strcpy(colorName, other.colorName);
          colorant = other.colorant;
          sclass = other.sclass;
          type = other.type;
          max_capacity = other.max_capacity;
          level = other.level;
      }
  }

  supplysData& operator=(const supplysData& other)
  {
      if(this != &other)
      {
          memset(name, 0, sizeof(name));
          memset(color, 0, sizeof(color));
          memset(colorName, 0, sizeof(colorName));
          strcpy(name, other.name);
          strcpy(color, other.color);
          strcpy(colorName, other.colorName);
          colorant = other.colorant;
          sclass = other.sclass;
          type = other.type;
          max_capacity = other.max_capacity;
          level = other.level;
      }

      return *this;
  }
} SUPPLYSDATA;

class cupssnmp
{
public:
    cupssnmp();
    ~cupssnmp();

public:
    void setIP(const string& strUri);
    void setPPDName(const string& strppd);
    bool SNMPReadSupplies();
    vector<SUPPLYSDATA> getMarkInfo();

private:
    void SNMPInit();
    void QuirksInit();
    bool SNMPOpen();
    bool SNMPSupport();
    void SNMPClose();
    void SNMPWalk();

private:
    string m_strip;
    string m_strPPD;
    int m_iFd;
    http_addrlist_t* m_pHost;
    bool m_bSNMPSupport;
};

#endif // CUPSSNMP_H
