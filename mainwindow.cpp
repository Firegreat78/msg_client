// Copyright 2025 Medvedev Dan (https://github.com/Firegreat78)

/*
Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logger.h"
#include "socketmanager.h"
#include "windowmanager.h"
#include "JsonTypes.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QHostAddress>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Logger& logger = Logger::getInstance();
    SocketManager& sm = SocketManager::getInstance();
    WindowManager& wm = WindowManager::getInstance();
    QTcpSocket const* socket = SocketManager::getInstance().getSocket();

    ui->setupUi(this);
    this->setWindowTitle("Messenger");
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::loginButtonClick);
    connect(ui->registerButton, &QPushButton::clicked, this, &MainWindow::registerButtonClick);
    connect(ui->switchActionStatusButton, &QPushButton::clicked, this, &MainWindow::switchStatusButtonClick);
    connect(ui->reTryToConnectButton, &QPushButton::clicked, this, &MainWindow::tryReconnect);

    reconnectTimer = new QTimer(this);
    labelTickTimer = new QTimer(this);
    heartbeatTimer = new QTimer(this);
    connect(labelTickTimer, &QTimer::timeout, this, &MainWindow::updateReconnectionLabel);
    connect(reconnectTimer, &QTimer::timeout, this, &MainWindow::tryReconnect);
    sm.tryConnect();
    ui->reTryToConnectButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);

    ui->loginLineEdit->setValidator(wm.loginValidator.get());
    ui->usernameLineEdit->setValidator(wm.loginValidator.get());
    ui->enterUsernameLabel->hide();
    ui->usernameLineEdit->hide();

    connect(&sm, &SocketManager::arrivedJSMainWin, this, &MainWindow::onJsonArrived);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (isActive) return;
    isActive = true;
    this->customShow();
}

void MainWindow::hideEvent(QHideEvent* ev)
{
    QMainWindow::hideEvent(ev);
}

void MainWindow::closeEvent(QCloseEvent* ev)
{
    QMainWindow::closeEvent(ev);
}

void MainWindow::loginButtonClick()
{
    SocketManager& sm = SocketManager::getInstance();
    WindowManager& wm = WindowManager::getInstance();

    Logger& logger = Logger::getInstance();
    if (!SocketManager::isConnectedToServer())
    {
        QString msg = "Cannot log in: the client is not connected to the server.";
        logger.log(msg.toStdString());
        QMessageBox::warning(this, "Not connected", msg);
        return;
    }
    QString login = ui->loginLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    if (login.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Login failed!",
            "Cannot log in: empty fields are not permitted.");
        return;
    }

    QJsonObject json;
    json["type"] = USER_LOGIN;
    json["login"] = login;
    json["password_hash"] = wm.sha256FromQString(password);
    sm.sendJSON(json);
}

void MainWindow::registerButtonClick()
{
    WindowManager& wm = WindowManager::getInstance();
    SocketManager& sm = SocketManager::getInstance();
    QTcpSocket const* socket = sm.getSocket();
    Logger& logger = Logger::getInstance();
    if (socket->state() != QAbstractSocket::ConnectedState) // no server connection
    {
        QString msg = "Cannot register: the client is not connected to the server.";
        logger.log(msg.toStdString());
        QMessageBox::warning(this, "Not connected", msg);
        return;
    }

    QString login = ui->loginLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString username = ui->usernameLineEdit->text();
    if (login.isEmpty() || username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Registration failed!",
            "Registration failed: empty fields are not permitted.");
        return;
    }
    QJsonObject json;
    json["type"] = USER_REGISTER;
    json["login"] = login;
    json["username"] = username;
    json["password_hash"] = wm.sha256FromQString(password);
    sm.sendJSON(json);
}

void MainWindow::switchStatusButtonClick()
{
    if (login_mode)
    {
        // switch to 'register' state
        ui->switchActionStatusButton->setText("Изменить режим (на режим логина)");
        ui->enterUsernameLabel->show();
        ui->usernameLineEdit->show();
        if (connected) ui->registerButton->setEnabled(true);
        ui->loginButton->setEnabled(false);
    }
    else
    {
        //switch to 'login' state
        ui->switchActionStatusButton->setText("Изменить режим (на режим регистрации)");
        ui->enterUsernameLabel->hide();
        ui->usernameLineEdit->hide();
        if (connected) ui->loginButton->setEnabled(true);
        ui->registerButton->setEnabled(false);
    }
    login_mode = !login_mode;
}

void MainWindow::tryReconnect()
{
    if (!this->isVisible()) return;
    SocketManager& s_manager = SocketManager::getInstance();
    Logger::getInstance().log("Trying to reconnect to the server...");
    connected = false;
    ui->serverConnectionLabel->setText("Connecting to the server...");
    reconnectTimer->stop();
    labelTickTimer->stop();
    s_manager.tryConnect();
    ui->reTryToConnectButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);
}

void MainWindow::onSocketConnected()
{
    if (!this->isVisible()) return;
    SocketManager& s_manager = SocketManager::getInstance();
    QTcpSocket const* socket = s_manager.getSocket();
    if (socket->state() == QAbstractSocket::SocketState::UnconnectedState)
    {
        Logger::getInstance().log("Failed to connect to the server.");
        ui->serverConnectionLabel->setText("No server connection... Retrying in 60 sec(s).");
        reconnectTimer->start(60000);
        secondsUntilReconnection = 60;
        labelTickTimer->start(1000);
        heartbeatTimer->stop();
        ui->reTryToConnectButton->setEnabled(true);
        ui->registerButton->setEnabled(false);
        ui->loginButton->setEnabled(false);
        connected = false;
        return;
    }
    Logger::getInstance().log("Socket connected to the server.");
    ui->serverConnectionLabel->setText("Server connection established.");
    reconnectTimer->stop();
    labelTickTimer->stop();
    heartbeatTimer->start(5000);
    ui->reTryToConnectButton->setEnabled(false);
    ui->registerButton->setEnabled(!this->login_mode);
    ui->loginButton->setEnabled(this->login_mode);
    connected = true;
}

void logJSON(std::string const& msg, QJsonObject const& js)
{
    QString str = QString::fromUtf8(QJsonDocument(js).toJson(QJsonDocument::Compact));
    Logger::getInstance().log(msg + str.toStdString());
}

void MainWindow::onJsonArrived()
{
    Logger& logger = Logger::getInstance();
    SocketManager& s_manager = SocketManager::getInstance();
    std::optional<QJsonObject> js = s_manager.popJSON(MAIN_WINDOW);
    if (!js.has_value()) return;
    handle_json(js.value());
}

void MainWindow::handle_json(QJsonObject const& js)
{
    Logger& logger = Logger::getInstance();
    int const type = js["type"].toInt();

    if (type == ERROR_TYPE)
    {
        std::string const msg = std::string("Server error occured: ") +
            js["info"].toString().toStdString();
        Logger::getInstance().log(msg);
        return;
    }
    QJsonObject response = js["response"].toObject();
    if (type == USER_REGISTER) registerResponseHandler(response);
    else if (type == USER_LOGIN) loginResponseHandler(response);
}

void MainWindow::onSocketDisconnected()
{
    Logger::getInstance().log("Socket disconnected");
    ui->serverConnectionLabel->setText("Нет подключения к серверу... Повтор через 60 сек.");
    reconnectTimer->start(60000);
    secondsUntilReconnection = 60;
    labelTickTimer->start(1000);
    heartbeatTimer->stop();
    ui->reTryToConnectButton->setEnabled(true);
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);
    connected = false;
}

void MainWindow::onSocketError(QAbstractSocket::SocketError error)
{
    std::string log_msg = std::string("Socket error signal emitted. Error code: ") + std::to_string(error);
    Logger::getInstance().log(log_msg);
    ui->serverConnectionLabel->setText("Нет подключения к серверу... Повтор через 60 сек.");
    reconnectTimer->start(60000);
    secondsUntilReconnection = 60;
    labelTickTimer->start(1000);
    heartbeatTimer->stop();
    ui->reTryToConnectButton->setEnabled(true);
    ui->registerButton->setEnabled(false);
    ui->loginButton->setEnabled(false);
    connected = false;
}

void MainWindow::updateReconnectionLabel()
{
    secondsUntilReconnection--;
    if (secondsUntilReconnection <= 0) return;

    QString txt = QString("Нет подключения к серверу... Повтор через %1 сек.").arg(secondsUntilReconnection);
    ui->serverConnectionLabel->setText(txt);
}

void MainWindow::loginResponseHandler(QJsonObject const& response)
{
    bool success = (response["success"] == 1);
    if (!success)
    {
        QString reason = response["reason"].toString();
        QMessageBox::information(this, "Login is not successful", reason);
        return;
    }

    WindowManager& wm = WindowManager::getInstance();
    wm.user_id = response["id"].toInteger(-1);
    if (wm.user_id == -1)
    {
        QMessageBox::warning(this, "Failure", "Ошибка на сервере");
        return;
    }
    wm.login = ui->loginLineEdit->text();
    wm.username = response["username"].toString();
    wm.ts_account_created = response["created_at"].toString();
    ui->loginLineEdit->setText("");
    ui->usernameLineEdit->setText("");
    ui->passwordLineEdit->setText("");
    wm.showWindow("BaseWindow");
    this->customHide();
}

void MainWindow::registerResponseHandler(QJsonObject const& response)
{
    bool success = (response["success"] == 1);
    if (success)
    {
        QMessageBox::information(this, "Register successful!", "Success!");
        ui->loginLineEdit->setText("");
        ui->usernameLineEdit->setText("");
        ui->passwordLineEdit->setText("");
        return;
    }

    QMessageBox::information(this, "Register not successful!", response["reason"].toString());
}

void MainWindow::customHide()
{
    WindowManager::getInstance().hideWindow("MainWindow");
    this->isActive = false;
    QTcpSocket const* socket = SocketManager::getInstance().getSocket();
    disconnect(socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    disconnect(socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    disconnect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);
    heartbeatTimer->stop();
}

void MainWindow::customShow()
{
    QTcpSocket const* socket = SocketManager::getInstance().getSocket();
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);
    heartbeatTimer->start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}
