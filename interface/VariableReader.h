#ifndef MY_VARIABLE_READER_H
#define MY_VARIABLE_READER_H

#include <string>
#include <vector>

#include "UserCode/Stop4Body/interface/json.hpp"

class VariableInfo
{
public:
  VariableInfo(std::string exp, std::string name, std::string label, double min, double max, int bins):
  expression_(exp),
  name_(name),
  label_(label),
  min_(min),
  max_(max),
  bins_(bins)
  {}
  VariableInfo(nlohmann::json jsonInfo);

  std::string expression() const {return expression_;}
  std::string name() const {return name_;}
  std::string label() const {return label_;}
  double min() const {return min_;}
  double max() const {return max_;}
  int bins() const {return bins_;}

private:
protected:
  std::string expression_;
  std::string name_;
  std::string label_;
  double min_;
  double max_;
  int bins_;

};

class VariableJsonLoader
{
public:
  VariableJsonLoader(std::string);

  // Iteration
  typedef typename std::vector<VariableInfo>::iterator iterator;
  typedef typename std::vector<VariableInfo>::const_iterator const_iterator;

  iterator begin() {return variables_.begin();}
  const_iterator begin() const {return variables_.begin();}
  const_iterator cbegin() const {return variables_.cbegin();}
  iterator end() {return variables_.end();}
  const_iterator end() const {return variables_.end();}
  const_iterator cend() const {return variables_.cend();}

private:
protected:
  std::string inputFile_;
  std::vector<VariableInfo> variables_;
};

#endif
