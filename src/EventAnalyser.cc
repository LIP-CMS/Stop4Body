// <author>Cristóvão B da Cruz e Silva</author>
// <email>c.beirao@cern.ch</email>
// <date>2017-05-30</date>
// <summary>Implementation file for the Analyser class</summary>
//
// <description>
//  Implementation file with the definitions of the Analyser class and related classes.
//  The base analyser from which other analyses can be derived from is defined within.
// </description>

#include "UserCode/Stop4Body/interface/EventAnalyser.h"

EventInfo::EventInfo(): isLocked(false)
{
}

void EventInfo::Reset()
{
  if(isLocked)
  {
    for(auto& kv : eventDoubles)
      kv.second.Reset();
    for(auto& kv : eventFloats)
      kv.second.Reset();
    for(auto& kv : eventInts)
      kv.second.Reset();
    for(auto& kv : eventBools)
      kv.second.Reset();
  }
  else
  {
    eventDoubles.clear();
    eventFloats.clear();
    eventInts.clear();
    eventBools.clear();
  }
}

ValueWithSystematics<double>& EventInfo::GetDouble(std::string name)
{
  if(eventDoubles.count(name) == 0)
    throw AnalyserException("Tried to access non-existing double: "+name);
  return eventDoubles.at(name);
}

ValueWithSystematics<float>& EventInfo::GetFloat(std::string name)
{
  if(eventFloats.count(name) == 0)
    throw AnalyserException("Tried to access non-existing float: "+name);
  return eventFloats.at(name);
}

ValueWithSystematics<int>&    EventInfo::GetInt   (std::string name)
{
  if(eventInts.count(name) == 0)
    throw AnalyserException("Tried to access non-existing int: "+name);
  return eventInts.at(name);
}

ValueWithSystematics<bool>&   EventInfo::GetBool  (std::string name)
{
  if(eventBools.count(name) == 0)
    throw AnalyserException("Tried to access non-existing boolean: "+name);
  return eventBools.at(name);
}

ValueWithSystematics<double>& EventInfo::AddDouble(std::string name, double defaultVal = 0.0)
{
  if(eventDoubles.count(name) == 0)
  {
    if(isLocked)
      throw AnalyserException("Tried to add a double to the contents after locking the event content");
    eventDoubles[name] = ValueWithSystematics<double>(defaultVal);
    eventDoubles[name].DefaultValue() = defaultVal;
  }
  else
    std::cout << "The double " << name << " already exists. No action taken." << std::endl;

  return eventDoubles.at(name);
}

ValueWithSystematics<float>& EventInfo::AddFloat(std::string name, float defaultVal = 0.0)
{
  if(eventFloats.count(name) == 0)
  {
    if(isLocked)
      throw AnalyserException("Tried to add a float to the contents after locking the event content");
    eventFloats[name] = ValueWithSystematics<float>(defaultVal);
    eventFloats[name].DefaultValue() = defaultVal;
  }
  else
    std::cout << "The float " << name << " already exists. No action taken." << std::endl;

  return eventFloats.at(name);
}

ValueWithSystematics<int>&    EventInfo::AddInt   (std::string name, int defaultVal = 0)
{
  if(eventInts.count(name) == 0)
  {
    if(isLocked)
      throw AnalyserException("Tried to add an integer to the contents after locking the event content");
    eventInts[name] = ValueWithSystematics<int>(defaultVal);
    eventInts[name].DefaultValue() = defaultVal;
  }
  else
    std::cout << "The integer " << name << " already exists. No action taken." << std::endl;

  return eventInts.at(name);
}

ValueWithSystematics<bool>&   EventInfo::AddBool  (std::string name, bool defaultVal = false)
{
  if(eventBools.count(name) == 0)
  {
    if(isLocked)
      throw AnalyserException("Tried to add a bool to the contents after locking the event content");
    eventBools[name] = ValueWithSystematics<bool>(defaultVal);
    eventBools[name].DefaultValue() = defaultVal;
  }
  else
    std::cout << "The boolean " << name << " already exists. No action taken." << std::endl;

  return eventBools.at(name);
}

void EventInfo::SetSummaryTreeBranches(TTree* const tree)
{
  for(auto& kv: eventDoubles)
    AddBranch(tree, kv.second, "d_"+kv.first);
  for(auto& kv: eventFloats)
    AddBranch(tree, kv.second, "f_"+kv.first);
  for(auto& kv: eventInts)
    AddBranch(tree, kv.second, "i_"+kv.first);
  for(auto& kv: eventBools)
    AddBranch(tree, kv.second, "b_"+kv.first);

  return;
}

// TODO: make an option for the event list to be outputted as a tsv
void EventInfo::OutputEventListHeader(std::ofstream& file, const std::vector<std::string>& priority) const
{
  for(auto& entry: priority)
  {
    if(eventDoubles.count(entry) != 0)
      OutputValueListHeader(file, eventDoubles.at(entry), entry);
    if(eventFloats.count(entry) != 0)
      OutputValueListHeader(file, eventFloats.at(entry), entry);
    if(eventInts.count(entry) != 0)
      OutputValueListHeader(file, eventInts.at(entry), entry);
    if(eventBools.count(entry) != 0)
      OutputValueListHeader(file, eventBools.at(entry), entry);
  }

  for(auto& kv: eventDoubles)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueListHeader(file, kv.second, kv.first);
  for(auto& kv: eventFloats)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueListHeader(file, kv.second, kv.first);
  for(auto& kv: eventInts)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueListHeader(file, kv.second, kv.first);
  for(auto& kv: eventBools)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueListHeader(file, kv.second, kv.first);

  file << "\n";
  return;
}

void EventInfo::OutputEventList(std::ofstream& file, const std::vector<std::string>& priority) const
{
  for(auto& entry: priority)
  {
    if(eventDoubles.count(entry) != 0)
      OutputValueList(file, eventDoubles.at(entry));
    if(eventFloats.count(entry) != 0)
      OutputValueList(file, eventFloats.at(entry));
    if(eventInts.count(entry) != 0)
      OutputValueList(file, eventInts.at(entry));
    if(eventBools.count(entry) != 0)
      OutputValueList(file, eventBools.at(entry));
  }

  for(auto& kv: eventDoubles)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueList(file, kv.second);
  for(auto& kv: eventFloats)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueList(file, kv.second);
  for(auto& kv: eventInts)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueList(file, kv.second);
  for(auto& kv: eventBools)
    if (std::find(priority.begin(), priority.end(), kv.first) == priority.end())
      OutputValueList(file, kv.second);

  file << "\n";
  return;
}

