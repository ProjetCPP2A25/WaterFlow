#include "commandes.h"
#include "commande.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDate>
#include <QDebug>
#include <QSqlRecord>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QLabel>

Commandes::Commandes(QWidget *parent)
    : QWidget(parent)
{
    // Plus de ui->setupUi(this). On utilise findChild pour accéder aux widgets.
    QDateEdit* dateCmd = findChild<QDateEdit*>("dateEdit_commande");
    QDateEdit* datePrev = findChild<QDateEdit*>("dateEdit_prevue");
    QTableWidget* tableAlertes = findChild<QTableWidget*>("tableWidget_alertes");

    if(dateCmd) dateCmd->setDate(QDate::currentDate());
    if(datePrev) datePrev->setDate(QDate::currentDate().addDays(5));

    if(tableAlertes) {
        tableAlertes->setColumnCount(3);
        tableAlertes->setHorizontalHeaderLabels({"ID Cmd", "Client", "Alerte"});
        tableAlertes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    chargerDonnees();
    calculerStatistiques();
    detecterCommandesInhabituelles();

    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(tableCmd) {
        connect(tableCmd, &QTableWidget::cellClicked, this, &Commandes::on_tableWidget_commandes_cellClicked);
    }
}

Commandes::~Commandes()
{
}

void Commandes::chargerDonnees()
{
    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    tableCmd->setRowCount(0);

    Commande c;
    QSqlQueryModel* model = c.afficher();

    for (int i = 0; i < model->rowCount(); ++i) {
        int row = tableCmd->rowCount();
        tableCmd->insertRow(row);
        for (int j = 0; j < 9; ++j) {
            tableCmd->setItem(row, j, new QTableWidgetItem(model->record(i).value(j).toString()));
        }
    }
    delete model;
}

void Commandes::on_pushButton_ajouter_clicked()
{
    QLineEdit* editId = findChild<QLineEdit*>("lineEdit_id");
    QLineEdit* editClient = findChild<QLineEdit*>("lineEdit_client");
    QLineEdit* editProduit = findChild<QLineEdit*>("lineEdit_produit");
    QLineEdit* editQte = findChild<QLineEdit*>("lineEdit_quantite");
    QDateEdit* dateCmd = findChild<QDateEdit*>("dateEdit_commande");
    QLineEdit* editAdresse = findChild<QLineEdit*>("lineEdit_adresse");
    QDateEdit* datePrev = findChild<QDateEdit*>("dateEdit_prevue");
    QComboBox* comboStatut = findChild<QComboBox*>("comboBox_statut");
    QLineEdit* editMontant = findChild<QLineEdit*>("lineEdit_montant");

    if(!editId || !editClient || !editProduit || !editQte || !dateCmd || !editAdresse || !datePrev || !comboStatut || !editMontant) return;

    QString id = editId->text().trimmed();
    QString client = editClient->text().trimmed();
    QString produit = editProduit->text().trimmed();
    int qte = editQte->text().toInt();
    QString dateCmdStr = dateCmd->date().toString("yyyy-MM-dd");
    QString adresse = editAdresse->text().trimmed();
    QString datePrevStr = datePrev->date().toString("yyyy-MM-dd");
    QString statut = comboStatut->currentText();
    double montant = editMontant->text().toDouble();

    if(id.isEmpty() || client.isEmpty() || produit.isEmpty() || editQte->text().isEmpty() || editMontant->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Veuillez remplir tous les champs obligatoires.");
        return;
    }

    Commande c(id, client, produit, qte, dateCmdStr, adresse, datePrevStr, statut, montant);

    if(c.ajouter()) {
        QMessageBox::information(this, "Succès", "Commande ajoutée dans la base de données !");
        chargerDonnees();
        calculerStatistiques();
        detecterCommandesInhabituelles();
        on_pushButton_vider_clicked();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible d'ajouter la commande.");
    }
}

void Commandes::on_pushButton_modifier_clicked()
{
    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    int row = tableCmd->currentRow();
    if(row < 0) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner une commande à modifier.");
        return;
    }

    QLineEdit* editClient = findChild<QLineEdit*>("lineEdit_client");
    QLineEdit* editProduit = findChild<QLineEdit*>("lineEdit_produit");
    QLineEdit* editQte = findChild<QLineEdit*>("lineEdit_quantite");
    QDateEdit* dateCmd = findChild<QDateEdit*>("dateEdit_commande");
    QLineEdit* editAdresse = findChild<QLineEdit*>("lineEdit_adresse");
    QDateEdit* datePrev = findChild<QDateEdit*>("dateEdit_prevue");
    QComboBox* comboStatut = findChild<QComboBox*>("comboBox_statut");
    QLineEdit* editMontant = findChild<QLineEdit*>("lineEdit_montant");

    if(!editClient || !editProduit || !editQte || !dateCmd || !editAdresse || !datePrev || !comboStatut || !editMontant) return;

    QString id = tableCmd->item(row, 0)->text();
    QString client = editClient->text();
    QString produit = editProduit->text();
    int qte = editQte->text().toInt();
    QString dateCmdStr = dateCmd->date().toString("yyyy-MM-dd");
    QString adresse = editAdresse->text();
    QString datePrevStr = datePrev->date().toString("yyyy-MM-dd");
    QString statut = comboStatut->currentText();
    double montant = editMontant->text().toDouble();

    Commande c(id, client, produit, qte, dateCmdStr, adresse, datePrevStr, statut, montant);

    if(c.modifier(id)) {
        QMessageBox::information(this, "Succès", "Commande modifiée !");
        chargerDonnees();
        calculerStatistiques();
        detecterCommandesInhabituelles();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de modifier.");
    }
}

void Commandes::on_pushButton_supprimer_clicked()
{
    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    int row = tableCmd->currentRow();
    if(row < 0) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner une commande à supprimer.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirmation", "Êtes-vous sûr de vouloir supprimer cette commande ?", QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes) {
        QString id = tableCmd->item(row, 0)->text();

        Commande c;
        if(c.supprimer(id)) {
            QMessageBox::information(this, "Supprimé", "La commande a été supprimée.");
            chargerDonnees();
            calculerStatistiques();
            detecterCommandesInhabituelles();
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer.");
        }
    }
}

void Commandes::on_pushButton_vider_clicked()
{
    QLineEdit* editId = findChild<QLineEdit*>("lineEdit_id");
    QLineEdit* editClient = findChild<QLineEdit*>("lineEdit_client");
    QLineEdit* editProduit = findChild<QLineEdit*>("lineEdit_produit");
    QLineEdit* editQte = findChild<QLineEdit*>("lineEdit_quantite");
    QLineEdit* editAdresse = findChild<QLineEdit*>("lineEdit_adresse");
    QLineEdit* editMontant = findChild<QLineEdit*>("lineEdit_montant");
    QLineEdit* editTransporteur = findChild<QLineEdit*>("lineEdit_transporteur");
    QComboBox* comboStatut = findChild<QComboBox*>("comboBox_statut");

    if(editId) editId->clear();
    if(editClient) editClient->clear();
    if(editProduit) editProduit->clear();
    if(editQte) editQte->clear();
    if(editAdresse) editAdresse->clear();
    if(editMontant) editMontant->clear();
    if(editTransporteur) editTransporteur->clear();
    if(comboStatut) comboStatut->setCurrentIndex(0);
}

void Commandes::on_tableWidget_commandes_cellClicked(int row, int column)
{
    Q_UNUSED(column);
    if(row < 0) return;

    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    QLineEdit* editId = findChild<QLineEdit*>("lineEdit_id");
    QLineEdit* editClient = findChild<QLineEdit*>("lineEdit_client");
    QLineEdit* editProduit = findChild<QLineEdit*>("lineEdit_produit");
    QLineEdit* editQte = findChild<QLineEdit*>("lineEdit_quantite");
    QDateEdit* dateCmd = findChild<QDateEdit*>("dateEdit_commande");
    QLineEdit* editAdresse = findChild<QLineEdit*>("lineEdit_adresse");
    QDateEdit* datePrev = findChild<QDateEdit*>("dateEdit_prevue");
    QComboBox* comboStatut = findChild<QComboBox*>("comboBox_statut");
    QLineEdit* editMontant = findChild<QLineEdit*>("lineEdit_montant");

    if(editId) editId->setText(tableCmd->item(row, 0)->text());
    if(editClient) editClient->setText(tableCmd->item(row, 1)->text());
    if(editProduit) editProduit->setText(tableCmd->item(row, 2)->text());
    if(editQte) editQte->setText(tableCmd->item(row, 3)->text());
    if(dateCmd) dateCmd->setDate(QDate::fromString(tableCmd->item(row, 4)->text(), "yyyy-MM-dd"));
    if(editAdresse) editAdresse->setText(tableCmd->item(row, 5)->text());
    if(datePrev) datePrev->setDate(QDate::fromString(tableCmd->item(row, 6)->text(), "yyyy-MM-dd"));
    if(comboStatut) comboStatut->setCurrentText(tableCmd->item(row, 7)->text());
    if(editMontant) editMontant->setText(tableCmd->item(row, 8)->text());
}

void Commandes::on_pushButton_trierDate_clicked()
{
    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(tableCmd) tableCmd->sortItems(4, Qt::DescendingOrder);
}

void Commandes::on_lineEdit_recherche_textChanged(const QString &arg1)
{
    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    QString filtre = arg1.toLower();
    for(int i = 0; i < tableCmd->rowCount(); ++i) {
        bool match = false;
        for(int j = 0; j < tableCmd->columnCount(); ++j) {
            if(tableCmd->item(i, j) && tableCmd->item(i, j)->text().toLower().contains(filtre)) {
                match = true;
                break;
            }
        }
        tableCmd->setRowHidden(i, !match);
    }
}

void Commandes::on_comboBox_filtreStatut_currentIndexChanged(int index)
{
    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    QComboBox* comboFiltre = findChild<QComboBox*>("comboBox_filtreStatut");
    if(!tableCmd || !comboFiltre) return;

    if(index == 0) {
        for(int i = 0; i < tableCmd->rowCount(); ++i)
            tableCmd->setRowHidden(i, false);
        return;
    }
    QString statutRecherche = comboFiltre->currentText();
    for(int i = 0; i < tableCmd->rowCount(); ++i) {
        QString statut = tableCmd->item(i, 7)->text();
        tableCmd->setRowHidden(i, statut != statutRecherche);
    }
}

void Commandes::on_pushButton_exportCSV_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Exporter en CSV", "commandes_waterflow.csv", "CSV Files (*.csv)");
    if(filename.isEmpty()) return;

    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "ID,Client,Produit,Qte,DateCmd,Adresse,DatePrev,Statut,Montant\n";
        for(int i = 0; i < tableCmd->rowCount(); ++i) {
            QStringList rowData;
            for(int j = 0; j < tableCmd->columnCount(); ++j) {
                rowData << (tableCmd->item(i, j) ? tableCmd->item(i, j)->text() : "");
            }
            out << rowData.join(",") << "\n";
        }
        file.close();
        QMessageBox::information(this, "Export Réussi", "Fichier CSV exporté avec succès !");
    }
}

void Commandes::on_pushButton_majStatutAuto_clicked()
{
    int retardCount = 0;
    QDate aujourdhui = QDate::currentDate();

    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    for(int i = 0; i < tableCmd->rowCount(); ++i) {
        QString statut = tableCmd->item(i, 7)->text();
        QDate datePrev = QDate::fromString(tableCmd->item(i, 6)->text(), "yyyy-MM-dd");

        if(statut != "Livrée" && statut != "Annulée" && datePrev < aujourdhui) {
            retardCount++;
        }
    }
    QMessageBox::information(this, "Suivi en Temps Réel", QString("Vérification des livraisons effectuée.\n%1 commande(s) active(s) dépassent la date prévue de livraison !").arg(retardCount));
}

void Commandes::calculerStatistiques()
{
    double caTotal = 0.0;
    int attente = 0, preparation = 0, expediee = 0, livree = 0;

    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableCmd) return;

    for(int i = 0; i < tableCmd->rowCount(); ++i) {
        caTotal += tableCmd->item(i, 8)->text().toDouble();
        QString statut = tableCmd->item(i, 7)->text();
        if(statut == "En attente") attente++;
        else if(statut == "En préparation") preparation++;
        else if(statut == "Expédiée") expediee++;
        else if(statut == "Livrée") livree++;
    }

    QLabel* labelCA = findChild<QLabel*>("label_caTotal");
    QLabel* labelStats = findChild<QLabel*>("label_statsDetails");
    if(labelCA) labelCA->setText(QString("Chiffre d'Affaires Total : %1 TND").arg(caTotal, 0, 'f', 2));
    if(labelStats) labelStats->setText(QString("En attente: %1 | En préparation: %2 | Expédiées: %3 | Livrées: %4").arg(attente).arg(preparation).arg(expediee).arg(livree));
}

void Commandes::detecterCommandesInhabituelles()
{
    QTableWidget* tableAlertes = findChild<QTableWidget*>("tableWidget_alertes");
    QTableWidget* tableCmd = findChild<QTableWidget*>("tableWidget_commandes");
    if(!tableAlertes || !tableCmd) return;

    tableAlertes->setRowCount(0);
    for(int i = 0; i < tableCmd->rowCount(); ++i) {
        int qte = tableCmd->item(i, 3)->text().toInt();
        if(qte > 200) {
            int row = tableAlertes->rowCount();
            tableAlertes->insertRow(row);
            tableAlertes->setItem(row, 0, new QTableWidgetItem(tableCmd->item(i, 0)->text()));
            tableAlertes->setItem(row, 1, new QTableWidgetItem(tableCmd->item(i, 1)->text()));
            tableAlertes->setItem(row, 2, new QTableWidgetItem("Quantité élevée (> 200 unités)"));
        }
    }
}