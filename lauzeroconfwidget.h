/**************************************************************************************************
    Copyright (C) 2017 Dr. Daniel L. Lau
    This file is part of LAURobotWidget.

    LAURobotWidget is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LAURobotWidget is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LAURobotWidget.  If not, see <http://www.gnu.org/licenses/>.

**************************************************************************************************/

#ifndef LAUZEROCONFWIDGET_H
#define LAUZEROCONFWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <qzeroconf.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUZeroConfClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LAUZeroConfClientWidget(QString service = QString("_qtzeroconf_test._tcp"), QWidget *parent = 0);
    ~LAUZeroConfClientWidget();

    QString address() const
    {
        return (ipString);
    }

    int port() const
    {
        return (portNumber);
    }

public slots:
    void onLineEdit_currentIndexChanged(QString string);

private:
    QComboBox *tcpAddressComboBox;
    QString ipString, serviceString;
    QZeroConf *zeroConf;
    int portNumber;

private slots:
    void onServiceError(QZeroConf::error_t error);
    void onRemoveService(QZeroConfService *item);
    void onUpdateService(QZeroConfService *);
    void onAddService(QZeroConfService *item);

signals:
    void emitValidAddress(bool state);
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUZeroConfClientDialog : public QDialog
{
    Q_OBJECT

public:
    LAUZeroConfClientDialog(QString serviceString = QString("_qtzeroconf_test._tcp"), QWidget *parent = 0) : QDialog(parent)
    {
        this->setWindowTitle(QString("TCP Server Address"));
        this->setLayout(new QVBoxLayout());
        this->layout()->setContentsMargins(6, 6, 6, 6);
        this->layout()->setSpacing(6);

        widget = new LAUZeroConfClientWidget(serviceString, this);
        this->layout()->addWidget(widget);

        QPushButton *button = new QPushButton(QString("Connect"));
        button->setEnabled(false);
        connect(button, SIGNAL(clicked()), this, SLOT(accept()));
        connect(widget, SIGNAL(emitValidAddress(bool)), button, SLOT(setEnabled(bool)));

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
        buttonBox->addButton(button, QDialogButtonBox::AcceptRole);
        connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
        this->layout()->addWidget(buttonBox);
    }
    ~LAUZeroConfClientDialog() { ; }

    QString address() const
    {
        return (ipString);
    }

    int port() const
    {
        return (portNumber);
    }

protected:
    void showEvent(QShowEvent *)
    {
        this->setFixedSize(this->size());
    }
    void accept()
    {
        ipString = widget->address();
        portNumber = widget->port();
        QDialog::accept();
    }

private:
    LAUZeroConfClientWidget *widget;
    QString ipString, serviceString;
    int portNumber;
};

#endif // LAUZEROCONFWIDGET_H
