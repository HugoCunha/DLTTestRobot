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
#include <QRandomGenerator>

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
    fail = "next";
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

const QStringList &DLTTest::getCommands() const
{
    return commands;
}

const QString &DLTTest::getFail() const
{
    return fail;
}

void DLTTest::setFail(const QString &newFail)
{
    fail = newFail;
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

    status("connect");

    tcpSocket.connectToHost("localhost",4490);    

    // if it is already connected set connected state
    if(tcpSocket.state()==QAbstractSocket::ConnectedState)
        status("connected");

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
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
            //qDebug() << "DltTestRobot: readLine" << text;

            QStringList list = text.split(' ');

            QString currentCommand;
            if(testNum>=0 && testNum<tests.size())
                currentCommand= tests[testNum].at(commandNum);
            QStringList listCommand = currentCommand.split(' ');

            if(listCommand.size()>=7 && list.size()>=5 && listCommand[0]=="find" && listCommand[3]==list[0] && listCommand[4]==list[1] && listCommand[5]==list[2])
            {
                if(listCommand[1]=="equal")
                {
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    //qDebug() << "DltTestRobot: find equal" << listCommand.join(' ');
                    if(text.contains(listCommand.join(' ')))
                    {
                        emit this->report(text);
                        qDebug() << "DltTestRobot: find equal matches";
                        timer.stop();
                        commandNum++;
                        runTest();
                    }
                }
                else if(listCommand[1]=="unequal")
                {
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    listCommand.removeAt(0);
                    //qDebug() << "DltTestRobot: find unequal" << listCommand.join(' ');
                    if(text.contains(listCommand.join(' ')))
                    {
                        emit this->report(text);
                        qDebug() << "DltTestRobot: find unequal matches";
                        timer.stop();

                        failedTestCommand();
                    }
                }
                else if(listCommand[1]=="greater" && listCommand[6]==list[3])
                {
                    float value = list[4].toFloat();
                    float commandValue = listCommand[7].toFloat();
                    qDebug() << "DltTestRobot: find greater" << commandValue;
                    if(value>commandValue)
                    {
                        emit this->report(list[4]);
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
                        emit this->report(list[4]);
                        qDebug() << "DltTestRobot: find smaller matches";
                        timer.stop();
                        commandNum++;
                        runTest();
                    }
                }
            }
            else if(listCommand.size()==7 && list.size()>=5 &&  listCommand[0]=="measure" && listCommand[2]==list[0] && listCommand[3]==list[1] && listCommand[4]==list[2] && listCommand[5]==list[3])
            {
                qDebug() << "DltTestRobot: measure" << listCommand[6] << list[4];
                emit this->report(QString("Measure %1 %2").arg(listCommand[6]).arg(list[4]));
                emit this->reportSummary(QString("Measure %1 %2").arg(listCommand[6]).arg(list[4]));
                timer.stop();
                commandNum++;
                runTest();
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

    testsFilename = filename;

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
               else if(list[1]=="fail")
               {
                   qDebug() << "DLTTestRobot: fail" << list[2];
                   test.setFail(list[2]);
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
           else if(list[0]=="version")
           {
               version = list[1];
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
    emit statusTests("Prerun");
    timer.start(3000);
    state = Prerun;

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

    failed = false;
}

void DLTTestRobot::stopTest()
{
    if(testNum != -1 && commandNum!=-1)
    {
        timer.stop();

        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"stopped");

        testNum = -1;
        commandNum = -1;
    }

    emit statusTests("Postrun");
    timer.start(3000);
    state = Postrun;

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
            if(list.size()==3)
            {
                // Random wait
                uint waitTime = QRandomGenerator::global()->bounded(list[1].toUInt(), list[2].toUInt());
                qDebug() << "DLTTestRobot: start random wait timer" << waitTime;
                timer.start(waitTime);
                emit this->report(QString("Random Wait %1").arg(waitTime));
                emit this->reportSummary(QString("Random Wait %1").arg(waitTime));
            }
            else
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
        else if(list.size()>=2 && list[0]=="measure")
        {
            timer.start(list[1].toUInt());
            qDebug() << "DLTTestRobot: start measure timer" << list[1].toUInt();
            return;
        }
        else
        {
            send(currentCommand);
        }
        commandNum++;
    }
    // end reached
    if(failed)
        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"failed");
    else
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
        failed = false;

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
            failed = false;

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
        failed = false;

        qDebug() << "DLTTestRobot: start test" << tests[testNum].getId();

        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"started");

        runTest();
        return true;
    }

    command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"end");

    testNum = -1;
    commandNum = -1;

    qDebug() << "DLTTestRobot: all tests done" ;

    emit statusTests("Postrun");
    timer.start(3000);
    state = Postrun;

    return false;
}

int DLTTestRobot::getFailedTestCommands() const
{
    return failedTestCommands;
}

bool DLTTestRobot::getAllTests() const
{
    return allTests;
}

const QString &DLTTestRobot::getVersion() const
{
    return version;
}

const QString &DLTTestRobot::getTestsFilename() const
{
    return testsFilename;
}

bool DLTTestRobot::getFailed() const
{
    return failed;
}

void DLTTestRobot::timeout()
{
    timer.stop();

    qDebug() << "DLTTestRobot: timer expired";

    if(state==Prerun)
    {
        state = Running;
        emit statusTests("Running");

        testCount = tests.size();

        testRepeat = tests[testNum].getRepeat();
        testRepeatNum = 0;

        commandCount = tests[testNum].size();
        commandNum = 0;

        failedTestCommands = 0;

        qDebug() << "DLTTestRobot: start test" << tests[testNum].getId();

        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"started");

        runTest();
    }
    else if(state==Running)
    {
        QString currentCommand = tests[testNum].at(commandNum);
        QStringList list = currentCommand.split(' ');

        if(list.size()>=1 && list[0]!="wait" && list[0]!= "measure")
        {
            if(list[1]=="unequal")
                successTestCommand();
            else
                failedTestCommand();

            return;
        }

        commandNum++;
        runTest();

    }
    else if(state==Postrun)
    {
        state = Finished;
        emit statusTests("Finished");
    }
}

void DLTTestRobot::successTestCommand()
{
    commandNum++;
    runTest();
}

void DLTTestRobot::failedTestCommand()
{
    failedTestCommands++;
    failed = true;

    if(tests[testNum].getFail()=="continue")
    {
        // continue with current test job
        //command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"failed");
        emit report("Command FAILED, but continue");
        commandNum++;

        qDebug() << "DLTTestRobot: fail continue current test" ;

        runTest();
    }
    else if(tests[testNum].getFail()=="stop")
    {
        // stop all tests
        emit report("Command FAILED, stop all tests");
        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"failed");
        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"end");

        testNum = -1;
        commandNum = -1;

        qDebug() << "DLTTestRobot: fail stop all tests" ;

        emit statusTests("Postrun");
        timer.start(3000);
        state = Postrun;
    }
    else /* tests[testNum].getFail()=="next" */
    {
        // default, run next test
        emit report("Command FAILED, next test");
        command(allTestRepeatNum,allTestRepeat,testRepeatNum,testRepeat,testNum,commandNum,commandCount,"failed");

        qDebug() << "DLTTestRobot: fail run next test";

        nextTest();
    }

}
