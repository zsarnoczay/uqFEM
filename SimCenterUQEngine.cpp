// Written: fmckenna

/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written: fmckenna

#include "SimCenterUQEngine.h"
#include <SimCenterUQResultsSampling.h>

#include <QStackedWidget>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QLabel>

#include <QDebug>

#include <SimCenterUQInputSampling.h>
//#include <SimCenterUQInputReliability.h>
#include <SimCenterUQInputSensitivity.h>
//#include <SimCenterUQInputCalibration.h>
//#include <SimCenterUQInputBayesianCalibration.h>

#include "InputWidgetEDP.h"

SimCenterUQEngine::SimCenterUQEngine(InputWidgetEDP *edpWidget, QWidget *parent)
: UQ_Engine(parent), theCurrentEngine(0), theEdpWidget(edpWidget)
{

    QVBoxLayout *layout = new QVBoxLayout();

    //
    // the selection part
    //

    QHBoxLayout *theSelectionLayout = new QHBoxLayout();
    QLabel *label = new QLabel();
    label->setText(QString("SimCenterUQ Method Catagory"));
    theEngineSelectionBox = new QComboBox();
    //theEngineSelectionBox->addItem(tr("Forward Propagation"));
    //theEngineSelectionBox->addItem(tr("Parameters Estimation"));
    //theEngineSelectionBox->addItem(tr("Inverse Problem"));
    //theEngineSelectionBox->addItem(tr("Reliability Analysis"));
    theEngineSelectionBox->addItem(tr("Sensitivity Analysis"));
    theEngineSelectionBox->setMinimumWidth(600);

    theSelectionLayout->addWidget(label);
    theSelectionLayout->addWidget(theEngineSelectionBox);
    theSelectionLayout->addStretch();
    layout->addLayout(theSelectionLayout);

    //
    // create the stacked widget
    //

    theStackedWidget = new QStackedWidget();

    //
    // create the individual widgets add to stacked widget
    //

    theSamplingEngine = new SimCenterUQInputSampling();
    //theReliabilityEngine = new SimCenterUQInputReliability();
    //theCalibrationEngine = new SimCenterUQInputCalibration();
    //theBayesianCalibrationEngine = new SimCenterUQInputBayesianCalibration();
    theSensitivityEngine = new SimCenterUQInputSensitivity();

    //theStackedWidget->addWidget(theSamplingEngine);
    //theStackedWidget->addWidget(theCalibrationEngine);
    //theStackedWidget->addWidget(theBayesianCalibrationEngine);
    //theStackedWidget->addWidget(theReliabilityEngine);
    theStackedWidget->addWidget(theSensitivityEngine);

    layout->addWidget(theStackedWidget);
    this->setLayout(layout);
    theCurrentEngine=theSensitivityEngine;

    connect(theEngineSelectionBox, SIGNAL(currentIndexChanged(QString)), this,
          SLOT(engineSelectionChanged(QString)));

    connect(theSamplingEngine, SIGNAL(onNumModelsChanged(int)), this, SLOT(numModelsChanged(int)));

}

SimCenterUQEngine::~SimCenterUQEngine()
{

}


void SimCenterUQEngine::engineSelectionChanged(const QString &arg1)
{
    UQ_Engine *theOldEngine = theCurrentEngine;

    if ((arg1 == QString("Sampling")) || (arg1 == QString("Forward Propagation"))) {
      //theStackedWidget->setCurrentIndex(0);
      //theCurrentEngine = theSamplingEngine;
    } else if ((arg1 == QString("Calibration"))
               || (arg1 == QString("Parameters Estimation"))
               || (arg1 == QString("Parameter Estimation"))) {

      //theStackedWidget->setCurrentIndex(1);
      //theCurrentEngine = theCalibrationEngine;
    } else if ((arg1 == QString("Bayesian Calibration")) || (arg1 == QString("Inverse Problem"))) {
      //theStackedWidget->setCurrentIndex(2);
      //theCurrentEngine = theBayesianCalibrationEngine;
    } else if ((arg1 == QString("Reliability")) || (arg1 == QString("Reliability Analysis"))) {
      //theStackedWidget->setCurrentIndex(3);
      //theCurrentEngine = theReliabilityEngine;
    } else if ((arg1 == QString("Sensitivity")) || (arg1 == QString("Sensitivity Analysis"))) {
      theStackedWidget->setCurrentIndex(0);
      theCurrentEngine = theSensitivityEngine;
    } else {
      qDebug() << "ERROR .. SimCenterUQEngine selection .. type unknown: " << arg1;
    }

    // emit signal if engine changed
    if (theCurrentEngine != theOldEngine)
        emit onUQ_EngineChanged();
}


int
SimCenterUQEngine::getMaxNumParallelTasks(void) {
    return theCurrentEngine->getMaxNumParallelTasks();
}

bool
SimCenterUQEngine::outputToJSON(QJsonObject &jsonObject) {

    jsonObject["uqType"] = theEngineSelectionBox->currentText();
    return theCurrentEngine->outputToJSON(jsonObject);
}

bool
SimCenterUQEngine::inputFromJSON(QJsonObject &jsonObject) {
    bool result = false;

    QString selection = jsonObject["uqType"].toString();

    int index = theEngineSelectionBox->findText(selection);
    theEngineSelectionBox->setCurrentIndex(index);
    this->engineSelectionChanged(selection);
    if (theCurrentEngine != 0)
        result = theCurrentEngine->inputFromJSON(jsonObject);
    else
        result = false; // don't emit error as one should have been generated

    return result;
}


bool
SimCenterUQEngine::outputAppDataToJSON(QJsonObject &jsonObject)
{
    jsonObject["Application"] = "SimCenterUQ-UQ";
    QJsonObject dataObj;
    jsonObject["ApplicationData"] = dataObj;

    return true;
}

bool
SimCenterUQEngine::inputAppDataFromJSON(QJsonObject &jsonObject)
{
    Q_UNUSED(jsonObject);
    return true;
}

int
SimCenterUQEngine::processResults(QString &filenameResults, QString &filenameTab) {
    return theCurrentEngine->processResults(filenameResults, filenameTab);
}

RandomVariablesContainer *
SimCenterUQEngine::getParameters() {
       return theCurrentEngine->getParameters();
}

UQ_Results *
SimCenterUQEngine::getResults(void) {
    return theCurrentEngine->getResults();
}

QString
SimCenterUQEngine::getProcessingScript() {
    return QString("parseSimCenterUQ.py");
}

void
SimCenterUQEngine::numModelsChanged(int newNum) {
    emit onNumModelsChanged(newNum);
}
