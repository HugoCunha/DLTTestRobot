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

DLTTest::DLTTest()
{
    clear();
}

DLTTest::~DLTTest()
{

}

DLTTest& DLTTest::operator=(DLTTest &test)
{
    commands = test.commands;
    id = test.id;
    description = test.description;

    return *this;
}

void DLTTest::clear()
{
    commands.clear();
    id="";
    description="";
    repeat = 1;
}

QString DLTTest::getId() const
{
    return id;
}

void DLTTest::setId(const QString &value)
{
    id = value;
}

QString DLTTest::getDescription() const
{
    return description;
}

void DLTTest::setDescription(const QString &value)
{
    description = value;
}

int DLTTest::getRepeat() const
{
    return repeat;
}

void DLTTest::setRepeat(int value)
{
    repeat = value;
}


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
    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(&tcpSocket, SIGNAL(hostFound()), this, SLOT(hostFound()));
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

    tcpSocket.connectToHost("localhost",4490);

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

    status("connect");
}

void DLTTestRobot::stop()
{
    tcpSocket.close();

    disconnect(&tcpSocket, SIGNAL(connected()), this, SLOT(connected()));
    disconnect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    disconnect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    disconnect(&tcpSocket, SIGNAL(hostFound()), this, SLOT(hostFound()));
    disconnect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

    timer.stop();
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

    testNum = -1;
    commandNum = -1;

    status("stopped");
}

void DLTTestRobot::clearSettings()
{
    timer.stop();

    testNum = -1;
    commandNum = -1;
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

void DLTTestRobot::readyRead()
{
    // data on was received
    while (tcpSocket.canReadLine())
    {
        QString text = QString(tcpSocket.readLine());

        if(text.size()>0 && commandNum!=-1 && testNum!=-1)
        {
            text.chop(1);

            // line is not empty
            qDebug() << "DltTestRobot: readLine" << text;

            QStringList list = text.split(' ');

            QString currentCommand = tests[testNum].at(commandNum);
            QStringList listCommand = currentCommand.split(' ');

            if(listCommand.size()>=8 && list.size()>=5 && listCommand[0]=="find" && listCommand[3]==list[0] && listCommand[4]==list[1] && listCommand[5]==list[2])
            {
                if(listCommand[1]=="equal")
                {
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    qDebug() << "DltTestRobot: find equal" << listCommand.join(' ');
                    if(text.contains(listCommand.join(' ')))
                    {
                        qDebug() << "DltTestRobot: find equal matches";
                        timer.stop();
                        commandNum++;
                        runTest();
                    }
                }
                else if(listCommand[1]=="greater" && listCommand[6]==list[3])
                {
                    float value = list[4].toFloat();
                    float commandValue = listCommand[7].toFloat();
                    qDebug() << "DltTestRobot: find greater" << commandValue;
                    if(value>commandValue)
                    {
                        qDebug() << "DltTestRobot: find greater matches";
                        timer.stop();
                        commandNum++;
                        runTest();
                    }
                }
                else if(listCommand[1]=="smaller" && listCommand[6]==list[3])
                {
                    float value = list[4].toFloat();
                    float commandValue = listCommand[7].toFloat();
                    qDebug() << "DltTestRobot: find smaller" << commandValue;
                    if(value<commandValue)
                    {
                        qDebug() << "DltTestRobot: find smaller matches";
                        timer.stop();
                        commandNum++;
                        runTest();
                    }
                }
            }

        }
    }

}

void DLTTestRobot::connected()
{
    status("connected");

    qDebug() << "DLTTestRobot: connected";
}

void DLTTestRobot::disconnected()
{
    status("disconnected");

    qDebug() << "DLTTestRobot: disconnected";

    tcpSocket.connectToHost("localhost",4490);
}

void DLTTestRobot::hostFound()
{
    qDebug() << "DLTTestRobot: hostFound";
}

void DLTTestRobot::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "DLTTestRobot: error" << socketError;

    if(tcpSocket.state()==QAbstractSocket::UnconnectedState)
        tcpSocket.connectToHost("localhost",4490);
}

void DLTTestRobot::send(QString text)
{
    qDebug() << "DLTTestRobot: send" << text;

    text += "\n";

    tcpSocket.write(text.toLatin1());

}

void DLTTestRobot::readTests(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "DLTTestRobot: failed to open file" << filename;
        return;
    }

    tests.clear();

    DLTTest test;
    bool isTest=false;
    QTextStream in(&file);
    while (!in.atEnd())
    {
       QString line = in.readLine();

       if(line.size()==0)
       {

       }
       else if(line.at(0)==';')
       {
           qDebug() << "DLTTestRobot: comment" << line;
       }
       else
       {
           line = line.simplified();
           QStringList list = line.split(' ');
           if(list[0]=="test")
           {
               if(list[1]=="id")
               {
                   qDebug() << "DLTTestRobot: id" << list[2];
                   test.setId(list[2]);
               }
               else if(list[1]=="description")
               {
                   list.removeAt(0);
                   list.removeAt(0);
                   qDebug() << "DLTTestRobot: description" << list.join(' ');
                   test.setDescription(list.join(' '));
               }
               else if(list[1]=="repeat")
               {
                   qDebug() << "DLTTestRobot: repeat" << list[2];
                   test.setRepeat(list[2].toInt());
               }
               else if(list[1]=="begin")
               {
                   qDebug() << "DLTTestRobot: begin" << test.getId();
                   isTest = true;
               }
               else if(list[1]=="end")
               {
                   qDebug() << "DLTTestRobot: end" << test.getId();
                   isTest=false;
                   tests.append(test);
                   test.clear();
               }
           }
           else if(isTest)
           {
               qDebug() << "DLTTestRobot: command" << line;
               test.append(line);
           }
       }
    }

    file.close();
}

void DLTTestRobot::startTest(int num,int repeat)
{
    if(repeat<1)
        allTestRepeat = 1;
    else
        allTestRepeat = repeat;
    allTestRepeatNum = 0;

    if(num<0)
    {
        allTests = true;
        testNum = 0;
    }
    else
    {
        allTests = false;
        testNum = num;
    }
    testCount = tests.size();

    testRepeat = tests[testNum].getRepeat();
    testRepeatNum = 0;

    commandCount = tests[testNum].size();
    commandNum = 0;

    failed = 0;

    qDebug() << "DLTTestRobot: start test" << tests[testNum].getId();

    command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"started");

    runTest();
}

void DLTTestRobot::stopTest()
{
    timer.stop();

    command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"stopped");

    qDebug() << "DLTTestRobot: stopped test" ;
}

void DLTTestRobot::runTest()
{
    while(commandNum<commandCount)
    {

        QString currentCommand = tests[testNum].at(commandNum);

        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,currentCommand);

        QStringList list = currentCommand.split(' ');

        if(list.size()>=2 && list[0]=="wait")
        {
            timer.start(list[1].toUInt());
            qDebug() << "DLTTestRobot: start wait timer" << list[1].toUInt();
            return;
        }
        else if(list.size()>=3 && list[0]=="find")
        {
            timer.start(list[2].toUInt());
            qDebug() << "DLTTestRobot: start find timer" << list[2].toUInt();
            return;
        }
        else
        {
            send(currentCommand);
        }
        commandNum++;
    }
    // end reached
    command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"end success");

    qDebug() << "DLTTestRobot: end test" << tests[testNum].getId();

    nextTest();
}

bool DLTTestRobot::nextTest()
{
    testRepeatNum++;
    if(testRepeatNum<testRepeat)
    {
        commandCount = tests[testNum].size();
        commandNum = 0;

        qDebug() << "DLTTestRobot: start test" << tests[testNum].getId();

        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"started");

        runTest();
        return true;
    }

    if(allTests)
    {
        testNum++;
        if(testNum<testCount)
        {
            testRepeat = tests[testNum].getRepeat();
            testRepeatNum = 0;

            commandCount = tests[testNum].size();
            commandNum = 0;

            qDebug() << "DLTTestRobot: start test" << tests[testNum].getId();

            command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"started");

            runTest();
            return true;
        }
    }

    allTestRepeatNum++;
    if(allTestRepeatNum<allTestRepeat)
    {
        if(allTests)
        {
            testNum = 0;
        }

        testRepeat = tests[testNum].getRepeat();
        testRepeatNum = 0;

        commandCount = tests[testNum].size();
        commandNum = 0;

        qDebug() << "DLTTestRobot: start test" << tests[testNum].getId();

        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"started");

        runTest();
        return true;
    }

    testNum = -1;
    commandNum = -1;

    return false;
}

int DLTTestRobot::getFailed() const
{
    return failed;
}

void DLTTestRobot::setFailed(int value)
{
    failed = value;
}

void DLTTestRobot::timeout()
{
    timer.stop();

    qDebug() << "DLTTestRobot: timer expired";

    QString currentCommand = tests[testNum].at(commandNum);
    QStringList list = currentCommand.split(' ');

    if(list.size()>=1 && list[0]!="wait")
    {
        failed++;
        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"failed");

        qDebug() << "DLTTestRobot: end test" << tests[testNum].getId();

        nextTest();

        return;
    }

    commandNum++;

    runTest();
}
