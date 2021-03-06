//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "cmbProfileFunction.h"
#include "cmbManualProfileFunction.h"
#include "cmbProfileWedgeFunction.h"
#include <sstream>

void cmbProfileFunction::setName(std::string const& n)
{
  this->name = n;
}

std::string const& cmbProfileFunction::getName() const
{
  return name;
}

cmbProfileFunction* cmbProfileFunction::read(std::ifstream& in, size_t count)
{
  int version;
  in >> version;
  cmbProfileFunction* fun;
  if (version == 1)
  {
    fun = new cmbManualProfileFunction();
    std::stringstream ss;
    ss << count;
    std::string name;
    ss >> name;
    name = "Function_" + name;
    fun->setName(name);
  }
  else if (version == 2)
  {
    int type;
    in >> type;
    switch (type)
    {
      case MANUAL:
        fun = new cmbManualProfileFunction();
      case WEDGE:
        fun = new cmbProfileWedgeFunction();
    }
    std::string name;
    in >> name;
    fun->setName(name);
  }
  else
  {
    return NULL;
  }
  fun->readData(in, version);
  return fun;
}

bool cmbProfileFunction::write(std::ofstream& out) const
{
  out << 2 << " "
      << " " << getType() << '\n'
      << getName() << std::endl;
  return this->writeData(out);
}
