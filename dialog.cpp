/**
 * @licence app begin@
 * Copyright (C) 2021 Alexander Wenzel
 *
 * This file is part of the DLT Relais project.
 *
 * \copyright This code is licensed under GPLv3.
 *
 * \author Alexander Wenzel <alex@eli2.de>
 *
 * \file dialog.cpp
 * @licence end@
 */

#include <QSerialPortInfo>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

#include "dialog.h"
#include "ui_dialog.h"
#include "settingsdialog.h"
#include "version.h"

Dialog::Dialog(bool autostart,QString configuration,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , dltTestRobot(this)
{
    ui->setupUi(this);

    // clear settings
    on_pushButtonDefaultSettings_clicked();

    // set window title with version information
    setWindowTitle(QString("DLTTestRobot %1").arg(DLT_TEST_ROBOT_VERSION));
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    // disable stop button at startup
    ui->pushButtonStop->setDisabled(true);

    // connect status slots
    connect(&dltTestRobot, SIGNAL(status(QString)), this, SLOT(statusTestRobot(QString)));
    connect(&dltMiniServer, SIGNAL(status(QString)), this, SLOT(statusDlt(QString)));

    connect(&dltTestRobot, SIGNAL(command(int,int,int,int,int,int,int,QString)), this, SLOT(command(int,int,int,int,int,int,int,QString)));

    //  load global settings from registry
    QSettings settings;
    QString filename = settings.value("autoload/filename").toString();
    bool autoload = settings.value("autoload/checked").toBool();
    bool autostartGlobal = settings.value("autostart/checked").toBool();
    bool autoloadTests = settings.value("autoloadTests/checked").toBool();
    QString filenameTests = settings.value("autoloadTests/filename").toString();

    // autoload settings, when activated in global settings
    if(autoload)
    {
        dltTestRobot.readSettings(filename);
        dltMiniServer.readSettings(filename);
        restoreSettings();
    }

    // autoload tests, when activated in global settings
    if(autoloadTests)
    {
        ui->checkBoxAutoloadTests->setChecked(true);
        loadTests(filenameTests);
    }

    // autoload settings, when provided by command line
    if(!configuration.isEmpty())
    {
        dltTestRobot.readSettings(configuration);
        dltMiniServer.readSettings(configuration);
        restoreSettings();
    }

    // autostart, when activated in global settings or by command line
    if(autostartGlobal || autostart)
    {
        on_pushButtonStart_clicked();
    }
}

Dialog::~Dialog()
{

    // disconnect all slots
    disconnect(&dltTestRobot, SIGNAL(status(QString)), this, SLOT(statusTestRobot(QString)));
    disconnect(&dltMiniServer, SIGNAL(status(QString)), this, SLOT(statusDlt(QString)));

    delete ui;
}

void Dialog::restoreSettings()
{
}

void Dialog::updateSettings()
{
}

void Dialog::on_pushButtonStart_clicked()
{
    // start communication
    updateSettings();

    // start Relais and DLT communication
    dltTestRobot.start();
    dltMiniServer.start();

    // disable settings and start button
    // enable stop button
    ui->pushButtonStart->setDisabled(true);
    ui->pushButtonStop->setDisabled(false);
    ui->pushButtonDefaultSettings->setDisabled(true);
    ui->pushButtonLoadSettings->setDisabled(true);
    ui->pushButtonSettings->setDisabled(true);

    connect(&dltTestRobot, SIGNAL(message(unsigned int,QByteArray)), this, SLOT(message(unsigned int,QByteArray)));

}

void Dialog::on_pushButtonStop_clicked()
{
    // stop communication

    disconnect(&dltTestRobot, SIGNAL(message(unsigned int,QByteArray)), this, SLOT(message(unsigned int,QByteArray)));

    // stop Relais and DLT communication
    dltTestRobot.stop();
    dltMiniServer.stop();

    // enable settings and start button
    // disable stop button
    ui->pushButtonStart->setDisabled(false);
    ui->pushButtonStop->setDisabled(true);
    ui->pushButtonDefaultSettings->setDisabled(false);
    ui->pushButtonLoadSettings->setDisabled(false);
    ui->pushButtonSettings->setDisabled(false);
}

void Dialog::statusTestRobot(QString text)
{
    // status from Test Robot

    // status of CAN communication changed
    if(text == "" || text == "stopped" || text == "not active")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::white);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
    else if(text == "started")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
    else if(text == "disconnected")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::yellow);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
    else if(text == "error")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditStatusTestRobot->setPalette(palette);
        ui->lineEditStatusTestRobot->setText(text);
    }
}

void Dialog::statusDlt(QString text)
{
    // status from DLT Mini Server
    ui->lineEditStatusDLT->setText(text);

    // status of DLT communication changed
    if(text == "" || text == "stopped")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::white);
        ui->lineEditStatusDLT->setPalette(palette);
    }
    else if(text == "listening")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::yellow);
        ui->lineEditStatusDLT->setPalette(palette);
    }
    else if(text == "connected")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditStatusDLT->setPalette(palette);
    }
    else if(text == "error")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditStatusDLT->setPalette(palette);
    }
}

void Dialog::on_pushButtonDefaultSettings_clicked()
{
    // Reset settings to default
    dltTestRobot.clearSettings();
    dltMiniServer.clearSettings();

    restoreSettings();
}

void Dialog::on_pushButtonLoadSettings_clicked()
{
    // Load settings from XML file

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Settings"), "", tr("DLTTestRobot Settings (*.xml);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        // No file was selected or cancel was pressed
        return;
    }

    // read the settings from XML file
    dltTestRobot.readSettings(fileName);
    dltMiniServer.readSettings(fileName);

    restoreSettings();
}

void Dialog::on_pushButtonSaveSettings_clicked()
{
    // Save settings into XML file

    updateSettings();

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Settings"), "", tr("DLTTestRobot Settings (*.xml);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        // No file was selected or cancel was pressed
        return;
    }

    // read the settings from XML file
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        // Cannot open the file for writing
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);

    // FIXME: Cannot read data from XML file, which contains a start document
    // So currently do not call StartDocument
    //xml.writeStartDocument();

    xml.writeStartElement("DLTCanSettings");
        dltTestRobot.writeSettings(xml);
        dltMiniServer.writeSettings(xml);
    xml.writeEndElement(); // DLTRelaisSettings

    // FIXME: Cannot read data from XML file, which contains a end document
    // So currently do not call EndDocument
    //xml.writeEndDocument();
    file.close();

}

void Dialog::on_pushButtonSettings_clicked()
{
    // Open settings dialog
    SettingsDialog dlg(this);

    dlg.restoreSettings(&dltTestRobot, &dltMiniServer);
    if(dlg.exec()==QDialog::Accepted)
    {
        dlg.backupSettings(&dltTestRobot, &dltMiniServer);
        restoreSettings();
    }
}

void Dialog::on_pushButtonInfo_clicked()
{
    // Open information window
    QMessageBox msgBox(this);

    msgBox.setWindowTitle("Info DLTPower");
    msgBox.setTextFormat(Qt::RichText);

    QString text;
    text += QString("Version: %1<br>").arg(DLT_TEST_ROBOT_VERSION);
    text += "<br>";
    text += "Information and Documentation can be found here:<br>";
    text += "<br>";
    text += "<a href='https://github.com/alexmucde/DLTCan'>Github DLTTestRobot</a><br>";
    text += "<br>";
    text += "This SW is licensed under GPLv3.<br>";
    text += "<br>";
    text += "(C) 2021 Alexander Wenzel <alex@eli2.de>";

    msgBox.setText(text);

    msgBox.setStandardButtons(QMessageBox::Ok);

    msgBox.exec();
}

void Dialog::on_pushButtonTestLoad_clicked()
{
    // Load test file

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Tests"), "", tr("DLTTestRobot Tests (*.dtr);;All files (*.*)"));

    if(fileName.isEmpty())
    {
        // No file was selected or cancel was pressed
        return;
    }

    QSettings settings;
    settings.setValue("autoloadTests/filename",fileName);

    loadTests(fileName);
}

void Dialog::loadTests(QString fileName)
{
    ui->lineEditTestFile->setText(fileName);

    // read the settings from XML file
    dltTestRobot.readTests(fileName);

    ui->comboBoxTestName->clear();
    for(int num=0;num<dltTestRobot.size();num++)
    {
        ui->comboBoxTestName->addItem(QString("%1 %2 (%3)").arg(dltTestRobot.testId(num)).arg(dltTestRobot.testSize(num)).arg(dltTestRobot.testDescription(num)));
    }
}

void Dialog::on_pushButtonStartTest_clicked()
{
    ui->lineEditCmdNo->setText(QString("%1/%2").arg(0).arg(dltTestRobot.testSize(ui->comboBoxTestName->currentIndex())));

    dltMiniServer.sendValue2("test start ",dltTestRobot.testId(ui->comboBoxTestName->currentIndex()));

    if(ui->checkBoxRunAllTest->isChecked())
        dltTestRobot.startTest(-1,ui->lineEditRepeat->text().toInt());
    else
        dltTestRobot.startTest(ui->comboBoxTestName->currentIndex(),ui->lineEditRepeat->text().toInt());
}

void Dialog::on_pushButtonStopTest_clicked()
{
    dltTestRobot.stopTest();

    dltMiniServer.sendValue("test stopped ",DLT_LOG_FATAL);
}

void Dialog::command(int allTestRepeatNum,int allTestRepeat, int testRepeatNum,int testRepeat,int testNum, int commandNum,int commandCount, QString text)
{
    ui->lineEditCmdNo->setText(QString("%1/%2").arg(commandNum+1).arg(commandCount));
    ui->lineEditCurrentCommand->setText(text);
    ui->lineEditRepeatNo->setText(QString("%1/%2").arg(allTestRepeatNum+1).arg(allTestRepeat));
    ui->lineEditTestRepeatNo->setText(QString("%1/%2").arg(testRepeatNum+1).arg(testRepeat));
    ui->lineEditCurrentTest->setText(QString("%1 (%2)").arg(dltTestRobot.testId(testNum)).arg(dltTestRobot.testDescription(testNum)));
    ui->lineEditFailed->setText(QString("%1").arg(dltTestRobot.getFailed()));

    if(dltTestRobot.getFailed()>0)
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditFailed->setPalette(palette);
    }
    else
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditFailed->setPalette(palette);
    }

    if(text=="end success")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::green);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);
        dltMiniServer.sendValue2("test end success",dltTestRobot.testId(ui->comboBoxTestName->currentIndex()));
    }
    else if(text=="failed")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);
        dltMiniServer.sendValue2("test failed",dltTestRobot.testId(ui->comboBoxTestName->currentIndex()),DLT_LOG_FATAL);
    }
    else if(text=="stopped")
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::red);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);
    }
    else
    {
        QPalette palette;
        palette.setColor(QPalette::Base,Qt::white);
        ui->lineEditCurrentCommand->setPalette(palette);
        ui->lineEditCurrentCommand->setText(text);
        dltMiniServer.sendValue3("test step",QString("%1").arg(commandNum),text);
    }
}

void Dialog::on_checkBoxAutoloadTests_clicked(bool checked)
{
    QSettings settings;
    settings.setValue("autoloadTests/checked",checked);
}

void Dialog::on_checkBoxRunAllTest_clicked(bool checked)
{
}
