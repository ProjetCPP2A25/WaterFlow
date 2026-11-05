#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connection.h"
#include "commande.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QDate>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QGroupBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1400, 900);

    // 1. Connexion à la base
    Connection& dbConn = Connection::createInstance();
    if (!dbConn.createConnection()) {
        qDebug() << "Échec de la connexion à la base de données !";
    }

    // 2. Connexion du menu à gauche aux pages
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, [=](int row) {
        if (ui->stackedWidget) {
            ui->stackedWidget->setCurrentIndex(row);
        }
    });

    // 3. Créer toute l'interface de la page Commandes DANS LE CODE
    QWidget *page = ui->pageCommandes;

    // --- METTRE TOUTE LA PAGE DANS UN SCROLL AREA ---
    QScrollArea *pageScroll = new QScrollArea();
    pageScroll->setWidgetResizable(true);

    QWidget *pageContent = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(pageContent);

    // Titre
    QLabel *titre = new QLabel("Gestion des Commandes");
    QFont font = titre->font();
    font.setPointSize(14);
    font.setBold(true);
    titre->setFont(font);
    mainLayout->addWidget(titre);

    // Formulaire
    QGroupBox *groupBox = new QGroupBox("Nouvelle commande");
    QGridLayout *formLayout = new QGridLayout(groupBox);

    lineEdit_id = new QLineEdit(); lineEdit_id->setMinimumHeight(30);
    lineEdit_client = new QLineEdit(); lineEdit_client->setMinimumHeight(30);
    lineEdit_produit = new QLineEdit(); lineEdit_produit->setMinimumHeight(30);
    lineEdit_quantite = new QLineEdit(); lineEdit_quantite->setMinimumHeight(30);
    dateEdit_commande = new QDateEdit(QDate::currentDate()); dateEdit_commande->setMinimumHeight(30);
    dateEdit_commande->setCalendarPopup(true);
    lineEdit_adresse = new QLineEdit(); lineEdit_adresse->setMinimumHeight(30);
    dateEdit_prevue = new QDateEdit(QDate::currentDate().addDays(5)); dateEdit_prevue->setMinimumHeight(30);
    dateEdit_prevue->setCalendarPopup(true);
    comboBox_statut = new QComboBox(); comboBox_statut->setMinimumHeight(30);
    comboBox_statut->addItems({"En attente", "En préparation", "Expédiée", "Livrée", "Annulée"});
    lineEdit_transporteur = new QLineEdit(); lineEdit_transporteur->setMinimumHeight(30);
    lineEdit_montant = new QLineEdit(); lineEdit_montant->setMinimumHeight(30);

    formLayout->addWidget(new QLabel("ID Commande :"), 0, 0);
    formLayout->addWidget(lineEdit_id, 0, 1);
    formLayout->addWidget(new QLabel("Client / Société :"), 1, 0);
    formLayout->addWidget(lineEdit_client, 1, 1);
    formLayout->addWidget(new QLabel("Produit / Article :"), 2, 0);
    formLayout->addWidget(lineEdit_produit, 2, 1);
    formLayout->addWidget(new QLabel("Quantité :"), 3, 0);
    formLayout->addWidget(lineEdit_quantite, 3, 1);
    formLayout->addWidget(new QLabel("Date Commande :"), 4, 0);
    formLayout->addWidget(dateEdit_commande, 4, 1);
    formLayout->addWidget(new QLabel("Adresse Livraison :"), 5, 0);
    formLayout->addWidget(lineEdit_adresse, 5, 1);
    formLayout->addWidget(new QLabel("Date Prévisionnelle :"), 6, 0);
    formLayout->addWidget(dateEdit_prevue, 6, 1);
    formLayout->addWidget(new QLabel("Statut :"), 7, 0);
    formLayout->addWidget(comboBox_statut, 7, 1);
    formLayout->addWidget(new QLabel("Transporteur :"), 8, 0);
    formLayout->addWidget(lineEdit_transporteur, 8, 1);
    formLayout->addWidget(new QLabel("Montant (TND) :"), 9, 0);
    formLayout->addWidget(lineEdit_montant, 9, 1);

    mainLayout->addWidget(groupBox);

    // Boutons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    pushButton_ajouter = new QPushButton("Ajouter");
    pushButton_modifier = new QPushButton("Modifier");
    pushButton_supprimer = new QPushButton("Supprimer");
    pushButton_vider = new QPushButton("Vider");
    buttonLayout->addWidget(pushButton_ajouter);
    buttonLayout->addWidget(pushButton_modifier);
    buttonLayout->addWidget(pushButton_supprimer);
    buttonLayout->addWidget(pushButton_vider);
    mainLayout->addLayout(buttonLayout);

    // Tableau principal
    tableViewCommandes = new QTableView();
    tableViewCommandes->setMinimumHeight(250);
    mainLayout->addWidget(tableViewCommandes);

    // Tableau des alertes
    tableWidget_alertes = new QTableWidget();
    tableWidget_alertes->setColumnCount(3);
    tableWidget_alertes->setHorizontalHeaderLabels({"ID Cmd", "Client", "Alerte"});
    tableWidget_alertes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget_alertes->setMinimumHeight(100);
    mainLayout->addWidget(tableWidget_alertes);

    // Labels stats
    label_caTotal = new QLabel("Chiffre d'Affaires Total : 0.00 TND");
    label_statsDetails = new QLabel("En attente: 0 | En préparation: 0 | Expédiées: 0 | Livrées: 0");
    mainLayout->addWidget(label_caTotal);
    mainLayout->addWidget(label_statsDetails);

    // Boutons supplémentaires
    pushButton_majStatutAuto = new QPushButton("Vérifier Retards de Livraison");
    pushButton_exportCSV = new QPushButton("Exporter CSV");
    pushButton_trierDate = new QPushButton("Trier par Date");
    mainLayout->addWidget(pushButton_majStatutAuto);
    mainLayout->addWidget(pushButton_exportCSV);
    mainLayout->addWidget(pushButton_trierDate);

    // Recherche et filtre
    lineEdit_recherche = new QLineEdit();
    lineEdit_recherche->setPlaceholderText("Rechercher...");
    comboBox_filtreStatut = new QComboBox();
    comboBox_filtreStatut->addItems({"Tous les statuts", "En attente", "En préparation", "Expédiée", "Livrée", "Annulée"});
    mainLayout->addWidget(lineEdit_recherche);
    mainLayout->addWidget(comboBox_filtreStatut);

    // Appliquer le layout au contenu, puis le contenu au scroll, puis le scroll à la page
    pageContent->setLayout(mainLayout);
    pageScroll->setWidget(pageContent);

    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->addWidget(pageScroll);
    page->setLayout(pageLayout);
    // ----------------------------------------

    // Connexions
    connect(pushButton_ajouter, &QPushButton::clicked, this, &MainWindow::on_pushButton_ajouter_clicked);
    connect(pushButton_modifier, &QPushButton::clicked, this, &MainWindow::on_pushButton_modifier_clicked);
    connect(pushButton_supprimer, &QPushButton::clicked, this, &MainWindow::on_pushButton_supprimer_clicked);
    connect(pushButton_vider, &QPushButton::clicked, this, &MainWindow::on_pushButton_vider_clicked);
    connect(pushButton_trierDate, &QPushButton::clicked, this, &MainWindow::on_pushButton_trierDate_clicked);
    connect(pushButton_exportCSV, &QPushButton::clicked, this, &MainWindow::on_pushButton_exportCSV_clicked);
    connect(pushButton_majStatutAuto, &QPushButton::clicked, this, &MainWindow::on_pushButton_majStatutAuto_clicked);
    connect(lineEdit_recherche, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_recherche_textChanged);
    connect(comboBox_filtreStatut, &QComboBox::currentIndexChanged, this, &MainWindow::on_comboBox_filtreStatut_currentIndexChanged);
    connect(tableViewCommandes, &QTableView::clicked, this, &MainWindow::on_tableView_commandes_clicked);

    // Modèle SQL
    model = new QSqlTableModel(this, Connection::createInstance().getDatabase());
    model->setTable("COMMANDE");
    model->select();
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Client");
    model->setHeaderData(2, Qt::Horizontal, "Produit");
    model->setHeaderData(3, Qt::Horizontal, "Qté");
    model->setHeaderData(4, Qt::Horizontal, "Date Cmd");
    model->setHeaderData(5, Qt::Horizontal, "Adresse");
    model->setHeaderData(6, Qt::Horizontal, "Date Prév");
    model->setHeaderData(7, Qt::Horizontal, "Statut");
    model->setHeaderData(8, Qt::Horizontal, "Montant");
    tableViewCommandes->setModel(model);
    tableViewCommandes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    chargerDonnees();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- TOUTES LES FONCTIONS SLOTS ---
void MainWindow::chargerDonnees()
{
    if (model) model->select();
    calculerStatistiques();
    detecterCommandesInhabituelles();
}

void MainWindow::on_pushButton_ajouter_clicked()
{
    QString id = lineEdit_id->text().trimmed();
    QString client = lineEdit_client->text().trimmed();
    QString produit = lineEdit_produit->text().trimmed();
    int qte = lineEdit_quantite->text().toInt();
    QString dateCmd = dateEdit_commande->date().toString("yyyy-MM-dd");
    QString adresse = lineEdit_adresse->text().trimmed();
    QString datePrev = dateEdit_prevue->date().toString("yyyy-MM-dd");
    QString statut = comboBox_statut->currentText();
    double montant = lineEdit_montant->text().toDouble();

    if(id.isEmpty() || client.isEmpty() || produit.isEmpty() || lineEdit_quantite->text().isEmpty() || lineEdit_montant->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur de saisie", "Veuillez remplir tous les champs obligatoires.");
        return;
    }

    Commande c(id, client, produit, qte, dateCmd, adresse, datePrev, statut, montant);
    if(c.ajouter()) {
        QMessageBox::information(this, "Succès", "Commande ajoutée !");
        chargerDonnees();
        on_pushButton_vider_clicked();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible d'ajouter.");
    }
}

void MainWindow::on_pushButton_modifier_clicked()
{
    QModelIndex index = tableViewCommandes->currentIndex();
    if(!index.isValid()) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner une commande.");
        return;
    }
    QString id = model->record(index.row()).value(0).toString();
    Commande c(id, lineEdit_client->text(), lineEdit_produit->text(),
               lineEdit_quantite->text().toInt(),
               dateEdit_commande->date().toString("yyyy-MM-dd"),
               lineEdit_adresse->text(),
               dateEdit_prevue->date().toString("yyyy-MM-dd"),
               comboBox_statut->currentText(),
               lineEdit_montant->text().toDouble());
    if(c.modifier(id)) {
        QMessageBox::information(this, "Succès", "Commande modifiée !");
        chargerDonnees();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de modifier.");
    }
}

void MainWindow::on_pushButton_supprimer_clicked()
{
    QModelIndex index = tableViewCommandes->currentIndex();
    if(!index.isValid()) {
        QMessageBox::warning(this, "Sélection requise", "Veuillez sélectionner une commande.");
        return;
    }
    if(QMessageBox::question(this, "Confirmation", "Supprimer cette commande ?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QString id = model->record(index.row()).value(0).toString();
        Commande c;
        if(c.supprimer(id)) {
            QMessageBox::information(this, "Supprimé", "Commande supprimée.");
            chargerDonnees();
        } else {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer.");
        }
    }
}

void MainWindow::on_pushButton_vider_clicked()
{
    lineEdit_id->clear();
    lineEdit_client->clear();
    lineEdit_produit->clear();
    lineEdit_quantite->clear();
    lineEdit_adresse->clear();
    lineEdit_montant->clear();
    lineEdit_transporteur->clear();
    comboBox_statut->setCurrentIndex(0);
}

void MainWindow::on_tableView_commandes_clicked(const QModelIndex &index)
{
    if(!index.isValid()) return;
    int row = index.row();
    lineEdit_id->setText(model->record(row).value(0).toString());
    lineEdit_client->setText(model->record(row).value(1).toString());
    lineEdit_produit->setText(model->record(row).value(2).toString());
    lineEdit_quantite->setText(model->record(row).value(3).toString());
    dateEdit_commande->setDate(QDate::fromString(model->record(row).value(4).toString(), "yyyy-MM-dd"));
    lineEdit_adresse->setText(model->record(row).value(5).toString());
    dateEdit_prevue->setDate(QDate::fromString(model->record(row).value(6).toString(), "yyyy-MM-dd"));
    comboBox_statut->setCurrentText(model->record(row).value(7).toString());
    lineEdit_montant->setText(model->record(row).value(8).toString());
}

void MainWindow::on_pushButton_trierDate_clicked()
{
    if (model) {
        model->setSort(4, Qt::DescendingOrder);
        model->select();
    }
}

void MainWindow::on_lineEdit_recherche_textChanged(const QString &arg1)
{
    if (model) {
        model->setFilter(QString("CLIENT LIKE '%%1%'").arg(arg1));
        model->select();
    }
}

void MainWindow::on_comboBox_filtreStatut_currentIndexChanged(int index)
{
    if (!model) return;
    if(index == 0) {
        model->setFilter("");
    } else {
        QString statut = comboBox_filtreStatut->currentText();
        model->setFilter(QString("STATUT = '%1'").arg(statut));
    }
    model->select();
}

void MainWindow::on_pushButton_exportCSV_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Exporter en CSV", "commandes.csv", "CSV Files (*.csv)");
    if(filename.isEmpty()) return;
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "ID,Client,Produit,Qte,DateCmd,Adresse,DatePrev,Statut,Montant\n";
        for(int i = 0; i < model->rowCount(); ++i) {
            QStringList rowData;
            for(int j = 0; j < 9; ++j) {
                rowData << model->record(i).value(j).toString();
            }
            out << rowData.join(",") << "\n";
        }
        file.close();
        QMessageBox::information(this, "Export Réussi", "Fichier CSV exporté !");
    }
}

void MainWindow::on_pushButton_majStatutAuto_clicked()
{
    int retardCount = 0;
    QDate aujourdhui = QDate::currentDate();
    for(int i = 0; i < model->rowCount(); ++i) {
        QString statut = model->record(i).value(7).toString();
        QDate datePrev = QDate::fromString(model->record(i).value(6).toString(), "yyyy-MM-dd");
        if(statut != "Livrée" && statut != "Annulée" && datePrev < aujourdhui) {
            retardCount++;
        }
    }
    QMessageBox::information(this, "Suivi", QString("%1 commande(s) en retard.").arg(retardCount));
}

void MainWindow::calculerStatistiques()
{
    double caTotal = 0.0;
    int attente = 0, preparation = 0, expediee = 0, livree = 0;
    for(int i = 0; i < model->rowCount(); ++i) {
        caTotal += model->record(i).value(8).toDouble();
        QString statut = model->record(i).value(7).toString();
        if(statut == "En attente") attente++;
        else if(statut == "En préparation") preparation++;
        else if(statut == "Expédiée") expediee++;
        else if(statut == "Livrée") livree++;
    }
    label_caTotal->setText(QString("Chiffre d'Affaires Total : %1 TND").arg(caTotal, 0, 'f', 2));
    label_statsDetails->setText(QString("En attente: %1 | En préparation: %2 | Expédiées: %3 | Livrées: %4").arg(attente).arg(preparation).arg(expediee).arg(livree));
}

void MainWindow::detecterCommandesInhabituelles()
{
    tableWidget_alertes->setRowCount(0);
    for(int i = 0; i < model->rowCount(); ++i) {
        int qte = model->record(i).value(3).toInt();
        if(qte > 200) {
            int row = tableWidget_alertes->rowCount();
            tableWidget_alertes->insertRow(row);
            tableWidget_alertes->setItem(row, 0, new QTableWidgetItem(model->record(i).value(0).toString()));
            tableWidget_alertes->setItem(row, 1, new QTableWidgetItem(model->record(i).value(1).toString()));
            tableWidget_alertes->setItem(row, 2, new QTableWidgetItem("Quantité élevée (> 200 unités)"));
        }
    }
}