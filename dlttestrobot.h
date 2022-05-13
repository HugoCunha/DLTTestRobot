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
 * \file dltcan.h
 * @licence end@
 */

#ifndef DLT_TES_ROBOT_H
#define DLT_TES_ROBOT_H

#include <QObject>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTcpSocket>
#include <QTimer>

class DLTTest
{
public:
    DLTTest();
    ~DLTTest();

    DLTTest& operator=(DLTTest &test);

    void clear();

    QString getId() const;
    void setId(const QString &value);

    QString getDescription() const;
    void setDescription(const QString &value);

    int size() { return commands.size(); }

    void append(const QString &text) {  commands.append(text); }
    QString at(int num) { return commands[num]; }

    int getRepeat() const;
    void setRepeat(int value);

    const QStringList &getCommands() const;

private:
    QString id;
    QString description;
    QStringList commands;
    int repeat;
};

class DLTTestRobot : public QObject
{
    Q_OBJECT
public:
    explicit DLTTestRobot(QObject *parent = nullptr);
    ~DLTTestRobot();

    void start();
    void stop();

    void clearSettings();
    void writeSettings(QXmlStreamWriter &xml);
    void readSettings(const QString &filename);

    void readTests(const QString &filename);

    void send(QString text);

    int size() { return tests.size(); }
    QString testId(int num) { if(num>=0 && num<tests.length()) return tests[num].getId(); else return QString(); }
    QString testDescription(int num) { if(num>=0 && num<tests.length()) return tests[num].getDescription(); else return QString(); }
    int testSize(int num) { if(num>=0 && num<tests.length()) return tests[num].size(); else return 0; }

    void startTest(int num = -1,int repeat = 1);
    void stopTest();

    int getFailed() const;
    void setFailed(int value);

    DLTTest getTest(int num) { if(num>=0 && num<tests.length()) return tests[num]; else return DLTTest(); }

    const QString &getTestsFilename() const;

    const QString &getVersion() const;

    bool getAllTests() const;

signals:

    void status(QString text);
    void command(int allTestRepeatNum,int allTestRepeat, int testRepeatNum,int testRepeat,int testNum, int commandNum,int commandCount, QString text);

private slots:

    void hostFound();
    void readyRead();
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError);

    void timeout();

private:

    void runTest();
    bool nextTest();

    QTimer timer;
    unsigned int watchDogCounter,watchDogCounterLast;

    QTcpSocket tcpSocket;

    QList<DLTTest> tests;

    QString testsFilename;

    QString version;

    int allTestRepeat;
    int allTestRepeatNum;

    bool allTests;
    int testCount;
    int testNum;

    int testRepeat;
    int testRepeatNum;

    int commandCount;
    int commandNum;

    int failed;

};

#endif // DLT_TES_ROBOT_H
