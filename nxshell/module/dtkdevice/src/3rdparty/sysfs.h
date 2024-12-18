#ifndef _SYSFS_H_
#define _SYSFS_H_

#include <string>
#include "hw.h"

using namespace std;

namespace sysfs
{

  class entry
  {
    public:

      static entry byBus(string devbus, string devname);
      static entry byClass(string devclass, string devname);
      static entry byPath(string path);

      entry & operator =(const entry &);
      entry(const entry &);
      ~entry();

      entry leaf() const;

      bool hassubdir(const string &) const;
      string name() const;
      string classname() const;
      string subsystem() const;
      bool isvirtual() const;
      string businfo() const;
      string driver() const;
      string modalias() const;
      string device() const;
      string vendor() const;
      string sysfs_path() const;
      entry parent() const;
      string name_in_class(const string &) const;
      string string_attr(const string & name, const string & def = "") const;
      unsigned long long hex_attr(const string & name, unsigned long long def = 0) const;
      vector < string > multiline_attr(const string & name) const;

      vector < entry > devices() const;

      struct entry_i * This;

    private:
      entry(const string &);

  };

  vector < entry > entries_by_bus(const string & busname);
  vector < entry > entries_by_class(const string & classname);

}                                                 // namespace sysfs


bool scan_sysfs(hwNode & n);

std::string sysfs_finoceanvice(const string &name);
#endif
