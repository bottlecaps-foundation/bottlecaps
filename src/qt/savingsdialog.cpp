#include "savingsdialog.h"
#include "ui_savingsdialog.h"

#include "walletmodel.h"
#include "base58.h"
#include "addressbookpage.h"
#include "init.h"


AutoSavingsDialog::AutoSavingsDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoSavingsDialog),
    model(0)
{
    ui->setupUi(this);

    ui->label_2->setFocus();
}

AutoSavingsDialog::~AutoSavingsDialog()
{
    delete ui;
}

void AutoSavingsDialog::setModel(WalletModel *model)
{
    this->model = model;

    QString strAddress = model->getAutoSavingsAddress();
    QString strPer = QString::number(model->getAutoSavingsPercent());

    if (!strAddress.isEmpty() && strPer.toInt() > 0 )
    {
        ui->savingsAddressEdit->setText(strAddress);
        ui->savingsPercentEdit->setText(strPer);
        ui->message->setStyleSheet("QLabel { color: green; }");
        ui->message->setText(tr("You are now saving to\n") + strAddress + tr("."));
    }
}

void AutoSavingsDialog::setAddress(const QString &address)
{
    ui->savingsAddressEdit->setText(address);
    ui->savingsAddressEdit->setFocus();
}

void AutoSavingsDialog::on_addressBookButton_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress(dlg.getReturnValue());
        }
    }
}

void AutoSavingsDialog::on_enableButton_clicked()
{
    if(model->getEncryptionStatus() == WalletModel::Locked)
    {
        ui->message->setStyleSheet("QLabel { color: black; }");
        ui->message->setText(tr("Please unlock wallet before starting auto savings."));
        return;
    }

    bool fValidConversion = false;
    int64 nMinAmount = MIN_TXOUT_AMOUNT;
    int64 nMaxAmount = MAX_MONEY;

    CBitcoinAddress address = ui->savingsAddressEdit->text().toStdString();
    if (!address.IsValid())
    {
        ui->message->setStyleSheet("QLabel { color: red; }");
        ui->message->setText(tr("The entered address: ") + ui->savingsAddressEdit->text() + tr(" is invalid.\nPlease check the address and try again."));
        ui->savingsAddressEdit->setFocus();
        return;
    }

    int nSavingsPercent = ui->savingsPercentEdit->text().toInt(&fValidConversion, 10);
    if (!fValidConversion || nSavingsPercent > 50 || nSavingsPercent <= 0)
    {
        ui->message->setStyleSheet("QLabel { color: red; }");
        ui->message->setText(tr("Please Enter 1 - 50 for percent."));
        ui->savingsPercentEdit->setFocus();
        return;
    }

    if (!ui->savingsMinEdit->text().isEmpty())
    {
        nMinAmount = ui->savingsMinEdit->text().toDouble(&fValidConversion) * COIN;
        if(!fValidConversion || nMinAmount <= MIN_TXOUT_AMOUNT || nMinAmount >= MAX_MONEY  )
        {
            ui->message->setStyleSheet("QLabel { color: red; }");
            ui->message->setText(tr("Min Amount out of Range, please re-enter."));
            ui->savingsMinEdit->setFocus();
            return;
        }
    }

    if (!ui->savingsMaxEdit->text().isEmpty())
    {
        nMaxAmount = ui->savingsMaxEdit->text().toDouble(&fValidConversion) * COIN;
        if(!fValidConversion || nMaxAmount <= MIN_TXOUT_AMOUNT || nMaxAmount >= MAX_MONEY  )
        {
            ui->message->setStyleSheet("QLabel { color: red; }");
            ui->message->setText(tr("Max Amount out of Range, please re-enter."));
            ui->savingsMaxEdit->setFocus();
            return;
        }
    }

    model->setAutoSavings(true, nSavingsPercent, address, nMinAmount, nMaxAmount);
    ui->message->setStyleSheet("QLabel { color: green; }");
    ui->message->setText(tr("You are now saving to\n") + QString(address.ToString().c_str()) + tr("."));
    return;
}

void AutoSavingsDialog::on_disableButton_clicked()
{
    int nSavingsPercent = 0;
    CBitcoinAddress address = "";
    int64 nMinAmount = MIN_TXOUT_AMOUNT;
    int64 nMaxAmount = MAX_MONEY;

    model->setAutoSavings(false, nSavingsPercent, address, nMinAmount, nMaxAmount);
    ui->savingsAddressEdit->clear();
    ui->savingsMaxEdit->clear();
    ui->savingsMinEdit->clear();
    ui->savingsPercentEdit->clear();
    ui->message->setStyleSheet("QLabel { color: black; }");
    ui->message->setText(tr("Auto Savings is now off"));
    return;
}
