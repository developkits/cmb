//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkPVSceneGenFileInformation.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkClientServerStream.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkStringReader.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkPVSceneGenFileInformation);

vtkPVSceneGenFileInformation::vtkPVSceneGenFileInformation()
{
}

vtkPVSceneGenFileInformation::~vtkPVSceneGenFileInformation()
{
}

void vtkPVSceneGenFileInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << endl;
}

void vtkPVSceneGenFileInformation::CopyFromObject(vtkObject* obj)
{
  vtkStringReader* dataObject = vtkStringReader::SafeDownCast(obj);

  if (!dataObject)
  {
    vtkErrorMacro("Object is not a String Reader!");
    return;
  }

  this->FileContents = dataObject->GetFileContents();
  this->FileName = dataObject->GetFileName();
}

void vtkPVSceneGenFileInformation::AddInformation(vtkPVInformation* /*info*/)
{
}

void vtkPVSceneGenFileInformation::CopyToStream(vtkClientServerStream* css)
{
  css->Reset();
  *css << vtkClientServerStream::Reply;
  //*css << this->FileName.length() << this->FileName.c_str()
  //     << this->FileContents.length() << this->FileContents.c_str()
  *css << this->FileName.c_str() << this->FileContents.c_str() << vtkClientServerStream::End;
}

void vtkPVSceneGenFileInformation::CopyFromStream(const vtkClientServerStream* css)
{
  /*
  unsigned int size;
  css->GetArgument(0, 0, &size);
  char *buffer = new char[size];
  css->GetArgument(0, 1, buffer, size);
  this->FileName = buffer;
  delete [] buffer;

  css->GetArgument(0, 2, &size);
  buffer = new char[size];
  css->GetArgument(0, 3, buffer, size);
  this->FileContents = buffer;
  delete [] buffer;
  */
  const char* sres;
  int retVal = css->GetArgument(0, 0, &sres);
  this->FileName = sres;
  retVal = css->GetArgument(0, 1, &sres);
  this->FileContents = sres;
}
