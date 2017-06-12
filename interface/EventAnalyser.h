// <author>Cristóvão B da Cruz e Silva</author>
// <email>c.beirao@cern.ch</email>
// <date>2017-05-30</date>
// <summary>Header file for the Analyser class</summary>
//
// <description>
//  Header file with the declarations of the Analyser class and related classes.
//  The base analyser from which other analyses can be derived from is declared within.
// </description>

#ifndef MY_ANALYSER_H
#define MY_ANALYSER_H

#include <map>
#include <vector>
#include <string>
#include <exception>
#include <fstream>

#include "TTree.h"

#include "UserCode/Stop4Body/interface/ValueWithSystematics.h"

class AnalyserException: public std::exception
{
  public:
    AnalyserException(std::string mess): message(mess)
    {
    }

  private:
    std::string message;

    virtual const char* what() const throw()
    {
      return message.c_str();
    }

  protected:
};

class EventInfo
{
  public:
    EventInfo();
    EventInfo(const EventInfo&) = delete; // Delete copy constructor
    EventInfo(EventInfo&&) = delete;      // Delete move constructor
    void Reset();
    inline void Lock()
    {
      isLocked = true;
      for(auto & kv : eventDoubles)
        kv.second.Lock();
      for(auto & kv : eventFloats)
        kv.second.Lock();
      for(auto & kv : eventInts)
        kv.second.Lock();
      for(auto & kv : eventBools)
        kv.second.Lock();
    };
    #ifdef WITH_UNLOCK
    inline void Unlock()
    {
      isLocked = false;
      for(auto & kv : eventDoubles)
        kv.second.Unlock();
      for(auto & kv : eventFloats)
        kv.second.Unlock();
      for(auto & kv : eventInts)
        kv.second.Unlock();
      for(auto & kv : eventBools)
        kv.second.Unlock();
    };
    #endif

    ValueWithSystematics<double>& AddDouble(std::string name, double defaultVal);
    ValueWithSystematics<double>& GetDouble(std::string name);
    ValueWithSystematics<float>&  AddFloat (std::string name, float defaultVal);
    ValueWithSystematics<float>&  GetFloat (std::string name);
    ValueWithSystematics<int>&    AddInt   (std::string name, int defaultVal);
    ValueWithSystematics<int>&    GetInt   (std::string name);
    ValueWithSystematics<bool>&   AddBool  (std::string name, bool defaultVal);
    ValueWithSystematics<bool>&   GetBool  (std::string name);

    void OutputEventListHeader(std::ofstream& file, const std::vector<std::string>& priority = std::vector<std::string>(0)) const;
    void OutputEventList(std::ofstream& file, const std::vector<std::string>& priority = std::vector<std::string>(0)) const;
    void SetSummaryTreeBranches(TTree* const tree);

  private:
  protected:
    bool isLocked;
    std::map<std::string,ValueWithSystematics<double>> eventDoubles;
    std::map<std::string,ValueWithSystematics<float>>  eventFloats;
    std::map<std::string,ValueWithSystematics<int>>    eventInts;
    std::map<std::string,ValueWithSystematics<bool>>   eventBools;

    template<class T>
    void OutputValueListHeader(std::ofstream& file, const ValueWithSystematics<T>& val, const std::string& name) const;

    template<class T>
    void OutputValueList(std::ofstream& file, const ValueWithSystematics<T>& val) const;

    template<class T>
    void AddBranch(TTree* const tree, ValueWithSystematics<T>& val, std::string name);
};

#include "UserCode/Stop4Body/interface/EventAnalyser.hpp"

#endif
