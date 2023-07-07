#include "DialogAddFeed.h"
#include "ui_DialogAddFeed.h"

ZapFR::Client::DialogAddFeed::DialogAddFeed(QWidget* parent) : QDialog(parent), ui(new Ui::DialogAddFeed)
{
    ui->setupUi(this);

    mSourcesModel = std::make_unique<QStandardItemModel>(this);
    ui->comboBoxSource->setModel(mSourcesModel.get());
}

ZapFR::Client::DialogAddFeed::~DialogAddFeed()
{
    delete ui;
}

void ZapFR::Client::DialogAddFeed::reset(const std::vector<std::unique_ptr<ZapFR::Engine::Source>>& sources)
{
    ui->lineEditURL->setText("");

    mSourcesModel->clear();
    for (const auto& source : sources)
    {
        auto item = new QStandardItem(QString::fromUtf8(source->title()));
        item->setData(QVariant::fromValue<uint64_t>(source->id()), SourceIDRole);
        mSourcesModel->appendRow(item);
    }
}

QString ZapFR::Client::DialogAddFeed::url() const
{
    return ui->lineEditURL->text().trimmed();
}

uint64_t ZapFR::Client::DialogAddFeed::sourceID() const
{
    return ui->comboBoxSource->currentData(SourceIDRole).toULongLong();
}
