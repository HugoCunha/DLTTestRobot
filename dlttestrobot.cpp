/**
 * @licence app begin@
 * Copyright (C) 2021 Alexander Wenzel
 *
 * This file is part of the DLT Multimeter project.
 *
 * \copyright This code is licensed under GPLv3.
 *
 * \author Alexander Wenzel <alex@eli2.de>
 *
 * \file dltcan.cpp
 * @licence end@
 */

#include "dlttestrobot.h"

#include <QDebug>
#include <QFile>
#include <QSerialPortInfo>

DLTTestRobot::DLTTestRobot(QObject *parent) : QObject(parent)
{
    clearSettings();
}

DLTTestRobot::~DLTTestRobot()
{
    stop();
}

void DLTTestRobot::start()
{
}

void DLTTestRobot::stop()
{
}

void DLTTestRobot::clearSettings()
{
}

void DLTTestRobot::writeSettings(QXmlStreamWriter &xml)
{
    /* Write project settings */
    xml.writeStartElement(QString("DLTTestRobot"));
    xml.writeEndElement(); // DLTTestRobot
}

void DLTTestRobot::readSettings(const QString &filename)
{
    bool isDLTTestRobot = false;

    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text))
             return;

    QXmlStreamReader xml(&file);

    while (!xml.atEnd())
    {
          xml.readNext();

          if(xml.isStartElement())
          {
              if(isDLTTestRobot)
              {

              }
              else if(xml.name() == QString("DLTTestRobot"))
              {
                    isDLTTestRobot = true;
              }
          }
          else if(xml.isEndElement())
          {
              /* Connection, plugin and filter */
              if(xml.name() == QString("DLTTestRobot"))
              {
                    isDLTTestRobot = false;
              }
          }
    }
    if (xml.hasError())
    {
         qDebug() << "Error in processing filter file" << filename << xml.errorString();
    }

    file.close();
}

