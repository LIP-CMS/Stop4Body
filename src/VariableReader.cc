#include "UserCode/Stop4Body/interface/VariableReader.h"
#include "UserCode/Stop4Body/interface/SampleReader.h"

#include <iostream>
#include <fstream>
#include <algorithm>

using json = nlohmann::json;

VariableInfo::VariableInfo(json jsonInfo)
{
  if(jsonInfo.count("expression") == 0)
    throw MissingJSONParam("The variable does not have an expression. It is not possible to draw without an expression.");
  expression_ = jsonInfo["expression"];

  name_ = expression_;
  label_ = expression_;
  if(jsonInfo.count("name") != 0)
    name_ = jsonInfo["name"];
  if(jsonInfo.count("label") != 0)
    label_ = jsonInfo["label"];

  if(jsonInfo.count("max") == 0)
    throw MissingJSONParam("The variable '" + name_ + "' does not have a maximum value. It is not possible to draw without a maximum value.");
  max_ = jsonInfo["max"];

  min_ = 0;
  if(jsonInfo.count("min") != 0)
    min_ = jsonInfo["min"];

  bins_ = 20;
  if(jsonInfo.count("bins") != 0)
    bins_ = jsonInfo["bins"];

  legPos_ = "top";
  if(jsonInfo.count("legPos") != 0)
  {
    legPos_ = jsonInfo["legPos"];
    std::transform(legPos_.begin(), legPos_.end(), legPos_.begin(), ::tolower);
  }
}

VariableJsonLoader::VariableJsonLoader(std::string file): inputFile_(file)
{
  json jsonFile;
  std::ifstream inputFile(inputFile_);
  inputFile >> jsonFile;

  if(jsonFile.count("variables") == 0)
    throw MissingJSONParam("The JSON file does not contain the 'variables' entry. It is not a valid file.");

  for(auto& var : jsonFile["variables"])
  {
    try
    {
      VariableInfo thisVariable(var);
      variables_.push_back(thisVariable);
    }
    catch(MissingJSONParam& exception)
    {
      std::cout << "Incomplete variable found, skipping it." << std::endl;
      std::cout << "The message was: " << exception.what() << std::endl;
    }
  }
}

TwoDVariableJsonLoader::TwoDVariableJsonLoader(std::string file): inputFile_(file)
{
  json jsonFile;
  std::ifstream inputFile(inputFile_);
  inputFile >> jsonFile;

  if(jsonFile.count("2Dplots") == 0)
    throw MissingJSONParam("The JSON file does not contain the '2Dplots' entry. It is not a valid file.");

  for(auto& var : jsonFile["2Dplots"])
  {
    try
    {
      if(var.count("name") == 0)
        throw MissingJSONParam("The 2Dplot does not have a name.");
      std::string name = var["name"];

      if(var.count("X") == 0)
        throw MissingJSONParam("The 2Dplot '" + name + "' does not have an X variable.");
      if(var.count("Y") == 0)
        throw MissingJSONParam("The 2Dplot '" + name + "' does not have a Y variable.");

      TwoDVariableInfo thisPlot(name, var["X"], var["Y"]);
      variables_.push_back(thisPlot);
    }
    catch(MissingJSONParam& exception)
    {
      std::cout << "Incomplete variable found, skipping it." << std::endl;
      std::cout << "The message was: " << exception.what() << std::endl;
    }
  }
}
