//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBStreamTracerPanel.h"

#include "vtkClientServerStream.h"
#include "vtkProcessModule.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMProxy.h"
#include "vtkSMSession.h"
#include <QDoubleValidator>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

pqCMBStreamTracerPanel::pqCMBStreamTracerPanel(vtkSMProxy* pxy, vtkSMPropertyGroup*, QWidget* p)
  : pqPropertyWidget(pxy, p)
{
  QObject::connect(this->findChild<QLineEdit*>("NumberOfTestLocations"), SIGNAL(editingFinished()),
    this, SLOT(updateTestLocationsUI()));

  this->TestLocationsFrame = NULL;
  this->NumberOfTestLocations = 0;
  this->updateTestLocationsUI();
}

void pqCMBStreamTracerPanel::updateTestLocationsUI()
{
  int number = this->findChild<QLineEdit*>("NumberOfTestLocations")->text().toInt();
  if (this->NumberOfTestLocations == number)
  {
    return;
  }
  this->NumberOfTestLocations = number;
  vtkSMIntVectorProperty* numproperty =
    vtkSMIntVectorProperty::SafeDownCast(this->proxy()->GetProperty("NumberOfTestLocations"));
  numproperty->SetElement(0, number);
  this->proxy()->UpdateProperty("NumberOfTestLocations", true);

  if (this->TestLocationsFrame)
  {
    this->layout()->removeWidget(this->TestLocationsFrame);
    delete this->TestLocationsFrame;
  }
  QFrame* frame = this->TestLocationsFrame = new QFrame(this);
  QGridLayout* testLayout = new QGridLayout(frame);
  QGridLayout* panelLayout = qobject_cast<QGridLayout*>(this->layout());
  if (panelLayout)
  {
    panelLayout->addWidget(frame, panelLayout->rowCount() - 1, 0, 1, panelLayout->columnCount());
  }
  else
  {
    this->layout()->addWidget(frame);
  }

  testLayout->addWidget(new QLabel("Test Location", frame), 0, 0);
  testLayout->addWidget(new QLabel("Offset-X", frame), 0, 1);
  testLayout->addWidget(new QLabel("Offset-Y", frame), 0, 2);
  testLayout->addWidget(new QLabel("Offset-Z", frame), 0, 3);

  QDoubleValidator* validator = new QDoubleValidator(frame);
  for (int i = 0; i < number; i++)
  {
    testLayout->addWidget(new QLabel(QString::number(i + 1), frame), i + 1, 0);
    QString boxName("location");
    boxName.append(QString::number(i));
    for (int j = 0; j < 3; j++)
    {
      QLineEdit* editbox = new QLineEdit(frame);
      boxName.append(QString::number(j));
      editbox->setObjectName(boxName);
      editbox->setValidator(validator);
      editbox->setText("0.0");
      testLayout->addWidget(editbox, i + 1, j + 1);
      QObject::connect(editbox, SIGNAL(editingFinished()), this, SLOT(onTestLocationChanged()));
    }
  }
}

void pqCMBStreamTracerPanel::accept()
{
  // set the test location changes
  if (this->TestLocationsFrame)
  {
    int number = this->findChild<QLineEdit*>("NumberOfTestLocations")->text().toInt();
    QGridLayout* testLayout = static_cast<QGridLayout*>(this->TestLocationsFrame->layout());
    for (int i = 0; i < number; i++)
    {
      double offset[3];
      for (int j = 0; j < 3; j++)
      {
        QLineEdit* editbox =
          static_cast<QLineEdit*>(testLayout->itemAtPosition(i + 1, j + 1)->widget());
        offset[j] = editbox->text().toDouble();
      }
      this->SendDouble3Vector("SetRelativeOffsetOfTestLocation", i, offset);
    }
  }
}

void pqCMBStreamTracerPanel::onTestLocationChanged()
{
  emit this->changeAvailable();
}

void pqCMBStreamTracerPanel::SendDouble3Vector(const char* func, int index, double* data)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this->proxy()) << func << index << data[0]
         << data[1] << data[2] << vtkClientServerStream::End;
  this->proxy()->GetSession()->ExecuteStream(this->proxy()->GetLocation(), stream);
  this->proxy()->MarkModified(NULL);
}
