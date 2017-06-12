// <author>Cristóvão B da Cruz e Silva</author>
// <email>c.beirao@cern.ch</email>
// <date>2017-05-30</date>
// <summary>Implementation file for the templates from the Analyser class</summary>
//
// <description>
//  Implementation file with the definitions of the Analyser class and related classes.
// </description>

#ifndef MY_ANALYSER_HPP
#define MY_ANALYSER_HPP

#define DEFAULT_WIDTH 15

template<class T>
void EventInfo::OutputValueListHeader(std::ofstream& file, const ValueWithSystematics<T>& val, const std::string& name) const
{
  std::string metadata = val.GetMetadata("eventlist");
  if(metadata == "true")
  {
    const std::string widthStr = val.GetMetadata("eventlistWidth");
    int width = DEFAULT_WIDTH;
    if(widthStr != "")
    {
      std::stringstream tmp;
      tmp << widthStr;
      tmp >> width;
      if(width <= 0)
        width = DEFAULT_WIDTH;
    }

    file << std::setw(width) << name << " | ";
  }

  return;
}

template<class T>
void EventInfo::OutputValueList(std::ofstream& file, const ValueWithSystematics<T>& val) const
{
  std::string metadata = val.GetMetadata("eventlist");
  if(metadata == "true")
  {
    const std::string widthStr = val.GetMetadata("eventlistWidth");
    int width = DEFAULT_WIDTH;
    if(widthStr != "")
    {
      std::stringstream tmp;
      tmp << widthStr;
      tmp >> width;
      if(width == 0)
        width = DEFAULT_WIDTH;
    }

    file << std::setw(width) << val.Value() << " | ";
  }

  return;
}

template<>
void EventInfo::OutputValueList(std::ofstream& file, const ValueWithSystematics<bool>& val) const
{
  std::string metadata = val.GetMetadata("eventlist");
  if(metadata == "true")
  {
    const std::string widthStr = val.GetMetadata("eventlistWidth");
    int width = DEFAULT_WIDTH;
    if(widthStr != "")
    {
      std::stringstream tmp;
      tmp << widthStr;
      tmp >> width;
      if(width == 0)
        width = DEFAULT_WIDTH;
    }

    file << std::setw(width) << (val.Value()?("True"):("False")) << " | ";
  }

  return;
}

template<class T>
void EventInfo::AddBranch(TTree* const tree, ValueWithSystematics<T>& val, std::string name)
{
  std::string metadata = val.GetMetadata("eventtree");
  if(metadata == "true" || metadata == "")
  {
    tree->Branch(name.c_str(), &(val.Value()));

    for(auto& syst: val.Systematics())
    {
      tree->Branch((name + "_" + syst).c_str(), &(val.Systematic(syst)));
    }
  }

  return;
}

#endif
