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

private:
    QString id;
    QString description;
    QStringList commands;
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
    QString testId(int num) { return tests[num].getId(); }
    QString testDescription(int num) { return tests[num].getDescription(); }
    int testSize(int num) { return tests[num].size(); }

    void startTest(int num);

signals:

    void status(QString text);
    void command(int num, QString text);

private slots:

    void readyRead();
    void connected();
    void disconnected();

    void timeout();

private:

    void runTest();

    QTimer timer;
    unsigned int watchDogCounter,watchDogCounterLast;

    QTcpSocket tcpSocket;

    QList<DLTTest> tests;

    int currentTest;
    int currentCmd;

};

#endif // DLT_TES_ROBOT_H
