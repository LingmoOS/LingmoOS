/*
 * mem.cpp
 *
 * This scan tries to guess the size of system memory by looking at several
 * sources:
 * - the size of /proc/kcore
 * - the value returned by the sysconf libc function
 * - the sum of sizes of kernel hotplug memory blocks
 * - the sum of sizes of individual memory banks
 *
 * NOTE: In the first two cases this guess can be widely inaccurate, as the
 * kernel itself limits the memory addressable by userspace processes.
 * Because of that, this module reports the biggest value found if it can't
 * access the size of individual memory banks (information filled in by other
 * scans like DMI (on PCs) or OpenFirmare device-tree (on Macs).
 *
 */


#include "config.h"
#include "sysfs.h"
#include "mem.h"
#include "osutils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// // MemTotal:        8068476 kB
// // MemFree:          159580 kB
// // MemAvailable:    2303208 kB    hw::strip(

vector < string > meminfo_lines;

static bool get_procMeminfoFile(hwNode *memory)
{


  vector < string > meminfo_lines;
  int MemTotal;
  int MemFree;
  int MemAvailable;
  double memUsage = 0;

  if (memory)
  {

      if (loadfile("/proc/meminfo", meminfo_lines))
        for (unsigned int i = 0; i < 3; i++)
        {
          vector < string > variable;
          vector < string > var_unit;
          string stmp;

          splitlines(meminfo_lines[i], variable, ':');
          stmp = hw::strip(variable[1]);
          splitlines(stmp, var_unit, ' ');


          if ((variable[0] == "MemTotal") && (variable.size() == 2))
          {
            memory->setConfig("MemTotal",hw::strip(variable[1]));
            MemTotal = stoi(var_unit[0]);  // // memory->addHint("MemTotal", var_unit[0]);      
          }

          if ((variable[0] == "MemFree") && (variable.size() == 2))
          {
            memory->setConfig("MemFree",hw::strip(variable[1]));
            MemFree = stoi(var_unit[0]);  // memory->addHint("MemFree", var_unit[0]);
          }
          if ((variable[0] == "MemAvailable") && (variable.size() == 2))
          {
            memory->setConfig("MemAvailable",hw::strip(variable[1]));
            MemAvailable = stoi(var_unit[0]);  // memory->addHint("MemAvailable", var_unit[0]);        
          }         

        }

        // 为返回值赋值，计算内存占用率
        
        if(MemTotal  && (MemTotal > MemAvailable))
        {
            memUsage = (MemTotal - MemAvailable) * 100.0 / MemTotal;
            memory->setConfig("MemUsage",(to_string((int)memUsage)+ " %"));            
        }

    } //if (memory)
  return true;
}


static unsigned long long get_kcore_size()
{
  struct stat buf;

  if (stat("/proc/kcore", &buf) != 0)
    return 0;
  else
    return buf.st_size;
}


static unsigned long long get_sysconf_size()
{
  long pagesize = 0;
  long physpages = 0;
  unsigned long long logicalmem = 0;

  pagesize = sysconf(_SC_PAGESIZE);
  physpages = sysconf(_SC_PHYS_PAGES);
  if ((pagesize > 0) && (physpages > 0))
    logicalmem =
      (unsigned long long) pagesize *(unsigned long long) physpages;

  return logicalmem;
}


static unsigned long long get_hotplug_size()
{
  vector < sysfs::entry > entries = sysfs::entries_by_bus("memory");

  if (entries.empty())
    return 0;

  unsigned long long memblocksize =
    sysfs::entry::byPath("/system/memory").hex_attr("block_size_bytes");
  if (memblocksize == 0)
    return 0;

  unsigned long long totalsize = 0;
  for (vector < sysfs::entry >::iterator it = entries.begin();
      it != entries.end(); ++it)
  {
    const sysfs::entry & e = *it;
    if (e.string_attr("online") != "1")
      continue;
    totalsize += memblocksize;
  }
  return totalsize;
}


static unsigned long long count_memorybanks_size(hwNode & n)
{
  hwNode *memory = n.getChild("motherboard/ddr");

  if (memory)
  {
    unsigned long long size = 0;

    memory->claim(true);                          // claim memory and all its children

    for (unsigned int i = 0; i < memory->countChildren(); i++)
      if (memory->getChild(i)->getClass() == hw::memory)
        size += memory->getChild(i)->getSize();

    memory->setSize(size);
    return size;
  }
  else
    return 0;
}


static void claim_memory(hwNode & n)
{
  hwNode *core = n.getChild("motherboard");

  if (core)
  {
    for (unsigned int i = 0; i < core->countChildren(); i++)
      if (core->getChild(i)->getClass() == hw::memory)
        if(core->getChild(i)->claimed())
          core->getChild(i)->claim(true);         // claim memory and all its children
  }
}


bool scan_memory(hwNode & n)
{
  hwNode *memory = n.getChild("motherboard/ddr");
  unsigned long long logicalmem = 0;
  unsigned long long kcore = 0;
  unsigned long long hotplug_size = 0;

  logicalmem = get_sysconf_size();
  kcore = get_kcore_size();
  hotplug_size = get_hotplug_size();
  count_memorybanks_size(n);
  claim_memory(n);

  if (!memory)
  {
    hwNode *core = n.getChild("motherboard");

    if (!core)
    {
      n.addChild(hwNode("motherboard", hw::ddr));
      core = n.getChild("motherboard");
    }

    if (core)
    {
      core->addChild(hwNode("ddr", hw::ddr));
      memory = core->getChild("ddr");
    }
  }

  if (memory)
  {
    memory->claim();
    memory->addHint("icon", string("memory"));

    if (memory->getDescription() == "")
      memory->setDescription(_("ddr memory"));

    if (memory->getSize() > logicalmem)           // we already have a value
      return true;

    if (hotplug_size > logicalmem)
      memory->setSize(hotplug_size);
    else if ((logicalmem == 0)
      || ((kcore > logicalmem) && (kcore < 2 * logicalmem)))
      memory->setSize(kcore);
    else
      memory->setSize(logicalmem);

    get_procMeminfoFile(memory);

    return true;
  }

  return false;
}
