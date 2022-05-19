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
 * \file dialog.h
 * @licence end@
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSettings>
#include <QFile>
#include <QDateTime>

#include "dlttestrobot.h"
#include "dltminiserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(bool autostart,QString configuration,QWidget *parent = nullptr);
    ~Dialog();

private slots:

    // Status of Test Robot and DLT connection
    void statusTestRobot(QString text);
    void statusTests(QString text);
    void statusDlt(QString text);
    void report(QString text);
    void reportSummary(QString text);

    // Settings and Info
    void on_pushButtonSettings_clicked();
    void on_pushButtonDefaultSettings_clicked();
    void on_pushButtonLoadSettings_clicked();
    void on_pushButtonSaveSettings_clicked();
    void on_pushButtonInfo_clicked();

    // Start and stop communication
    void on_pushButtonStart_clicked();
    void on_pushButtonStop_clicked();

    void on_pushButtonTestLoad_clicked();

    void on_pushButtonStartTest_clicked();

    void command(int allTestRepeatNum,int allTestRepeat, int testRepeatNum,int testRepeat,int testNum, int commandNum,int commandCount, QString text);

    void on_checkBoxAutoloadTests_clicked(bool checked);

    void on_pushButtonStopTest_clicked();

private:
    Ui::Dialog *ui;

    DLTTestRobot dltTestRobot;
    DLTMiniServer dltMiniServer;

    QFile reportFile;
    QStringList reportSummaryList;
    int reportFailedCounter,reportSuccessCounter;

    // Settings
    void restoreSettings();
    void updateSettings();

    void loadTests(QString fileName);

    void startTests();
    void stopTests();

    void writeSummaryToReport();

};
#endif // DIALOG_H
