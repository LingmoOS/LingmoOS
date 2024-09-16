#ifndef _HW_H_
#define _HW_H_


#include <string>
#include <vector>
// #include "dlsdevicetypes.h"

using namespace std;


 namespace hw
{
  typedef enum
  {
    sys_tem,
    motherboard,
    bios,
    ddr,
    memory,
    processor,
    cpu_cache,
    address,
    storage,
    disk,
    tape,
    bus,  
    bridge,
    usb_hub,  //hub
    controller,
    network,
    display,
    input,
    printer,
    multimedia,
    media_camera,
    communication,
    power,
    volume,
    keyboard,
    mouse,
    monitor,
    generic
  } hwClass;

  typedef enum { none, iomem, ioport, mem, irq, dma }  hwResourceType;
  typedef enum { nil, boolean, integer, text }  hwValueType;

  string strip(const string &);
  string asString(long);

  string reportSize(unsigned long long);

  class resource
  {
    public:

      resource();
      ~resource();
      resource(const resource &);
      resource & operator =(const resource &);

      static resource iomem(unsigned long long, unsigned long long);
      static resource ioport(unsigned long, unsigned long);
      static resource mem(unsigned long long, unsigned long long, bool prefetchable = false);
      static resource irq(unsigned int);
      static resource dma(unsigned int);

      bool operator ==(const resource &) const;

      string asString(const string & separator = ":") const;

    private:
      struct resource_i * This;

  };

  class value
  {
    public:

      value();
      ~value();
      value(const value &);
      value(long long);
      value(const string &);
      value & operator =(const value &);

      bool operator ==(const value &) const;

      string asString() const;
      long long asInteger() const;
      bool defined() const;

    private:
      struct value_i * This;

  };

}                                                 // namespace hw

//主要的API接口在这，详见后面有标的 ，即为常用的 
class  hwNode
{
  public:
    hwNode(const string & id,
      hw::hwClass c = hw::generic,
      const string & vendor = "",
      const string & product = "",
      const string & version = "");
    hwNode(const hwNode & o);
    ~hwNode();
    hwNode & operator =(const hwNode & o);

    string getId() const;

    void setHandle(const string & handle);
    string getHandle() const;

    bool enabled() const;
    bool disabled() const;
    void enable();
    void disable();
    bool claimed() const;
    void claim(bool claimchildren=false);
    void unclaim();

    hw::hwClass getClass() const;  //获取设备类别
    const char * getClassName() const; //获取设备类别名
    void setClass(hw::hwClass c); 

    string getsubClassName() const;             //获取设备子类别名
    void setsubClassName(const string & classname); 

    string getDescription() const;         //获取设备相关描述
    void setDescription(const string & description);

    string getVendor() const;             //获取设备供应商
    void setVendor(const string & vendor);

    string getSubVendor() const;          //获取设备子供应商
    void setSubVendor(const string & subvendor);

    string getProduct() const;                  //获取设备
    void setProduct(const string & product);

    string getSubProduct() const;                //获取设备子设备
    void setSubProduct(const string & subproduct);


    string getVendor_name() const;                 //获取设备供应商名
    void setVendor_name(const string & vendor_name);

    string getSubVendor_name() const;               //获取设备子供应商名
    void setSubVendor_name(const string & subvendor_name);

    string getProduct_name() const;                //获取设备设备名
    void setProduct_name(const string & product_name);
 
    string getSubProduct_name() const;            //获取设备子设备名
    void setSubProduct_name(const string & subproduct_name);


    string getVendor_id() const;
    void setVendor_id(const string & vendor);

    string getProduct_id() const;
    void setProduct_id(const string & product);

    // string getSubVendor_id() const;
    // void setSubVendor_id(const string & subvendor);

    // string getSubProduct_id() const;
    // void setSubProduct_id(const string & subproduct);

    string getVersion() const;              //获取设备版本相关信息
    void setVersion(const string & version);

    string getDate() const;
    void setDate(const string &);

    string getSerial() const;
    void setSerial(const string & serial);

    unsigned long long getStart() const;
    void setStart(unsigned long long start);

    unsigned long long getSize() const;
    void setSize(unsigned long long size);

    unsigned long long getCapacity() const;
    void setCapacity(unsigned long long capacity);

    unsigned long long getClock() const;
    void setClock(unsigned long long clock);

    unsigned int getWidth() const;
    void setWidth(unsigned int width);

    string getSlot() const;
    void setSlot(const string & slot);

    string getModalias() const;                  //获取设备  Modalias  即硬件ID号
    void setModalias(const string & modalias);

    int countChildren(hw::hwClass c = hw::generic) const;
    hwNode * getChild(unsigned int);
    hwNode * getChildByPhysId(long);
    hwNode * getChildByPhysId(const string &);
    hwNode * getChild(const string & id);
    hwNode * findChildByHandle(const string & handle);
    hwNode * findChildByLogicalName(const string & handle);
    hwNode * findChildByBusInfo(const string & businfo);
    hwNode * findChildByResource(const hw::resource &);
    hwNode * findChild(bool(*matchfunction)(const hwNode &));
    hwNode * addChild(const hwNode & node);
    bool isBus() const
    {
      return countChildren()>0;
    }

    bool isCapable(const string & feature) const;
    void addCapability(const string & feature, const string & description = "");
    void describeCapability(const string & feature, const string & description);
    string getCapabilities() const;
    vector<string> getCapabilitiesList() const;
    string getCapabilityDescription(const string & feature) const;

    void attractHandle(const string & handle);

    void setConfig(const string & key, const string & value);
    void setConfig(const string & key, unsigned long long value);
    string getConfig(const string & key) const;      //获取设备 相关key值 如 "vid:pid"  "driver"  
    //cpu  "cpufreq" "cpufreq",
    //disk  "ReadSectors"  "WriteSectors"
    // 用法详见本文件末尾，或 print.cpp文件

    vector<string> getConfigKeys() const;
    vector<string> getConfigValues(const string & separator = "") const;

    vector<string> getLogicalNames() const;
    string getLogicalName() const;
    void setLogicalName(const string &);

    string getDev() const;
    void setDev(const string &);

    string getBusInfo() const;
    void setBusInfo(const string &);

    string getSysFS_Path() const;
    void setSysFS_Path(const string &);

    string getPhysId() const;
    void setPhysId(long);
    void setPhysId(unsigned, unsigned);
    void setPhysId(unsigned, unsigned, unsigned);
    void setPhysId(const string &);
    void assignPhysIds();

    void addResource(const hw::resource &);
    bool usesResource(const hw::resource &) const;
    vector<string> getResources(const string & separator = "") const;

    void addHint(const string &, const hw::value &);
    hw::value getHint(const string &) const;
    vector<string> getHints() const;

    void merge(const hwNode & node);

    void fixInconsistencies();

    string asString();

    bool dump(const string & filename, bool recurse = true);
  private:

    void setId(const string & id);

    bool attractsHandle(const string & handle) const;
    bool attractsNode(const hwNode & node) const;

    struct hwNode_i * This;
};
/*

// getConfig("driver")
// getConfig("cpufreq")
// setConfig("driver", shortname(drivername));
//getConfig("cpufreq", cur);
getConfig("vid:pid")

disk getConfig("ReadSectors",curRead);
disk getConfig("WriteSectors",curWrite);
*/


#endif
