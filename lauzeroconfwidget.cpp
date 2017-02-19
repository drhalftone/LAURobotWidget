/**************************************************************************************************
    Copyright 2017 Dr. Daniel L. Lau

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
**************************************************************************************************/
#include "lauzeroconfwidget.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUZeroConfClientWidget::LAUZeroConfClientWidget(QString service, QWidget *parent) : QWidget(parent), tcpAddressComboBox(NULL), serviceString(service), zeroConf(NULL)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->setSpacing(0);
    this->setWindowTitle(QString("TCP 3D Video Recorder"));

    QWidget *widget = new QWidget();
    widget->setLayout(new QVBoxLayout());
    widget->layout()->setContentsMargins(6, 6, 6, 6);
    this->layout()->addWidget(widget);

    QGroupBox *box = new QGroupBox(QString("Server Address"));
    box->setLayout(new QHBoxLayout());
    box->layout()->setContentsMargins(6, 6, 6, 6);
    widget->layout()->addWidget(box);

    tcpAddressComboBox = new QComboBox();
    tcpAddressComboBox->setMinimumWidth(300);
    connect(tcpAddressComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLineEdit_currentIndexChanged(QString)));

    QLabel *label = new QLabel(QString("Host:"));
    label->setFixedWidth(40);
    box->layout()->addWidget(label);
    box->layout()->addWidget(tcpAddressComboBox);

    // MAKE CONNECTIONS BETWEEN THIS OBJECT AND THE BONJOUR SERVICE OBJECT
    zeroConf = new QZeroConf();
    connect(zeroConf, SIGNAL(serviceAdded(QZeroConfService *)), this, SLOT(onAddService(QZeroConfService *)));
    connect(zeroConf, SIGNAL(serviceRemoved(QZeroConfService *)), this, SLOT(onRemoveService(QZeroConfService *)));
    connect(zeroConf, SIGNAL(serviceUpdated(QZeroConfService *)), this, SLOT(onUpdateService(QZeroConfService *)));
    connect(zeroConf, SIGNAL(error(QZeroConf::error_t)), this, SLOT(onServiceError(QZeroConf::error_t)));
    zeroConf->startBrowser(serviceString, QAbstractSocket::IPv4Protocol);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
LAUZeroConfClientWidget::~LAUZeroConfClientWidget()
{
    if (zeroConf) {
        delete zeroConf;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onServiceError(QZeroConf::error_t error)
{
    switch (error) {
        case QZeroConf::noError:
            break;
        case QZeroConf::serviceRegistrationFailed:
            QMessageBox::warning(NULL, QString("LAUZeroConfClientWidget"), QString("Zero Conf Client Error: Registration failed!"), QMessageBox::Ok);
            break;
        case QZeroConf::serviceNameCollision:
            QMessageBox::warning(NULL, QString("LAUZeroConfClientWidget"), QString("Zero Conf Client Error: Name collision!"), QMessageBox::Ok);
            break;
        case QZeroConf::browserFailed:
            QMessageBox::warning(NULL, QString("LAUZeroConfClientWidget"), QString("Zero Conf Client Error: Browser failed!"), QMessageBox::Ok);
            break;
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onAddService(QZeroConfService *item)
{
    tcpAddressComboBox->addItem(QString("%1::%2").arg(item->name).arg(item->port), item->ip.toString());
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onRemoveService(QZeroConfService *item)
{
    tcpAddressComboBox->removeItem(tcpAddressComboBox->findText(QString("%1::%2").arg(item->name).arg(item->port)));
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onUpdateService(QZeroConfService *)
{
    qDebug() << "LAUZeroConfClientWidget :: Update service!";
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void LAUZeroConfClientWidget::onLineEdit_currentIndexChanged(QString string)
{
    QStringList strings = string.split(QString("::"));
    if (strings.count() == 2) {
        ipString = tcpAddressComboBox->currentData().toString();
        portNumber = strings.last().toInt();
        emit emitValidAddress(true);
    } else {
        ipString = QString();
        portNumber = -1;
        emit emitValidAddress(false);
    }
}
